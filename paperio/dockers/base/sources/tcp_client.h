#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "constants.h"
#include <unistd.h>

#include <QTcpSocket>
#include <QProcess>
#include <QFile>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>


class TcpClient : public QObject
{
    Q_OBJECT

private:
    QTcpSocket *socket;
    QString host;
    int port;
    int solution_id;
    bool first_read;

    QString solution_run;
    QProcess *solution;

signals:
    void finished();

public:
    TcpClient(const QString& _host, int _port, int _solution_id, QString exec, QString work_dir) :
        host(_host),
        port(_port),
        solution_id(_solution_id),
        first_read(true)
    {
        socket = new QTcpSocket(this);
        solution = new QProcess(this);
        solution->setWorkingDirectory(work_dir);

        solution_run = exec;
//        QFile run_file;
//        run_file.setFileName(exec);
//        if (run_file.open(QFile::ReadOnly)) {
//            solution_run = QString(run_file.readAll());
//        }
//        run_file.close();
    }

    virtual ~TcpClient() {
        if (socket) delete socket;
        if (solution) delete solution;
    }

    void start() {
        connect(socket, SIGNAL(connected()), SLOT(on_connected()));
        connect(socket, SIGNAL(readyRead()), SLOT(on_ready_read()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_error(QAbstractSocket::SocketError)));

        connect(solution, SIGNAL(readyReadStandardOutput()), SLOT(on_solution_out()));
        connect(solution, SIGNAL(readyReadStandardError()), SLOT(on_solution_error()));
        connect(solution, SIGNAL(finished(int)), SLOT(on_solution_finished(int)));
        socket->connectToHost(host, port);
    }

private slots:
    void on_connected() {
        QString greeting = "{\"solution_id\": \"" + QString::number(solution_id) + "\"}\n";
        int sent = socket->write(greeting.toStdString().c_str());
        if (sent == -1) {
            qDebug() << "Can not send solution_id";
        }
    }

    void on_ready_read() {
        while (socket->canReadLine()) {
            QByteArray &&data = socket->readLine(0);
            qDebug() << data;
            // qDebug() << "Data:" << data;
            if (first_read) {
                solution->start(solution_run);
                first_read = false;
            }
            int written = solution->write(data);
            // qDebug() << "Written" << written;
            if (written == -1) {
                qDebug() << "Can not write to solution";
            }
        }
    }

    void on_error(QAbstractSocket::SocketError error) {
        qDebug() << "Socket error:" << error;
        solution->close();
        socket->disconnectFromHost();
        emit finished();
    }

public slots:
    void on_solution_out() {
        while (solution->canReadLine()) {
            QByteArray &&cmd = solution->readLine(MAX_RESP_LEN);
            // qDebug() << cmd;

            int sent = socket->write(cmd);
            if (sent == -1) {
                qDebug() << "Can not send command";
            }
        }
    }

    void on_solution_error() {
        QByteArray error = solution->readAllStandardError();
        qDebug() << "Error:" << error;

        QJsonObject jsonError;
        jsonError.insert("error", QJsonValue(QString(error)));
        QJsonDocument jsonDoc(jsonError);
        QString cmd = QString(jsonDoc.toJson(QJsonDocument::Compact)) + "\n";

        int sent = socket->write(cmd.toStdString().c_str());
        if (sent == -1) {
            qDebug() << "Can not send error";
        }
    }

    void on_solution_finished(int code) {
        qDebug() << "Solution finished: code" << code;
        sleep(5);
        socket->disconnectFromHost();
        emit finished();
    }
};



#endif // TCP_CLIENT_H
