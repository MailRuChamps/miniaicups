#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return a.exec();
}
