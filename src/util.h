#pragma once

#include <QApplication>
#include <QDateTime>
#include <QStandardPaths>
#include <QString>
#include <QUrl>

#include <filesystem>
#include <iostream>

constexpr const char *date_string_format = "yyyy.MM.dd hh:mm:ss";
constexpr const uint32_t app_id = 370070;

inline void log(const std::string_view &message)
{
	std::cout << "[" << QDateTime::currentDateTime().toString(date_string_format).toStdString() << "] " << message << '\n';
}

inline void log_error(const std::string_view &error_message)
{
	std::cerr << "[" << QDateTime::currentDateTime().toString(date_string_format).toStdString() << "] " << error_message << std::endl;
}

inline void report_exception(const std::exception &exception)
{
	try {
		std::rethrow_if_nested(exception);
	} catch (const std::exception &nested_exception) {
		report_exception(nested_exception);
	}

	log_error(exception.what());
}

inline void log_qt_message(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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

inline std::filesystem::path get_user_data_path()
{
	std::filesystem::path path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
	if (path.empty()) {
		throw std::runtime_error("No user data path found.");
	}

	//ignore the organization name for the user data path, e.g. the path should be "[AppName]" and not "[OrganizationName]/[AppName]"
	if (path.parent_path().filename() == QApplication::organizationName().toStdString()) {
		path = path.parent_path().parent_path() / path.filename();
	}

	if (!std::filesystem::exists(path)) {
		const bool success = std::filesystem::create_directories(path);
		if (!success) {
			throw std::runtime_error("Failed to create user data path: \"" + path.string() + "\".");
		}
	}

	return path;
}

inline std::filesystem::path get_logs_path()
{
	const std::filesystem::path path = get_user_data_path() / "logs";

	if (!std::filesystem::exists(path)) {
		const bool success = std::filesystem::create_directories(path);
		if (!success) {
			throw std::runtime_error("Failed to create logs path: \"" + path.string() + "\".");
		}
	}

	return path;
}

inline std::filesystem::path get_error_log_filepath()
{
	std::filesystem::path filepath = get_logs_path() / "launcher_error.log";
	filepath.make_preferred();
	return filepath;
}

inline std::string to_string(const std::filesystem::path &path)
{
	//convert a path to a UTF-8 encoded string
	const std::u8string u8str = path.u8string();
	return std::string(u8str.begin(), u8str.end());
}

inline QString to_qstring(const std::filesystem::path &path)
{
	return QString::fromStdString(to_string(path));
}

inline std::filesystem::path to_path(const QString &path_str)
{
#ifdef USE_WIN32
	return std::filesystem::path(path_str.toStdU16String());
#else
	return std::filesystem::path(path_str.toStdString());
#endif
}

inline std::filesystem::path to_path(const QUrl &url)
{
	return to_path(url.toLocalFile());
}

inline std::vector<std::string> split_string(const std::string &str, const char delimiter)
{
	std::vector<std::string> string_list{};

	size_t start_pos = 0;
	size_t find_pos = 0;
	while ((find_pos = str.find(delimiter, start_pos)) != std::string::npos) {
		std::string string_element = str.substr(start_pos, find_pos - start_pos);

		string_list.push_back(std::move(string_element));

		start_pos = find_pos + 1;
	}

	std::string string_element = str.substr(start_pos, str.length() - start_pos);

	string_list.push_back(std::move(string_element));

	return string_list;
}
