#include "process_manager.h"

#include "achievement_manager.h"

process_manager::process_manager(const bool clear_achievements) : clear_achievements(clear_achievements)
{
	this->process = new QProcess;
	connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &process_manager::on_finished);
}

process_manager::~process_manager()
{
	if (this->process != nullptr) {
		this->process->deleteLater();
	}
}

void process_manager::start()
{
	this->achievement_manager = std::make_unique<::achievement_manager>(clear_achievements);
	this->achievement_manager->check_achievements();
	this->process->start("wyrmsun", QStringList());
}

void process_manager::on_finished(const int exit_code, const QProcess::ExitStatus exit_status)
{
	Q_UNUSED(exit_status)

	QMetaObject::invokeMethod(QApplication::instance(), [exit_code] { QApplication::exit(exit_code); }, Qt::QueuedConnection);
	this->process->deleteLater();
	this->process = nullptr;

	this->achievement_manager->check_achievements();
	this->achievement_manager.reset();
}
