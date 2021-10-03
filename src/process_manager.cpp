#include "process_manager.h"

#include "achievement_manager.h"

process_manager::process_manager()
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
	this->process->start("wyrmsun", QStringList());
	this->achievement_manager = std::make_unique<::achievement_manager>();
}

void process_manager::on_finished(const int exit_code, const QProcess::ExitStatus exit_status)
{
	Q_UNUSED(exit_status)

	QMetaObject::invokeMethod(QApplication::instance(), [exit_code] { QApplication::exit(exit_code); }, Qt::QueuedConnection);
	this->process->deleteLater();
	this->process = nullptr;

	this->achievement_manager.reset();
}
