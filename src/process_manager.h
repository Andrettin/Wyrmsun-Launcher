#pragma once

#include <QApplication>
#include <QProcess>

class process_manager final : public QObject
{
	Q_OBJECT

public:
	process_manager()
	{
		this->process = new QProcess;
		connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &process_manager::on_finished);
	}

	~process_manager()
	{
		if (this->process != nullptr) {
			this->process->deleteLater();
		}
	}

	Q_INVOKABLE void start()
	{
		this->process->start("wyrmsun", QStringList());
	}

	void on_finished(const int exit_code, const QProcess::ExitStatus exit_status)
	{
		Q_UNUSED(exit_status)

		QMetaObject::invokeMethod(QApplication::instance(), [exit_code] { QApplication::exit(exit_code); }, Qt::QueuedConnection);
		this->process->deleteLater();
		this->process = nullptr;
	}

private:
	QProcess *process = nullptr;
};
