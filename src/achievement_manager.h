#pragma once

#include <filesystem>

class achievement_manager final
{
public:
	static std::filesystem::path get_achievements_filepath();

	achievement_manager()
	{
		this->check_achievements();
	}

	~achievement_manager()
	{
		//check achievements on start and exit, to prevent contention for the achievements file with Wyrmsun itself
		this->check_achievements();
	}

	void check_achievements();

private:
	std::filesystem::file_time_type previous_last_modified; //the last modified time for the previous achievements check
};
