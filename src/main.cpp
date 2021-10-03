#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <filesystem>
#include <iostream>

constexpr const char *date_string_format = "yyyy.MM.dd hh:mm:ss";

static void log(const std::string_view &message)
{
	std::cout << "[" << QDateTime::currentDateTime().toString(date_string_format).toStdString() << "] " << message << '\n';
}

static void log_error(const std::string_view &error_message)
{
	std::cerr << "[" << QDateTime::currentDateTime().toString(date_string_format).toStdString() << "] " << error_message << std::endl;
}

static void report_exception(const std::exception &exception)
{
	try {
		std::rethrow_if_nested(exception);
	} catch (const std::exception &nested_exception) {
		report_exception(nested_exception);
	}

	log_error(exception.what());
}

static void log_qt_message(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	std::string log_message;

	switch (type) {
		case QtDebugMsg:
			log_message += "Debug";
			break;
		case QtInfoMsg:
			log_message += "Info";
			break;
		case QtWarningMsg:
			log_message += "Warning";
			break;
		case QtCriticalMsg:
			log_message += "Critical";
			break;
		case QtFatalMsg:
			log_message += "Fatal";
			break;
	}

	log_message += ": ";

	static const std::string default_category_name = "default";
	if (context.category != nullptr && context.category != default_category_name) {
		log_message += context.category;
		log_message += ": ";
	}

	log_message += msg.toStdString();

	if (context.file != nullptr) {
		log_message += " (";
		log_message += context.file;
		log_message += ": ";
		log_message += context.line;

		if (context.function != nullptr) {
			log_message += ", ";
			log_message += context.function;
		}

		log_message += ")";
	}

	switch (type) {
		case QtDebugMsg:
		case QtInfoMsg:
			log(log_message);
			break;
		case QtWarningMsg:
		case QtCriticalMsg:
		case QtFatalMsg:
			log_error(log_message);
			break;
	}
}

int main(int argc, char **argv)
{
	try {
		qInstallMessageHandler(log_qt_message);

		QApplication app(argc, argv);
		app.setApplicationName("Wyrmsun Launcher");
		app.setOrganizationName("Wyrmsun");
		app.setOrganizationDomain("andrettin.github.io");

		const std::filesystem::path root_path = std::filesystem::current_path();
		const QString root_path_qstr = QString::fromUtf8(reinterpret_cast<const char *>(root_path.u8string().c_str()));

		app.setWindowIcon(QIcon(root_path_qstr + "/graphics/interface/icons/wyrmsun_icon_128_background.png"));

		//set cursor
		const int scale_factor = 2;
		const QPixmap pixmap = QPixmap::fromImage(QImage(root_path_qstr + "/graphics/cursors/dwarven/dwarven_gauntlet" + (scale_factor > 1 ? QString::fromStdString("_" + std::to_string(scale_factor) + "x") : "") + ".png"));
		const QPoint hot_pos(3 * scale_factor, 1 * scale_factor);
		const QCursor qcursor(pixmap, hot_pos.x(), hot_pos.y());

		QApplication::setOverrideCursor(qcursor);

		QQmlApplicationEngine engine;

		engine.addImportPath(root_path_qstr + "/libraries/qml");

		QUrl url = QDir(root_path_qstr + "/interface/").absoluteFilePath("Launcher.qml");
		url.setScheme("file");
		QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
			[url](QObject *obj, const QUrl &objUrl) {
				if (!obj && url == objUrl) {
					QCoreApplication::exit(-1);
				}
			}, Qt::QueuedConnection);
		engine.load(url);

		const int result = app.exec();

		return result;
	} catch (const std::exception &exception) {
		report_exception(exception);
		return -1;
	}
}
