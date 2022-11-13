#pragma once

#include <brutus/source_range_t.h>

#include <string>

#include <clang-c/Index.h>


struct fixit_t{
	source_range_t range;
	std::string replace;
};

fixit_t make_fixit(const CXSourceRange &range, const std::string &replace);

