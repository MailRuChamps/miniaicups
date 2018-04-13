#ifndef CUSTOM_H
#define CUSTOM_H

#include "strategy.h"
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <logger.h>


class Custom : public Strategy
{
    Q_OBJECT

protected:
    QProcess *solution;
    bool is_running;
    QMetaObject::Connection finish_connection;
#ifdef CONSOLE_RUNNER
    Logger *logger;
    int tick;
#endif
signals:
    void error(QString);

public:
    explicit Custom(int _id, const QString &_path) :
        Strategy(_id),
        solution(new QProcess(this))
#ifdef CONSOLE_RUNNER
        ,logger(new Logger()),
        tick(0)
#endif
    {
#ifdef CONSOLE_RUNNER
        logger->init_file(QString::number(id), DUMP_FILE);
#endif
        solution->start(_path);
        finish_connection = connect(solution, SIGNAL(finished(int)), this, SLOT(on_finished(int)));
        connect(solution, SIGNAL(readyReadStandardError()), this, SLOT(on_error()));
        is_running = true;
        send_config();
    }

    virtual ~Custom() {
        Constants &ins = Constants::instance();
        if (solution) {
            disconnect(finish_connection);
            PlayerArray pa;
            CircleArray ca;
            QString message = prepare_state(pa, ca);
            int sent = solution->write(message.toStdString().c_str());
            solution->waitForBytesWritten(5000);
            bool success = solution->waitForReadyRead(ins.RESP_TIMEOUT * 1000);
            if (!success) {
                solution->waitForFinished(5000);
            }
            solution->close();
            delete solution;
        }
    }

    virtual Direct tickEvent(const PlayerArray &fragments, const CircleArray &objects) {
        if (! is_running) {
            return Direct(0, 0);
        }
        QString message = prepare_state(fragments, objects);
#ifndef CONSOLE_RUNNER
        qDebug() << message;
#else
        tick++;
        logger->write_raw(tick, message);
#endif
        int sent = solution->write(message.toStdString().c_str());
        if (sent == -1) {
            emit error("Can't write to process");
            return Direct(0, 0);
        }
        Constants &ins = Constants::instance();
        QByteArray cmdBytes = "";
        while (! cmdBytes.endsWith('\n')) {
            bool success = solution->waitForReadyRead(ins.RESP_TIMEOUT * 1000);
            if (! success) {
                cmdBytes.append(solution->readAllStandardOutput());
                cmdBytes.append(solution->readAllStandardError());
                cmdBytes.append(solution->readAll());
                qDebug() << cmdBytes;
                emit error("Can't wait for process answer (limit expired)");
                return Direct(0, 0);
            }
            cmdBytes.append(solution->readLine());
        }

        QJsonObject json = parse_answer(cmdBytes);
#ifdef CONSOLE_RUNNER
        QJsonDocument doc(json);
        logger->write_raw_with_old_tick(doc.toJson(QJsonDocument::Compact)+ "\n");
#endif
        QStringList keys = json.keys();
        if (! keys.contains("X") || ! keys.contains("Y")) {
            emit error("No X or Y keys in answer json");
            return Direct(0, 0);
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
        return result;
    }

public slots:
    void on_finished(int code) {
        is_running = false;
        emit error("Process finished with code " + QString::number(code));
    }

    void on_error() {
        QByteArray err_data =  solution->readAllStandardError();
        emit error(QString(err_data));
    }

public:
    void send_config() {
        QJsonDocument jsonDoc(Constants::instance().toJson());
        QString message = QString(jsonDoc.toJson(QJsonDocument::Compact)) + "\n";
#ifndef CONSOLE_RUNNER
        qDebug() << message;
#else
        logger->write_raw(0, message);
#endif
        int sent = solution->write(message.toStdString().c_str());
        if (sent == 0) {
            emit error("Can't write config to process");
        }
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

    QJsonObject parse_answer(QByteArray &data) {
        QJsonObject empty;
        if (data.length() < 3) {
            return empty;
        }
        if (data[data.length() - 1] == '\n') {
            data = data.left(data.length() - 1);
        }

        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &err);
        if (jsonDoc.isNull()) {
            return empty;
        }
        return jsonDoc.object();
    }
#ifdef CONSOLE_RUNNER
    Logger* get_looger(){
        return logger;
    }
#endif
};

#endif // CUSTOM_H
