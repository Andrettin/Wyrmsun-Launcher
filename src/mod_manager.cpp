#include "mod_manager.h"

#include "util.h"

#include "steam/isteamugc.h"

#include <filesystem>
#include <fstream>

QString mod_manager::upload_mod(const QUrl &mod_dir_url)
{
	this->mod_data = std::make_unique<::mod_data>();

	try {
		const std::filesystem::path mod_dir_path = to_path(mod_dir_url);

		if (!std::filesystem::exists(mod_dir_path)) {
			throw std::runtime_error("The mod directory does not exist.");
		}

		this->mod_data->path = mod_dir_path;

		this->parse_mod();

		const std::filesystem::path mod_id_filepath = this->get_mod_id_filepath();
		if (std::filesystem::exists(mod_id_filepath)) {
			this->read_mod_id();

			//if we already have a mod ID, we don't need to create the item, just update its contents
			this->update_mod_content();

			return QString();
		}

		ISteamUGC *ugc = SteamUGC();
		const SteamAPICall_t call_handle = ugc->CreateItem(app_id, EWorkshopFileType::k_EWorkshopFileTypeCommunity);
		this->mod_data->create_item_call_result.Set(call_handle, this, &mod_manager::on_item_created);

		return QString();
	} catch (const std::exception &exception) {
		report_exception(exception);

		return exception.what();
	}
}

void mod_manager::parse_mod()
{
	const std::filesystem::path mod_filepath = this->get_mod_filepath();

	if (!std::filesystem::exists(mod_filepath)) {
		throw std::runtime_error("The module.txt file is missing for the mod.");
	}

	std::ifstream ifstream(mod_filepath);

	if (!ifstream) {
		throw std::runtime_error("Failed to open the module.txt file for reading.");
	}

	std::string line;
	while (std::getline(ifstream, line)) {
		std::vector<std::string> tokens = split_string(line, ' ');

		if (tokens.size() < 3) {
			continue;
		}

		const std::string &key = tokens.at(0);
		const std::string &value = tokens.at(2);

		if (key == "name") {
			this->mod_data->name = value;
		}
	}

	if (this->mod_data->name.empty()) {
		throw std::runtime_error("The mod has no name.");
	}
}

void mod_manager::read_mod_id()
{
	const std::filesystem::path mod_id_filepath = this->get_mod_id_filepath();

	std::ifstream ifstream(mod_id_filepath);

	if (!ifstream) {
		throw std::runtime_error("Failed to open the mod_id.txt file for reading.");
	}

	std::string mod_id_str;
	ifstream >> mod_id_str;

	if (mod_id_str.empty()) {
		throw std::runtime_error("The mod_id.txt file does not contain an ID.");
	}

	this->mod_data->published_file_id = std::stoull(mod_id_str);
}

void mod_manager::write_mod_id(const uint64_t published_file_id)
{
	const std::filesystem::path mod_id_filepath = this->get_mod_id_filepath();

	std::ofstream ofstream(mod_id_filepath);

	if (!ofstream) {
		throw std::runtime_error("Failed to open the mod_id.txt file for writing.");
	}

	ofstream << published_file_id;
}

void mod_manager::update_mod_content()
{
	const uint64_t mod_id = this->mod_data->published_file_id;

	ISteamUGC *ugc = SteamUGC();
	const UGCUpdateHandle_t update_handle = ugc->StartItemUpdate(app_id, mod_id);

	bool success = ugc->SetItemTitle(update_handle, this->mod_data->name.c_str());

	if (!success) {
		throw std::runtime_error("Failed to set item title.");
	}
	
	success = ugc->SetItemContent(update_handle, to_string(this->mod_data->path).c_str());

	if (!success) {
		throw std::runtime_error("Failed to set item content.");
	}

	const SteamAPICall_t call_handle = ugc->SubmitItemUpdate(update_handle, nullptr);
	this->mod_data->update_item_call_result.Set(call_handle, this, &mod_manager::on_item_updated);
}

void mod_manager::process_call_result_code(const EResult result_code)
{
	switch (result_code) {
		case k_EResultOK:
			break;
		case k_EResultFail:
			throw std::runtime_error("Failed.");
		case k_EResultInvalidParam:
			throw std::runtime_error("Invalid parameter.");
		case k_EResultFileNotFound:
			throw std::runtime_error("File not found.");
		case k_EResultDuplicateName:
			throw std::runtime_error("An item with that name already exists.");
		case k_EResultAccessDenied:
			throw std::runtime_error("Access denied.");
		case k_EResultTimeout:
			throw std::runtime_error("Timeout.");
		case k_EResultBanned:
			throw std::runtime_error("Banned.");
		case k_EResultServiceUnavailable:
			throw std::runtime_error("Service unavailable.");
		case k_EResultNotLoggedOn:
			throw std::runtime_error("Not logged into Steam.");
		case k_EResultInsufficientPrivilege:
			throw std::runtime_error("Insufficient privilege.");
		case k_EResultLimitExceeded:
			throw std::runtime_error("Steam Cloud limit exceeded. Please remove some items and try again.");
		case k_EResultDuplicateRequest:
			throw std::runtime_error("Duplicate request.");
		case k_EResultLockingFailed:
			throw std::runtime_error("Failed to acquire UGC lock.");
		case k_EResultServiceReadOnly:
			throw std::runtime_error("The user is temporarily unable to upload new content to Steam.");
		default:
			throw std::runtime_error("Error code: " + std::to_string(static_cast<int>(result_code)) + ".");
	}
}

void mod_manager::on_item_created(CreateItemResult_t *result, const bool io_failure)
{
	try {
		if (io_failure) {
			throw std::runtime_error("I/O failure occurred.");
		}

		this->process_call_result_code(result->m_eResult);

		if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
			throw std::runtime_error("The user needs to accept the Steam Workshop legal agreement.");
		}

		this->write_mod_id(result->m_nPublishedFileId);

		this->mod_data->published_file_id = result->m_nPublishedFileId;

		this->update_mod_content();
	} catch (const std::exception &exception) {
		report_exception(exception);

		emit modUploadFailed(exception.what());
	}
}

void mod_manager::on_item_updated(SubmitItemUpdateResult_t *result, const bool io_failure)
{
	try {
		if (io_failure) {
			throw std::runtime_error("I/O failure occurred.");
		}

		this->process_call_result_code(result->m_eResult);

		if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
			throw std::runtime_error("The user needs to accept the Steam Workshop legal agreement.");
		}

		emit modUploadCompleted();
	} catch (const std::exception &exception) {
		report_exception(exception);

		emit modUploadFailed(exception.what());
	}
}
