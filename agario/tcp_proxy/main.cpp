#include <QCommandLineParser>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRandomGenerator>
#include <QTcpSocket>

#include <QTimer>

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);
	QTcpSocket       sock(&app);

	QCoreApplication::setApplicationName("aicups_test_tcp_proxy");
	QCoreApplication::setApplicationVersion("1.0");

	QCommandLineParser cli_parser;
	cli_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
	cli_parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);

	QCommandLineOption host_option(QStringList{"H", "host"}, QCoreApplication::tr("Aicups server host"), "Host", "127.0.0.1");
	QCommandLineOption port_option(QStringList{"p", "port"}, QCoreApplication::tr("Aicups server port"), "Port", "9999");
	QCommandLineOption working_dir_option(QStringList{"w", "work-dir"}, QCoreApplication::tr("Strategy working directory"), "Working Directory", ".");

	cli_parser.addOptions(QList<QCommandLineOption>{host_option, port_option, working_dir_option});
	cli_parser.addPositionalArgument("strategy", QCoreApplication::tr("Strategy run command"), "<strategy> [args]");

	auto version_option = cli_parser.addVersionOption();
	auto help_option    = cli_parser.addHelpOption();

	cli_parser.process(app);

	if (cli_parser.isSet(help_option)) { cli_parser.showHelp(1); }

	QStringList args = cli_parser.positionalArguments();
	if (args.empty()) { cli_parser.showHelp(1); }

	QProcess process(&app);
	process.setProgram(args[0]);
	args.removeFirst();
	process.setArguments(args);
	QString working_dir = cli_parser.value(working_dir_option);
	if (!working_dir.isEmpty()) { process.setWorkingDirectory(working_dir); }

	quint16 port = cli_parser.value(port_option).toUShort();
	QString host = cli_parser.value(host_option);

	QRandomGenerator *rng = QRandomGenerator::system();
	app.connect(&sock, &QTcpSocket::connected, [&sock, rng] {
		qDebug() << "connected to server";
		QVariantMap json{{"solution_id", "solution_" + QString::number(rng->generate())}};
		QByteArray out_data = QJsonDocument::fromVariant(json).toJson(QJsonDocument::Compact);
		sock.write(out_data + "\n");
	});
	app.connect(&sock, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [&sock](QAbstractSocket::SocketError) {
		qDebug() << "socket error:" << sock.errorString();
		QCoreApplication::quit();
	});

	app.connect(&sock, &QTcpSocket::readyRead, [&process, &sock] {
		QByteArray data = sock.readAll();
		process.write(data);
	});

	app.connect(&sock, &QTcpSocket::disconnected, [] {
		qDebug() << "socket disconnected";
		QCoreApplication::quit();
	});

	app.connect(&process, &QProcess::readyReadStandardOutput, [&sock, &process] {
		QByteArray data = process.readAllStandardOutput();
		sock.write(data);
	});
	app.connect(&process, &QProcess::readyReadStandardError, [&process] {
		QByteArray data = process.readAllStandardError();
		qDebug().noquote() << data;
	});

	app.connect(&process, &QProcess::started, [] { qDebug() << "strategy process started"; });

	app.connect(&process, QOverload<int>::of(&QProcess::finished), [&app, &sock](int) {
		qDebug() << "process finished";
		sock.disconnectFromHost();
		app.quit();
	});

	app.connect(&app, &QCoreApplication::aboutToQuit, [&sock, &process] {
		qDebug() << "cleanup";
		sock.close();
		process.close();
	});

	process.start();
	sock.connectToHost(host, port);

	return app.exec();
}
