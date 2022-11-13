#include <pompeius/Project.h>

#include <brutus/string.h>



Project::Project(const std::string &path_build_directory_):
	path_build_directory(path_build_directory_)
{
	update();
}


std::string Project::get_build_directory() const{
	return path_build_directory;
}


std::set<std::shared_ptr<TranslationUnit>> Project::get_translation_units() const{
	return tu_server.get_translation_units();
}


std::set<std::shared_ptr<TranslationUnit>> Project::get_translation_units(const std::string &file) const{
	return tu_server.get_translation_units(file);
}


TUObservable *Project::get_tu_server(){
	return &tu_server;
}


size_t Project::count_units() const{
	return tu_server.count_units();
}


size_t Project::count_init_units() const{
	return tu_server.count_init_units();
}


size_t Project::count_ready_units() const{
	return tu_server.count_ready_units();
}


void Project::update(){
	compilation_database.read_from_build_directory(path_build_directory);
	tu_server.update(compilation_database);
}


void Project::auto_parse(){
	tu_server.auto_parse();
}

