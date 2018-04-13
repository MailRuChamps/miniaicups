#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <iostream>
#include <QDir>
#include "console_runner.h"

int main(int argc, char *argv[])
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    Constants::initialize(env);

    QCoreApplication a(argc, argv);

    QCommandLineParser parser;

    parser.setApplicationDescription("AgarIO console runner");
    parser.addHelpOption();
    parser.addPositionalArgument("dest", "destination directory for output files (required)", "<path to dir>");

    QCommandLineOption strategiesOption(QStringList() << "s" << "strategy", "strategy exec command (required)", "exec command");

    parser.addOption(strategiesOption);
    parser.process(QCoreApplication::arguments());

    QStringList args = parser.positionalArguments();
    QStringList strategies = parser.values(strategiesOption);

    if (args.isEmpty() || strategies.isEmpty()) {
        std::cout << "destination and strategies are required" << std::endl;
        return 0;
    };

    Constants::instance().LOG_DIR = args.first();

    ConsoleRunner console_runner(strategies);
    console_runner.run();
    return 0;
}
