#include <brutus/string.h>


std::string convert(CXString &&str){
	std::string result = "";
	const char *c_str = clang_getCString(str);
	if(c_str){
		result = c_str;
	}
	clang_disposeString(str);
	return result;
}


