#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "mechanic.h"
#include "tcp_connect.h"
#include <unistd.h>
#include <QTcpServer>
#include <QTime>


class TcpServer : public QObject
{
    Q_OBJECT

protected:
    QString result_path;

    QTcpServer *server;
    Mechanic *mechanic;
    ClientWrappers clients;

    int ready_cnt;
    int ready_player_id;
    int client_cnt;
    int current_tick;

    // timeouts implementation
    int timerId;
    int wait_timeout;
    bool game_active;

signals:
    void game_finished();

public:
    explicit TcpServer(const QString &_res_path, int _client_cnt) :
        result_path(_res_path),
        server(new QTcpServer),
        mechanic(new Mechanic),
        ready_cnt(0),
        ready_player_id(1),
        client_cnt(_client_cnt),
        current_tick(0),
        game_active(false)
    {
        timerId = startTimer(1000);
        connect(server, SIGNAL(newConnection()), this, SLOT(client_connected()));
    }

    virtual ~TcpServer() {
        for (ClientWrapper *client : clients) {
            if (client) delete client;
        }
        if (mechanic) delete mechanic;
        if (server) {
            server->close();
            delete server;
        }
    }

    void bind(const QString &host, int port) {
        bool result = server->listen(QHostAddress(host), port);
        if (! result) {
            qDebug() << "Already bound to that port. listen() failed";
        }
        qDebug() << "waiting for clients";
        wait_timeout = 0;
    }

    void timerEvent(QTimerEvent *event) {
        if (event->timerId() == timerId && ! game_active) {
            wait_timeout++;
            if (wait_timeout > CONNECT_TIMEOUT) {
                qDebug() << "Waiting expired";
                emit game_finished();
            }
        }
    }

public slots:
    void client_connected() {
        if (clients.length() >= client_cnt) {
            return;
        }
        QTcpSocket *client_socket = server->nextPendingConnection();
        ClientWrapper *client = new ClientWrapper(client_socket);
        clients.append(client);
        qDebug() << "client connected";

        connect(client, SIGNAL(disconnected()), this, SLOT(client_disconnected()));
        connect(client, SIGNAL(ready()), this, SLOT(client_ready()));
        connect(client, SIGNAL(response(Direct)), this, SLOT(client_responsed(Direct)));
        connect(client, SIGNAL(error(QString)), this, SLOT(client_error(QString)));

        connect(client, SIGNAL(debug(QString)), this, SLOT(client_debug(QString)));
        connect(client, SIGNAL(sprite(QString,QString)), this, SLOT(client_sprite(QString,QString)));
    }

    void client_disconnected() {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());
        qDebug() << "client disconnected" << client->getId();

        if (get_active_count() == 0 && game_active) {
            cancel_game();
        } else if (get_answered_clients_count() == get_active_count() && game_active) {
            next_tick();
        }
    }

    int get_active_count() {
        int count = 0;
        for (ClientWrapper *client: clients) {
            if (client->get_is_active()) count++;
        }
        return count;
    }

    int get_ready_clients_count() {
        int count = 0;
        for (ClientWrapper *client: clients) {
            if (client->get_is_ready()) count++;
        }
        return count;
    }

    int get_answered_clients_count() {
        int count = 0;
        for (ClientWrapper *client: clients) {
            if(client->get_answered() && client->get_is_active()) count ++;
        }
        return count;
    }

    void client_ready() {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());
        client->set_player(ready_player_id);

        qDebug() << "client ready" << client->getId();
        ready_player_id++;
        if (get_ready_clients_count() == client_cnt) {
            start_game();
        }
    }

    void start_game() {
        qDebug() << "starting game" << QDateTime::currentDateTime().toString();
        game_active = true;
        wait_timeout = 0;

        std::string seed = Constants::instance().SEED;
        mechanic->init_objects(seed, [] (Player*) -> Strategy* {
            return NULL;
        });

        Logger *ml = mechanic->get_logger();
        for (ClientWrapper *client : clients) {
            // independent from is_canceled
            ml->write_solution_id(client->getId(), client->get_solution_id());
        }
        broadcast_config();

        qDebug() << "sleeping" << PRE_PAUSE << "seconds";
        sleep(PRE_PAUSE);
        broadcast_state();
    }

    void broadcast_config() {
        for (ClientWrapper *client : clients) {
            if (client->get_is_active()) {
                client->send_config();
            }
        }
    }

    void broadcast_state() {
        for (ClientWrapper *client : clients) {
            if (client->get_is_active()) {
                PlayerArray fragments = mechanic->get_players_by_id(client->getId());
                CircleArray visibles = mechanic->get_visibles(fragments);
                client->send_state(fragments, visibles, current_tick);
            }
        }
        ready_cnt = 0;
    }


    void client_responsed(Direct direct) {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());

        mechanic->apply_direct_for(client->getId(), direct);
        if (get_answered_clients_count() == get_active_count() && game_active) {
            next_tick();
        }
    }

    void next_tick() {
        wait_timeout = 0;
        int tick = mechanic->tickEvent();
        if (tick % 100 == 0) {
            qDebug() << "tick" << tick << QDateTime::currentDateTime().toString("hh:mm:ss");
        }
        current_tick = tick;
        if (tick < Constants::instance().GAME_TICKS && !mechanic->known()) {
            broadcast_state();
        }
        else {
            qDebug() << "Successfully played";
            cancel_game();
        }
    }

    void client_error(QString msg) {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());
