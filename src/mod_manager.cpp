#include "mod_manager.h"

#include "util.h"

#include "steam/isteamugc.h"

#include <filesystem>
#include <thread>

QString mod_manager::upload_mod(const QUrl &mod_dir_url)
{
	const std::filesystem::path mod_dir_path = to_path(mod_dir_url);

	if (!std::filesystem::exists(mod_dir_path)) {
		return "The mod directory does not exist.";
	}

	const std::filesystem::path mod_file_path = mod_dir_path / "module.txt";

	if (!std::filesystem::exists(mod_file_path)) {
		return "The module.txt file is missing for the mod.";
	}

	ISteamUGC *ugc = SteamUGC();
	const SteamAPICall_t call_handle = ugc->CreateItem(app_id, EWorkshopFileType::k_EWorkshopFileTypeCommunity);
	this->create_item_call_result.Set(call_handle, this, &mod_manager::on_item_created);

	return QString();
}

void mod_manager::on_item_created(CreateItemResult_t *result, const bool io_failure)
{
	if (io_failure) {
		emit itemCreationFailed("I/O failure occurred.");
		return;
	}

	if (result->m_eResult != k_EResultOK) {
		QString error_message;

		switch (result->m_eResult) {
			case k_EResultInsufficientPrivilege:
				error_message = "Insufficient privilege.";
				break;
			case k_EResultBanned:
				error_message = "Banned.";
				break;
			case k_EResultTimeout:
				error_message = "Timeout.";
				break;
			case k_EResultNotLoggedOn:
				error_message = "Not logged into Steam.";
				break;
			case k_EResultServiceUnavailable:
				error_message = "Service unavailable.";
				break;
			case k_EResultInvalidParam:
				error_message = "Invalid parameter.";
				break;
			case k_EResultAccessDenied:
				error_message = "Access denied.";
				break;
			case k_EResultLimitExceeded:
				error_message = "Steam Cloud limit exceeded. Please remove some items and try again.";
				break;
			case k_EResultFileNotFound:
				error_message = "File not found.";
				break;
			case k_EResultDuplicateRequest:
				error_message = "Duplicate request.";
				break;
			case k_EResultDuplicateName:
				error_message = "An item with that name already exists.";
				break;
			case k_EResultServiceReadOnly:
				error_message = "User cannot upload new content to Steam.";
				break;
			default:
				error_message = "Error code: " + QString::number(static_cast<int>(result->m_eResult)) + ".";
				break;
		}

		emit itemCreationFailed(error_message);
		return;
	}

	emit itemCreated(result->m_nPublishedFileId);
}
