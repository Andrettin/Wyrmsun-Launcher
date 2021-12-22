#pragma once

#include "steam/isteamugc.h"
#include "steam/steam_api_common.h"

#include <QObject>
#include <QUrl>

#include <filesystem>

class mod_manager;

struct mod_data final
{
	std::filesystem::path path;
	std::string name;
	uint64_t published_file_id = 0;
	CCallResult<mod_manager, CreateItemResult_t> create_item_call_result;
	CCallResult<mod_manager, SubmitItemUpdateResult_t> update_item_call_result;
};

class mod_manager final : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString upload_mod(const QUrl &mod_dir_url);

private:
	std::filesystem::path get_mod_filepath() const
	{
		return this->mod_data->path / "module.txt";
	}

	std::filesystem::path get_mod_id_filepath() const
	{
		return this->mod_data->path / "mod_id.txt";
	}

	void parse_mod();
	void read_mod_id();
	void write_mod_id(const uint64_t published_file_id);
	void update_mod_content();
	void process_call_result_code(const EResult result_code);

	void on_item_created(CreateItemResult_t *result, const bool io_failure);
	void on_item_updated(SubmitItemUpdateResult_t *result, const bool io_failure);

signals:
	void modUploadCompleted();
	void modUploadFailed(const QString &error_message);

private:
	std::unique_ptr<mod_data> mod_data;
};
