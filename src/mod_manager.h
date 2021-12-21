#pragma once

#include "steam/isteamugc.h"
#include "steam/steam_api_common.h"

#include <QObject>
#include <QUrl>

class mod_manager final : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString upload_mod(const QUrl &mod_dir_url);

private:
	void on_item_created(CreateItemResult_t *result, const bool io_failure);

signals:
	void itemCreated(const uint64_t published_file_id);
	void itemCreationFailed(const QString &error_message);

private:
	CCallResult<mod_manager, CreateItemResult_t> create_item_call_result;
};
