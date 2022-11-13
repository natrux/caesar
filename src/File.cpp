#include <pompeius/File.h>

#include <fstream>
#include <stdexcept>


File::File(const std::string &file_path):
	hasher(),
	path(file_path)
{
	load();
}


std::string File::get_path() const{
	return path;
}


void File::load(){
	std::ifstream stream(path);
	if(!stream){
		throw std::runtime_error("Opening file " + path + " failed");
	}

	const std::string text = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	file_size = text.size();
	file_hash = hasher(text);
	set_content(text);
}


void File::save(){
	std::ofstream stream(path);
	if(!stream){
		throw std::runtime_error("Writing to file " + path + " failed");
	}

	stream << content;
	file_size = content.size();
	file_hash = hasher(content);
	content_differs_file = false;
	report_modified();
}


void File::set_content(const std::string &content_){
	content = content_;
	content_differs_file = (content.size() != file_size) || (hasher(content) != file_hash);
	report_modified();
}


std::string File::get_content() const{
	return content;
}


bool File::is_dirty() const{
	return content_differs_file;
}


std::unique_lock<std::mutex> File::with_tu() const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->with();
}


bool File::try_with_tu(std::unique_lock<std::mutex> &lock) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->try_with(lock);
}


bool File::has_translation_unit() const{
	return (current_unit != nullptr);
}


bool File::has_translation_unit(const std::shared_ptr<TranslationUnit> &unit) const{
	return (current_unit == unit);
}


bool File::has_translation_unit_ready() const{
	if(!current_unit){
		return false;
	}
	return current_unit->is_ready();
}


bool File::has_translation_unit_ready(const std::unique_lock<std::mutex> &lock) const{
	if(!current_unit){
		return false;
	}
	return current_unit->is_ready(lock);
}


tu_file_match_e File::match_translation_unit(const std::shared_ptr<const TranslationUnit> &unit) const{
	if(!unit){
		return tu_file_match_e::NONE;
	}

	const std::string main_file = unit->get_main_file();
	if(main_file == path){
		return tu_file_match_e::MAIN_FILE;
	}

	const auto included_files = unit->get_included_files();
	const auto find = included_files.find(path);
	if(find != included_files.end()){
		// filename heuristic: if I am "..../foo.h" and the main file is "..../foo.cpp", we probably belong together.
		bool filename_heuristic = false;
		{
			size_t my_last_sep = path.rfind(path_sep);
			size_t their_last_sep = main_file.rfind(path_sep);
			const size_t my_dot = path.rfind('.');
			const size_t their_dot = main_file.rfind('.');
			if(my_last_sep == std::string::npos){
				my_last_sep = 0;
			}else{
				my_last_sep++;
			}
			if(their_last_sep == std::string::npos){
				their_last_sep = 0;
			}else{
				their_last_sep++;
			}
			if(my_dot != std::string::npos && their_dot != std::string::npos){
				const auto cmp = path.compare(my_last_sep, my_dot-my_last_sep, main_file, their_last_sep, their_dot-their_last_sep);
				filename_heuristic = (cmp == 0);
			}
		}
		if(filename_heuristic){
			return tu_file_match_e::INCLUDED_FILE_MATCH;
		}
		return tu_file_match_e::INCLUDED_FILE;
	}

	return tu_file_match_e::NONE;
}


void File::set_translation_unit(std::shared_ptr<TranslationUnit> unit){
	if(current_unit){
		current_unit->file_unmodified(path);
	}
	current_unit = unit;
	if(current_unit){
		report_modified();
		if(!current_unit->is_initialized()){
			current_unit->parse_async();
		}
	}
}


cursor_t File::get_cursor_at(const source_location_t &location) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_location(location);
}


cursor_t File::get_cursor_at(const std::unique_lock<std::mutex> &lock, const source_location_t &location) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_location(lock, location);
}


std::vector<cursor_t> File::get_cursors() const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_cursors(path);
}


std::vector<cursor_t> File::get_cursors(const std::unique_lock<std::mutex> &lock) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_cursors(lock, path);
}


ASTReferences File::get_references(const cursor_t &cursor) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_references(cursor, path);
}


ASTReferences File::get_references(const std::unique_lock<std::mutex> &lock, const cursor_t &cursor) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_references(lock, cursor, path);
}


std::vector<completion_t> File::code_complete(const source_location_t &location) const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->code_complete(location);
}


std::vector<diagnostic_t> File::get_diagnostics() const{
	if(!current_unit){
		throw std::logic_error("No translation unit set");
	}
	return current_unit->get_diagnostics();
}


void File::notify_new_tu(std::shared_ptr<TranslationUnit> /*unit*/){
	// Nothing to do.
}


void File::notify_deleted_tu(std::shared_ptr<TranslationUnit> /*unit*/){
	// Nothing to do, manage on higher level.
}


void File::notify_updated_tu(std::shared_ptr<TranslationUnit> unit){
	if(current_unit == unit){
		if(report_modified_buffered){
			report_modified();
		}
	}
}


void File::report_modified(){
	report_modified_buffered = false;
	if(current_unit){
		if(current_unit->is_ready()){
			if(is_dirty()){
				current_unit->file_modified(path, content);
			}else{
				current_unit->file_unmodified(path);
			}
			current_unit->parse_async();
		}else{
			report_modified_buffered = true;
		}
	}else{
		report_modified_buffered = true;
	}
}


