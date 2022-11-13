#include <brutus/fixit_t.h>


fixit_t make_fixit(const CXSourceRange &range, const std::string &replace){
	fixit_t result;
	result.range = convert(range);
	result.replace = replace;
	return result;
}

