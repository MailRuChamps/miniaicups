#include "mainwindow.h"
#include <QApplication>
#include "config.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Config configuration;
    Constants::initialize(env, configuration);

    MainWindow window(configuration);
    window.show();
    return a.exec();
}
