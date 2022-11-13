#pragma once

#include <brutus/availability_e.h>
#include <brutus/fixit_t.h>
#include <brutus/completion_chunk_t.h>

#include <string>
#include <vector>

#include <clang-c/Index.h>


struct completion_t{
	CXCursorKind kind;
	availability_e availability;
	unsigned int priority;
	std::string brief_comment;
	std::vector<std::string> annotations;
	std::vector<fixit_t> fixits;
	std::vector<completion_chunk_t> chunks;

	std::string get_typed_text() const;

	static std::vector<availability_e> rank_availability;
	static std::vector<CXCursorKind> rank_cursor;

	// Returns true if a should be sorted strictly before b. Can be used directly for std::sort().
	static bool compare(const completion_t &a, const completion_t &b);
};

completion_t convert(const CXCompletionResult &completion_result);
std::vector<completion_t> convert(CXCodeCompleteResults *&&complete_results);


