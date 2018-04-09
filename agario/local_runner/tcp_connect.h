#ifndef TCP_CONNECT_H
#define TCP_CONNECT_H

#include "logger.h"
#include <QTimerEvent>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTcpSocket>


class ClientWrapper : public QObject
{
    Q_OBJECT

protected:
    QTcpSocket *socket;
    Logger *logger;
    Logger *dump_logger;
    QString solution_id;
    int player_id;
    bool is_ready;
    bool answered;
    bool is_active;

    QByteArray got_data;

    // timeouts implementation
    int timerId;
    int wait_timeout;
    bool waiting;
    int sum_waiting;

signals:
    void ready();
    void disconnected();
    void response(Direct);
    void error(QString);
    void debug(QString);
    void sprite(QString, QString);

public:
    explicit ClientWrapper(QTcpSocket *_socket) :
        socket(_socket),
        logger(new Logger),
        dump_logger(new Logger),
        is_ready(false),
        wait_timeout(0),
        waiting(false),
        sum_waiting(0),
        is_active(false),
        answered(false)
    {
        timerId = startTimer(100);
        connect(socket, SIGNAL(readyRead()), this, SLOT(read_data()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(client_disconnected()));
    }

    virtual ~ClientWrapper() {
        if (logger) delete logger;
        if (socket) delete socket;
    }

    void set_player(int pId) {
        player_id = pId;
        is_active = true;
    }

    int getId() const {
        return player_id;
    }

    QString get_solution_id() const {
        return solution_id;
    }

    void timerEvent(QTimerEvent *event) {

        if (event->timerId() == timerId && waiting && is_active) {
            wait_timeout++;
            if (wait_timeout > Constants::instance().RESP_TIMEOUT * 10) {
                bool is_expired = accumulate_wait();
                if (is_expired) return;

                emit error(RESP_EXPIRED);
                is_active = false;
                this->socket->disconnectFromHost();
            }
        }
    }

    bool accumulate_wait() {
        waiting = false;
        sum_waiting += wait_timeout;
        wait_timeout = 0;

        if (sum_waiting > Constants::instance().SUM_RESP_TIMEOUT * 10) {
            is_active = false;
            emit error(SUM_RESP_EXPIRED);
            this->socket->disconnectFromHost();
            return true;
        }
        return false;
    }

    bool get_is_ready() {
        return is_ready;
    }

    bool get_answered() {
        return answered;
    }

    bool get_is_active() {
        return is_active;
    }

public slots:
    void client_disconnected() {
        if (is_active) {
            emit error(CLIENT_DISCONNECTED);
        }
        is_active = false;
        emit disconnected();
    }

    void read_data() {
        QByteArray data = socket->readLine(MAX_RESP_LEN + 1);
        if (data[data.length() - 1] != '\n' && data.length() < MAX_RESP_LEN) {
            got_data.append(data);
            return;
        }
        got_data.append(data);
        got_data = got_data.left(MAX_RESP_LEN);
        answered = true;
        bool is_expired = accumulate_wait();
        if (is_expired) return;

        if (! is_ready) {
            QJsonObject json = parse_answer(got_data);
            if (json.isEmpty()) {
                emit error("Can't parse json: " + got_data);
                return;
            }
            got_data.clear();
            QStringList keys = json.keys();
            if (! keys.contains("solution_id")) {
                emit error("No required key 'solution_id'");
                return;
            }

            solution_id = json.value("solution_id").toString();
            logger->init_file(solution_id, DEBUG_FILE);
            dump_logger->init_file(solution_id, DUMP_FILE);
            is_ready = true;
            emit ready();
        }
        else {
            QJsonObject json = parse_answer(got_data);
            if (json.isEmpty()) {
                emit error("Can't parse json: " + got_data);
                return;
            }
            got_data.clear();
            QJsonDocument doc(json);
            dump_logger->write_raw_with_old_tick(doc.toJson(QJsonDocument::Compact)+ "\n");
            QStringList keys = json.keys();
            if (keys.contains("error")) {
                QString err_msg = json.value("error").toString();
                emit error(err_msg.left(MAX_DEBUG_LEN));
//                logger->write_error(player_id, error);
                return;
            }
            if (! keys.contains("X") || ! keys.contains("Y")) {
                emit error("No required key 'X' or 'Y'");
                return;
            }

            double x = json.value("X").toDouble(0.0);
            double y = json.value("Y").toDouble(0.0);

            Direct result(x, y);
            if (keys.contains("Split")) {
                result.split = json.value("Split").toBool(false);
            }
            if (keys.contains("Eject")) {
                result.eject = json.value("Eject").toBool(false);
            }
            if (keys.contains("Debug")) {
                QString msg = json.value("Debug").toString("");
                if (msg != "") {
                    msg = msg.left(MAX_DEBUG_LEN);
                    emit debug(msg);
                }
            }
            if (keys.contains("Sprite")) {
                QJsonObject spriteJson = json.value("Sprite").toObject();
                if (! spriteJson.empty()) {
                    QString player = spriteJson.value("Id").toString("");
                    QString msg = spriteJson.value("S").toString("");
                    if (player != "" && msg != "") {
                        player = player.left(MAX_ID_LEN);
                        msg = msg.left(MAX_DEBUG_LEN);
                        emit sprite(player, msg);
                    }
                }
            }
            emit response(result);
        }
    }

    QJsonObject parse_answer(QByteArray &data) {
        QJsonObject empty;
        if (data.length() < 3) {
            emit error("Incorrect response (len < 3)");
            return empty;
        }
        if (data[data.length() - 1] == '\n') {
            data = data.left(data.length() - 1);
        }

        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &err);
        if (jsonDoc.isNull()) {
            emit error("Incorrect response (" + err.errorString() + ")");
            return empty;
        }
        return jsonDoc.object();
    }

    void send_config() {
        QJsonDocument jsonDoc(Constants::instance().toJson());
        QString message = QString(jsonDoc.toJson(QJsonDocument::Compact)) + "\n";

        int sent = socket->write(message.toStdString().c_str());
        if (sent == 0) {
            emit error("Fatal error: can't send config");
        }
        dump_logger->write_raw(0, message);
        socket->flush();
    }

    void send_state(const PlayerArray &fragments, const CircleArray &visibles, int tick=0) {
        waiting = true;
        wait_timeout = 0;

        QString message = prepare_state(fragments, visibles);
        int sent = socket->write(message.toStdString().c_str());
        if (sent == 0) {
            emit error("Fatal error: can't send state");
        }
        dump_logger->write_raw(tick + 1, message);
        socket->flush();
        answered = false;
    }

    QString prepare_state(const PlayerArray &fragments, const CircleArray &visibles) {
        QJsonArray mineArray;
        for (Player *player : fragments) {
            mineArray.append(player->toJson(true));
        }
        QJsonArray objectsArray;
        for (Circle *circle : visibles) {
            objectsArray.append(circle->toJson());
        }
        QJsonObject json;
        json.insert("Mine", mineArray);
        json.insert("Objects", objectsArray);

        QJsonDocument jsonDoc(json);
        return QString(jsonDoc.toJson(QJsonDocument::Compact)) + "\n";
    }

    Logger *get_logger() const {
        return logger;
    }

    Logger *get_dump_logger() const {
        return dump_logger;
    }
};

typedef QVector<ClientWrapper*> ClientWrappers;

#endif // TCP_CONNECT_H
