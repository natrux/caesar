#pragma once

#include <map>
#include <vector>
#include <string>


class CompilationDatabase : public std::map<std::string, std::vector<std::vector<std::string>>>{
public:
	CompilationDatabase();
	void read_from_build_directory(const std::string &path);
	CompilationDatabase diffplus(const CompilationDatabase &other) const;

private:
	bool inject_include_directories = true;
	std::string clang_version;
};



