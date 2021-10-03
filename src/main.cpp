#include "achievement_manager.h"
#include "process_manager.h"
#include "util.h"

#include "steam/isteamuserstats.h"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <filesystem>

int main(int argc, char **argv)
{
	try {
		qInstallMessageHandler(log_qt_message);

		QApplication app(argc, argv);
		app.setApplicationName("Wyrmsun");
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

		if (SteamUserStats() == nullptr) {
			throw std::runtime_error("No Steam user information provided.");
		}

		achievement_manager achievement_manager;

		QQmlApplicationEngine engine;

		process_manager *process_manager = new ::process_manager;
		engine.rootContext()->setContextProperty("process_manager", process_manager);

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

		process_manager->deleteLater();

		return result;
	} catch (const std::exception &exception) {
		report_exception(exception);
		return -1;
	}
}
