#pragma once

#include <brutus/location_type_e.h>

#include <string>

#include <clang-c/Index.h>


struct source_location_t{
	std::string file;
	size_t row;           // 1-based
	size_t column;        // 1-based
	ssize_t offset;       // 0-based, -1 if not available

	bool equals(const source_location_t &other) const;
};

source_location_t convert(const CXSourceLocation &location, const location_type_e &type=location_type_e::SPELLING_LOCATION);


