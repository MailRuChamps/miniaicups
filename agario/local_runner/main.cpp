#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    QApplication::setOrganizationName("MRG");
    QApplication::setOrganizationDomain("mail.ru");
    QApplication::setApplicationName("AgarLocalRunner");

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return a.exec();
}
