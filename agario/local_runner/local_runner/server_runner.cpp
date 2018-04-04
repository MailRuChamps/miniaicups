#include "tcp_server.h"
#include <iostream>

#include <QCoreApplication>


int main(int argc, char *argv[]) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    QString result_path = env.value("GAME_LOG_LOCATION");
    if (result_path == "") {
        qDebug() << "GAME_LOG_LOCATION not specified";
        return 0;
    }
    QString client_cnt = env.value("CLIENT_CNT", "4");
    QCoreApplication a(argc, argv);

    TcpServer server(result_path, client_cnt.toInt());
    server.bind(HOST, PORT);

    QObject::connect(&server, SIGNAL(game_finished()), &a, SLOT(quit()));
    return a.exec();
}