//        qDebug() << "error (client=" << client->getId() << "):" << msg << "tick=" << current_tick;

        Logger *logger = client->get_logger();
        logger->write_error(current_tick, client->getId(), msg);

        if (get_answered_clients_count() == get_active_count() && game_active) {
            next_tick();
        }
    }

    void cancel_game() {
        for (ClientWrapper *client : clients) {
            client->get_logger()->flush();
            client->get_dump_logger()->flush();
        }
        game_active = false;
        Logger *ml = mechanic->get_logger();
        ml->rewrite_game_ticks(current_tick);
        ml->flush();

        write_scores();
        write_result();
        qDebug() << "Successfully written";
        emit game_finished();
    }

    void client_debug(QString msg) {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());
        Logger *logger = client->get_logger();
        logger->write_debug(current_tick, client->getId(), msg);
    }

    void client_sprite(QString playerId, QString msg) {
        ClientWrapper *client = static_cast<ClientWrapper*>(sender());
        Logger *logger = client->get_logger();
        logger->write_to_sprite(current_tick, client->getId(), playerId, msg);
    }

    void write_scores() {
        QJsonObject jsonResult;
        for (ClientWrapper *client : clients) {
            int score = mechanic->get_score_for(client->getId());
            jsonResult.insert(client->get_solution_id(), QJsonValue(score));
        }
        QJsonDocument jsonDoc(jsonResult);
        QString result = QString(jsonDoc.toJson(QJsonDocument::Compact));

        QFile file(LOG_DIR + SCORES_FILE);
        if (file.open(QIODevice::WriteOnly|QFile::Truncate)) {
            QTextStream f_Stream(&file);
            f_Stream << result;
            file.close();
        }
    }

    void write_result() {
        QJsonObject jsonScores;
        jsonScores.insert("filename", QJsonValue(SCORES_FILE));
        jsonScores.insert("location", QJsonValue(LOG_DIR + SCORES_FILE));
        jsonScores.insert("is_private", QJsonValue(false));

        QJsonArray jsonDebugAll;
        for (ClientWrapper *client : clients) {
            Logger *cl = client->get_logger();
            QJsonObject jsonDebug;
            jsonDebug.insert("filename", QJsonValue(cl->get_file_name() + ".gz"));
            jsonDebug.insert("is_private", QJsonValue(true));
            jsonDebug.insert("location", QJsonValue(cl->get_path() + ".gz"));
            jsonDebugAll.append(jsonDebug);

            cl = client->get_dump_logger();
            QJsonObject jsonDumpDebug;
            jsonDebug.insert("filename", QJsonValue(cl->get_file_name() + ".gz"));
            jsonDebug.insert("is_private", QJsonValue(true));
            jsonDebug.insert("location", QJsonValue(cl->get_path() + ".gz"));
            jsonDebugAll.append(jsonDebug);

        }
        Logger *ml = mechanic->get_logger();
        QJsonObject jsonResult;
        jsonResult.insert("filename", QJsonValue(ml->get_file_name() + ".gz"));
        jsonResult.insert("is_private", QJsonValue(false));
        jsonResult.insert("location", QJsonValue(ml->get_path() + ".gz"));

        QJsonObject jsonAll;
        jsonAll.insert(SCORES_JSON_KEY, QJsonValue(jsonScores));
        jsonAll.insert(DEBUG_JSON_KEY, QJsonValue(jsonDebugAll));
        jsonAll.insert(MAIN_JSON_KEY, QJsonValue(jsonResult));

        QJsonDocument jsonDoc(jsonAll);
        QString result = QString(jsonDoc.toJson(QJsonDocument::Compact));

        QFile file(result_path);
        if (file.open(QIODevice::WriteOnly|QFile::Truncate)) {
            QTextStream f_Stream(&file);
            f_Stream << result;
            file.close();
        }
    }
};

#endif // TCP_SERVER_H
