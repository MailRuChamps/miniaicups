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
    QString solution_id;
    int player_id;
    bool is_ready;

    QByteArray got_data;

    // timeouts implementation
    int timerId;
    int wait_timeout;
    bool waiting;
    int sum_waiting;

public:
    bool is_canceled;

signals:
    void ready();
    void disconnected();
    void response(Direct);
    void error(QString, bool);
    void debug(QString);
    void sprite(QString, QString);

public:
    explicit ClientWrapper(QTcpSocket *_socket) :
        socket(_socket),
        logger(new Logger),
        is_ready(false),
        wait_timeout(0),
        waiting(false),
        sum_waiting(0),
        is_canceled(false)
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
    }

    int getId() const {
        return player_id;
    }

    QString get_solution_id() const {
        return solution_id;
    }

    void timerEvent(QTimerEvent *event) {

        if (event->timerId() == timerId && waiting && !is_canceled) {
            wait_timeout++;
            if (wait_timeout > Constants::instance().RESP_TIMEOUT * 10) {
                bool is_expired = accumulate_wait();
                if (is_expired) return;

                emit error(RESP_EXPIRED, true);
            }
        }
    }

    bool accumulate_wait() {
        waiting = false;
        sum_waiting += wait_timeout;
        wait_timeout = 0;

        if (sum_waiting > Constants::instance().SUM_RESP_TIMEOUT * 10) {
            is_canceled = true;
            emit error(SUM_RESP_EXPIRED, false);
            return true;
        }
        return false;
    }

public slots:
    void client_disconnected() {
        if (! is_canceled) {
            is_canceled = true;
            emit error(CLIENT_DISCONNECTED, false);
        }
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

        bool is_expired = accumulate_wait();
        if (is_expired) return;

        if (! is_ready) {
            QJsonObject json = parse_answer(got_data);
            if (json.isEmpty()) {
                emit error("Can't parse json: " + got_data, true);
                return;
            }
            got_data.clear();
            QStringList keys = json.keys();
            if (! keys.contains("solution_id")) {
                emit error("No required key 'solution_id'", true);
                return;
            }

            solution_id = json.value("solution_id").toString();
            logger->init_file(solution_id, true);
            is_ready = true;
            emit ready();
        }
        else {
            QJsonObject json = parse_answer(got_data);
            if (json.isEmpty()) {
                emit error("Can't parse json: " + got_data, true);
                return;
            }
            got_data.clear();
            QStringList keys = json.keys();
            if (keys.contains("error")) {
                QString err_msg = json.value("error").toString();
                emit error(err_msg.left(MAX_DEBUG_LEN), true);
//                logger->write_error(player_id, error);
                return;
            }
            if (! keys.contains("X") || ! keys.contains("Y")) {
                emit error("No required key 'X' or 'Y'", true);
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
            emit error("Incorrect response (len < 3)", true);
            return empty;
        }
        if (data[data.length() - 1] == '\n') {
            data = data.left(data.length() - 1);
        }

        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &err);
        if (jsonDoc.isNull()) {
            emit error("Incorrect response (" + err.errorString() + ")", true);
            return empty;
        }
        return jsonDoc.object();
    }

    void send_config() {
        QString message = prepare_config();
        int sent = socket->write(message.toStdString().c_str());
        if (sent == 0) {
            emit error("Fatal error: can't send config", true);
        }
        socket->flush();
    }

    void send_state(const PlayerArray &fragments, const CircleArray &visibles) {
        waiting = true;
        wait_timeout = 0;

        QString message = prepare_state(fragments, visibles);
        int sent = socket->write(message.toStdString().c_str());
        if (sent == 0) {
            emit error("Fatal error: can't send state", true);
        }
        socket->flush();
    }

    QString prepare_config() {
        Constants &ins = Constants::instance();
        QJsonObject jsonConfig;
        jsonConfig.insert("GAME_WIDTH", QJsonValue(ins.GAME_WIDTH));
        jsonConfig.insert("GAME_HEIGHT", QJsonValue(ins.GAME_HEIGHT));
        jsonConfig.insert("GAME_TICKS", QJsonValue(ins.GAME_TICKS));

        jsonConfig.insert("FOOD_MASS", QJsonValue(ins.FOOD_MASS));
        jsonConfig.insert("MAX_FRAGS_CNT", QJsonValue(ins.MAX_FRAGS_CNT));
        jsonConfig.insert("TICKS_TIL_FUSION", QJsonValue(ins.TICKS_TIL_FUSION));
        jsonConfig.insert("VIRUS_RADIUS", QJsonValue(ins.VIRUS_RADIUS));
        jsonConfig.insert("VIRUS_SPLIT_MASS", QJsonValue(ins.VIRUS_SPLIT_MASS));

        jsonConfig.insert("VISCOSITY", QJsonValue(ins.VISCOSITY));
        jsonConfig.insert("INERTION_FACTOR", QJsonValue(ins.INERTION_FACTOR));
        jsonConfig.insert("SPEED_FACTOR", QJsonValue(ins.SPEED_FACTOR));

        QJsonDocument jsonDoc(jsonConfig);
        return QString(jsonDoc.toJson(QJsonDocument::Compact)) + "\n";
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
};

typedef QVector<ClientWrapper*> ClientWrappers;

#endif // TCP_CONNECT_H
