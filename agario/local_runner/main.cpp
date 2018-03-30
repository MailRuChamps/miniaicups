#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"

QSettings *ini___;
void createIniSettings()
{
    ::ini___ = new QSettings("settings.ini", QSettings::IniFormat);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    ::ini___->setIniCodec(codec);

    ::ini___->setValue("valid_params/color_1", "Красный"   );
    ::ini___->setValue("valid_params/color_2", "Зеленый"   );
    ::ini___->setValue("valid_params/color_3", "Синий"     );
    ::ini___->setValue("valid_params/color_4", "Желтый"    );
    ::ini___->setValue("valid_params/color_5", "Фиолетовый");
    ::ini___->setValue("valid_params/color_6", "Бирюзовый" );
    ::ini___->setValue("valid_params/types",   "local PC mouse");

    ::ini___->setValue("isSettingsButton", true);
    ::ini___->setValue("autoPauseEachTick", false);

    ::ini___->setValue("player1/name" , "PC1");
    ::ini___->setValue("player1/color", "Красный");
    ::ini___->setValue("player1/path" , "");
    ::ini___->setValue("player1/type" , "PC");

    ::ini___->setValue("player2/name" , "PC2");
    ::ini___->setValue("player2/color", "Зеленый");
    ::ini___->setValue("player2/path" , "");
    ::ini___->setValue("player2/type" , "PC");

    ::ini___->setValue("player3/name" , "PC3");
    ::ini___->setValue("player3/color", "Синий");
    ::ini___->setValue("player3/path" , "");
    ::ini___->setValue("player3/type" , "PC");

    ::ini___->setValue("player4/name" , "PC4");
    ::ini___->setValue("player4/color", "Фиолетовый");
    ::ini___->setValue("player4/path" , "");
    ::ini___->setValue("player4/type" , "PC");

    ::ini___->sync();
}

int qMain(int argc, char *argv[]) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    if (!QFile::exists("settings.ini")) createIniSettings();
    else
    {
        ::ini___ = new QSettings("settings.ini", QSettings::IniFormat);
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        ::ini___->setIniCodec(codec);
    }

    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return a.exec();
}
