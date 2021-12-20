#pragma once

#include <QObject>
#include <QUrl>

class mod_manager final : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString upload_mod(const QUrl &mod_dir_url);
};
