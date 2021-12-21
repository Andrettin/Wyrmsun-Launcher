#include "mod_manager.h"
#include "process_manager.h"
#include "util.h"

#include "steam/isteamuserstats.h"
#include "steam/steam_api.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include <filesystem>

static void init_output()
{
	const std::filesystem::path error_log_path = get_error_log_filepath();

	if (std::filesystem::exists(error_log_path) && std::filesystem::file_size(error_log_path) > 1000000) {
		std::filesystem::remove(error_log_path);
	}

	const std::u8string path_u8str = error_log_path.u8string();
	const std::string path_str(path_u8str.begin(), path_u8str.end());

	const FILE *error_log_file = freopen(path_str.c_str(), "a", stderr);
	if (error_log_file == nullptr) {
		log_error("Failed to create error log.");
	}
}

static void clean_output()
{
	std::cerr.clear();
	fclose(stderr);

	const std::filesystem::path error_log_path = get_error_log_filepath();

	if (std::filesystem::exists(error_log_path) && std::filesystem::file_size(error_log_path) == 0) {
		std::filesystem::remove(error_log_path);
	}
}

int main(int argc, char **argv)
{
	qInstallMessageHandler(log_qt_message);

	QApplication app(argc, argv);
	app.setApplicationName("Wyrmsun");
	app.setOrganizationName("Wyrmsun");
	app.setOrganizationDomain("andrettin.github.io");

	init_output();

	int result = 0;

	try {
		QCommandLineParser cmd_parser;

		const QCommandLineOption clear_option("clear-achievements", "Clear achievements, instead of setting them.");
		cmd_parser.addOption(clear_option);
		cmd_parser.process(*QApplication::instance());

		bool clear_achievements = false;
		if (cmd_parser.isSet(clear_option)) {
			clear_achievements = true;
		}

		const std::filesystem::path root_path = std::filesystem::current_path();
		const QString root_path_qstr = QString::fromUtf8(reinterpret_cast<const char *>(root_path.u8string().c_str()));

		app.setWindowIcon(QIcon(root_path_qstr + "/graphics/interface/icons/wyrmsun_icon_128_background.png"));

		//set cursor
		const int scale_factor = 2;
		const QPixmap pixmap = QPixmap::fromImage(QImage(root_path_qstr + "/graphics/cursors/dwarven/dwarven_gauntlet" + (scale_factor > 1 ? QString::fromStdString("_" + std::to_string(scale_factor) + "x") : "") + ".png"));
		const QPoint hot_pos(3 * scale_factor, 1 * scale_factor);
		const QCursor qcursor(pixmap, hot_pos.x(), hot_pos.y());

		QApplication::setOverrideCursor(qcursor);

		const bool initialized_steam = SteamAPI_Init();

		if (!initialized_steam) {
			log_error("Failed to initialize the Steam API.");
		}

		ISteamUserStats *user_stats = SteamUserStats();
		if (user_stats != nullptr) {
			user_stats->RequestCurrentStats();
		} else {
			log_error("No Steam user information provided.");
		}

		QQmlApplicationEngine engine;

		process_manager *process_manager = new ::process_manager(clear_achievements);
		engine.rootContext()->setContextProperty("process_manager", process_manager);

		mod_manager *mod_manager = new ::mod_manager;
		engine.rootContext()->setContextProperty("mod_manager", mod_manager);

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

		//run the Steam API callbacks intermittently
		QTimer run_callbacks_timer;
		run_callbacks_timer.setInterval(100);
		QObject::connect(&run_callbacks_timer, &QTimer::timeout, []() {
			SteamAPI_RunCallbacks();
		});
		run_callbacks_timer.start();

		result = app.exec();

		process_manager->deleteLater();

		if (initialized_steam) {
			SteamAPI_Shutdown();
		}
	} catch (const std::exception &exception) {
		report_exception(exception);
		result = -1;
	}

	clean_output();

	return result;
}
