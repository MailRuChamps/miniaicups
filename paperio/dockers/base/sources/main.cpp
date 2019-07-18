#include "tcp_client.h"

#include <QProcessEnvironment>
#include <QCoreApplication>

#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


int lookup_host(const char* host, char* addrstr) {
    struct addrinfo hints, *res;
    int errcode;
    void *ptr;

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, NULL, &hints, &res);
    if (errcode != 0) {
        return -1;
    }
    printf("Host: %s\n", host);
    while (res) {
        inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
        case AF_INET:
            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
            break;
        }
        inet_ntop(res->ai_family, ptr, addrstr, 100);
        printf("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        res = res->ai_next;
    }
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        qDebug() << "Usage: ./client <abs path to run.sh> <work_dir for run.sh>";
        return 0;
    }
    QString exec_path = argv[1];
    QString work_dir = argv[2];

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString solution_id = env.value("SOLUTION_ID");
    if (solution_id == "") {
        qDebug() << "solution_id must be specified!";
        return 0;
    }

    QString HOST = "127.0.0.1";
    QString env_value = env.value("WORLD_NAME");
    char host_ip[100];
    if (lookup_host(env_value.toStdString().c_str(), host_ip) == 0) {
        HOST = QString(host_ip);
    }
    else {
        qDebug() << "Can't determine host";
    }
    int PORT = 8000;

    QCoreApplication a(argc, argv);
    TcpClient client(HOST, PORT, solution_id.toInt(), exec_path, work_dir);
    qDebug() << "Sleeping (5 secs)...";
    sleep(5);

    qDebug() << "Starting client...";
    client.start();

    QObject::connect(&client, SIGNAL(finished()), &a,SLOT(quit()));
    return a.exec();
}
