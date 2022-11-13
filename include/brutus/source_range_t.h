#pragma once

#include <brutus/source_location_t.h>

#include <clang-c/Index.h>


struct source_range_t{
	source_location_t begin;
	source_location_t end;

	bool contains(const source_location_t &location) const;
	bool contains(const source_range_t &range) const;
	bool overlaps(const source_range_t &range) const;
};

source_range_t convert(const CXSourceRange &range, const location_type_e &type=location_type_e::SPELLING_LOCATION);


