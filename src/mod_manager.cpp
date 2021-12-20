#include "mod_manager.h"

#include "util.h"

#include <filesystem>

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

	return QString();
}
