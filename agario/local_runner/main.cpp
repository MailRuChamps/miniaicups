#include "mainclass.h"
#include <QtCore/QCoreApplication>


int main(int argc, char *argv[]) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    QCoreApplication a(argc, argv);

    MainClass game;
    game.play_game();
    if (!game.isGameRun()) a.quit();
    return 0;
}
