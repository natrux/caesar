#pragma once

#include <brutus/diagnostic_severity_e.h>
#include <brutus/source_location_t.h>
#include <brutus/source_range_t.h>
#include <brutus/fixit_t.h>

#include <string>
#include <vector>

#include <clang-c/Index.h>


struct diagnostic_t{
	diagnostic_severity_e severity;
	source_location_t location;
	std::string message;
	std::string category_name;
	std::string command_line_option;
	std::vector<source_range_t> ranges;
	std::vector<fixit_t> fixits;
	std::vector<diagnostic_t> children;
};


// Note that this overload frees the set.
std::vector<diagnostic_t> convert(CXDiagnosticSet &&diagnostic_set);
// Note that this overload does not free the set.
std::vector<diagnostic_t> convert(const CXDiagnosticSet &diagnostic_set);

