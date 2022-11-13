#pragma once

#include <pompeius/TUServer.h>

#include <brutus/Index.h>
#include <brutus/CompilationDatabase.h>



class Project{
public:
	Project(const std::string &path_build_directory_);

	std::string get_build_directory() const;
	std::set<std::shared_ptr<TranslationUnit>> get_translation_units() const;
	std::set<std::shared_ptr<TranslationUnit>> get_translation_units(const std::string &file) const;
	TUObservable *get_tu_server();
	size_t count_units() const;
	size_t count_init_units() const;
	size_t count_ready_units() const;
	void update();
	void auto_parse();

private:
	std::string path_build_directory;
	CompilationDatabase compilation_database;
	TUServer tu_server;
};

