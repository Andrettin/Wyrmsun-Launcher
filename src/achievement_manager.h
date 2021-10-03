#pragma once

#include <QApplication>
#include <QTimer>

#include <filesystem>

class achievement_manager final
{
public:
	static constexpr int initial_check_interval_ms = 10000;
	static constexpr int check_interval_ms = 1000;

	static std::filesystem::path get_achievements_filepath();

	achievement_manager()
	{
	}

	~achievement_manager()
	{
		if (this->timer != nullptr) {
			this->timer->deleteLater();
		}
	}

	void start_continuous_checking()
	{
		this->timer = new QTimer(QApplication::instance());
		QObject::connect(this->timer, &QTimer::timeout, [this]() {
			this->check_achievements();
		});
		this->timer->setSingleShot(true);
		this->timer->start(achievement_manager::initial_check_interval_ms);
	}

	void check_achievements();

private:
	QTimer *timer = nullptr;
	std::filesystem::file_time_type previous_last_modified; //the last modified time for the previous achievements check
};
