#include "achievement_manager.h"

#include "util.h"

#include "steam/isteamuserstats.h"

#include <QSettings>

std::filesystem::path achievement_manager::get_achievements_filepath()
{
	const std::filesystem::path user_data_path = get_user_data_path();

	std::filesystem::path filepath = user_data_path / "achievements.txt";
	filepath.make_preferred();
	return filepath;
}

void achievement_manager::check_achievements()
{
	try {
		const std::filesystem::path achievements_filepath = achievement_manager::get_achievements_filepath();

		if (!std::filesystem::exists(achievements_filepath)) {
			return;
		}

		const std::filesystem::file_time_type last_modified = std::filesystem::last_write_time(achievements_filepath);

		if (last_modified <= this->previous_last_modified) {
			//no change, nothing to check
			return;
		}

		const QString achievements_filepath_qstr = QString::fromUtf8(reinterpret_cast<const char *>(achievements_filepath.u8string().c_str()));
		const QSettings data(achievements_filepath_qstr, QSettings::IniFormat);

		ISteamUserStats *user_stats = SteamUserStats();

		if (user_stats == nullptr) {
			throw std::runtime_error("No Steam user information provided.");
		}

		for (QString key : data.childKeys()) {
			key.replace("_", "-");

			const std::string key_str = key.toStdString();

			bool unlocked = false;
			const bool result = user_stats->GetAchievement(key_str.c_str(), &unlocked);

			if (!result) {
				log_error("Achievement \"" + key_str + "\" is not registered on Steam.");
				continue;
			}

			if (!unlocked) {
				user_stats->SetAchievement(key_str.c_str());
			}
		}

		user_stats->StoreStats();

		this->previous_last_modified = last_modified;
	} catch (const std::exception &exception) {
		report_exception(exception);
	}
}
