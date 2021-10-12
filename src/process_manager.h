#pragma once

#include <QApplication>
#include <QProcess>

class achievement_manager;

class process_manager final : public QObject
{
	Q_OBJECT

public:
	explicit process_manager(const bool clear_achievements);
	~process_manager();

	Q_INVOKABLE void start();

	void on_finished(const int exit_code, const QProcess::ExitStatus exit_status);

private:
	QProcess *process = nullptr;
	std::unique_ptr<achievement_manager> achievement_manager;
	bool clear_achievements = false;
};
