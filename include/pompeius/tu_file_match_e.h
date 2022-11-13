#pragma once

#include <vector>


enum class tu_file_match_e{
	NONE,
	MAIN_FILE,
	INCLUDED_FILE,
	INCLUDED_FILE_MATCH,
};


static std::vector<tu_file_match_e> tu_file_match_rank = {
	tu_file_match_e::MAIN_FILE,
	tu_file_match_e::INCLUDED_FILE_MATCH,
	tu_file_match_e::INCLUDED_FILE,
	tu_file_match_e::NONE,
};

