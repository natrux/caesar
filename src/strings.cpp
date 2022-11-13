#include <util/strings.h>


std::vector<std::string> string_split(const std::string &str, char separator){
	std::vector<std::string> res;
	std::string tmp;
	for(char c : str) {
		if(c == separator) {
			res.push_back(tmp);
			tmp.clear();
		} else {
			tmp.push_back(c);
		}
	}
	res.push_back(tmp);
	return res;
}


bool compare_case_insensitive(const std::string &lhs, const std::string &rhs){
	return std::lexicographical_compare(
		lhs.begin(),
		lhs.end(),
		rhs.begin(),
		rhs.end(),
		[](const char &char1, const char &char2){
			return std::tolower(char1) < std::tolower(char2);
		}
	);
}


bool string_starts_with(const std::string &string, const std::string &starts_with){
	return string.rfind(starts_with, 0) != std::string::npos;
}


bool string_ends_with(const std::string &string, const std::string &ends_with){
	if(ends_with.length() > string.length()){
		return false;
	}
	return string.find(ends_with, string.length() - ends_with.length()) != std::string::npos;
}

