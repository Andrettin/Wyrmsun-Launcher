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

		const QString achievements_filepath_qstr = to_qstring(achievements_filepath);
		const QSettings data(achievements_filepath_qstr, QSettings::IniFormat);

		ISteamUserStats *user_stats = SteamUserStats();

		if (user_stats == nullptr) {
			throw std::runtime_error("No Steam user information provided.");
		}

		for (QString key : data.childKeys()) {
			key.replace("_", "-");

			const std::string key_str = key.toStdString();

			bool unlocked = false;
			bool result = user_stats->GetAchievement(key_str.c_str(), &unlocked);

			if (!result) {
				log_error("Achievement \"" + key_str + "\" is not registered on Steam.");
				continue;
			}

			if (this->clear) {
				if (unlocked) {
					result = user_stats->ClearAchievement(key_str.c_str());

					if (!result) {
						log_error("Failed to clear achievement \"" + key_str + "\" on Steam.");
					}
				}
			} else {
				if (!unlocked) {
					result = user_stats->SetAchievement(key_str.c_str());

					if (!result) {
						log_error("Failed to unlock achievement \"" + key_str + "\" on Steam.");
					}
				}
			}
		}

		user_stats->StoreStats();

		this->previous_last_modified = last_modified;

		if (this->timer != nullptr && this->timer->isSingleShot()) {
			//the initial check interval has gone by, now set the timer to recurrently check, with a smaller interval
			this->timer->setSingleShot(false);
			this->timer->start(achievement_manager::check_interval_ms);
		}
	} catch (const std::exception &exception) {
		report_exception(exception);
	}
}
