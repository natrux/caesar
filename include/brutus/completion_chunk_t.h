#pragma once

#include <brutus/completion_chunk_type_e.h>

#include <string>
#include <vector>

#include <clang-c/Index.h>


struct completion_chunk_t{
	completion_chunk_type_e type;
	std::string text;
};

void chunk_completion(std::vector<completion_chunk_t> &result, const CXCompletionString &completion_string);


