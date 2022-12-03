#include <brutus/error.h>
#include <brutus/string.h>

#include <pompeius/TranslationUnit.h>


const unsigned int TranslationUnit::default_parse_options = 
	CXTranslationUnit_None |
	CXTranslationUnit_DetailedPreprocessingRecord |
	//CXTranslationUnit_Incomplete |
	CXTranslationUnit_PrecompiledPreamble |
	CXTranslationUnit_CacheCompletionResults |
	//CXTranslationUnit_ForSerialization |
	//CXTranslationUnit_SkipFunctionBodies |
	CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
	CXTranslationUnit_CreatePreambleOnFirstParse |
	//CXTranslationUnit_KeepGoing |
	//CXTranslationUnit_SingleFileParse |
	//CXTranslationUnit_LimitSkipFunctionBodiesToPreamble |
	//CXTranslationUnit_IncludeAttributedTypes |
	//CXTranslationUnit_VisitImplicitAttributes |
	CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles |
	//CXTranslationUnit_RetainExcludedConditionalBlocks |
0;


const unsigned int TranslationUnit::complete_options =
	CXCodeComplete_IncludeMacros |
	CXCodeComplete_IncludeCodePatterns |
	CXCodeComplete_IncludeBriefComments |
	//CXCodeComplete_SkipPreamble |
	CXCodeComplete_IncludeCompletionsWithFixIts |
0;


TranslationUnit::TranslationUnit(std::shared_ptr<const Index> index, const std::string &main_file_, const std::vector<std::string> &compile_commands_):
	index(index),
	main_file(main_file_),
	compile_commands(compile_commands_)
{
}


std::unique_lock<std::mutex> TranslationUnit::with() const{
	return std::unique_lock<std::mutex>(mutex);
}


bool TranslationUnit::try_with(std::unique_lock<std::mutex> &lock) const{
	std::unique_lock<std::mutex> new_lock(mutex, std::try_to_lock);
	if(new_lock.owns_lock()){
		std::swap(lock, new_lock);
		return true;
	}
	return false;
}


void TranslationUnit::set_parser(TUParser *p){
	parser = p;
}


void TranslationUnit::set_parent(TUParent *p){
	parent = p;
}


std::string TranslationUnit::get_main_file() const{
	// main_file is const
	return main_file;
}


std::vector<std::string> TranslationUnit::get_compile_commands() const{
	// compile_commands is const
	return compile_commands;
}


std::set<std::string> TranslationUnit::get_included_files() const{
	std::lock_guard<std::mutex> lock(mutex_parsed_info);
	return included_files;
}


void TranslationUnit::file_modified(const std::string &file, const std::string &content){
	std::lock_guard<std::mutex> lock(mutex);
	unsaved_files[file] = content;
	switch_state(state_e::OUTDATED);
}


void TranslationUnit::file_unmodified(const std::string &file){
	std::lock_guard<std::mutex> lock(mutex);
	auto find = unsaved_files.find(file);
	if(find != unsaved_files.end()){
		unsaved_files.erase(find);
		switch_state(state_e::OUTDATED);
	}
}


void TranslationUnit::parse(){
	std::lock_guard<std::mutex> lock(mutex);
	const state_e old_state = state;
	switch_state(state_e::PARSING);

	std::vector<CXUnsavedFile> unsaved_files_for_clang = get_unsaved_files_for_clang();

	if(old_state == state_e::NOT_INITIALIZED){
		const int parse_options = default_parse_options;
		std::vector<const char *> commands_for_clang;
		for(const auto &arg : compile_commands){
			commands_for_clang.push_back(arg.c_str());
		}

		const CXErrorCode error = clang_parseTranslationUnit2(*index, NULL, commands_for_clang.data(), commands_for_clang.size(), unsaved_files_for_clang.data(), unsaved_files_for_clang.size(), parse_options, unit);
		if(error != CXError_Success){
			throw std::runtime_error("Error parsing translation unit: " + cx_error_string(error));
		}
	}else{
		const unsigned int parse_options = clang_defaultReparseOptions(unit);
		int ret = clang_reparseTranslationUnit(unit, unsaved_files_for_clang.size(), unsaved_files_for_clang.data(), parse_options);
		if(ret != 0){
			throw std::runtime_error("Error reparsing translation unit: " + cx_error_string(static_cast<CXErrorCode>(ret)));
		}
	}

	read_parsed_info();
	switch_state(state_e::READY);
}


void TranslationUnit::parse_async(){
	if(!parser){
		throw std::logic_error("No parsing service set");
	}
	parser->parse_this(this);
}


void TranslationUnit::suspend(){
	std::lock_guard<std::mutex> lock(mutex);
	ensure_ready();

	clang_suspendTranslationUnit(unit);
	switch_state(state_e::SUSPENDED);
}


bool TranslationUnit::is_initialized() const{
	std::lock_guard<std::mutex> lock(mutex);
	return state != state_e::NOT_INITIALIZED;
}


bool TranslationUnit::is_ready() const{
	std::unique_lock<std::mutex> lock;
	if(try_with(lock)){
		return (state == state_e::READY);
	}
	return false;
}


bool TranslationUnit::is_ready(const std::unique_lock<std::mutex> &lock) const{
	check_lock(lock);
	return (state == state_e::READY);
}


bool TranslationUnit::is_accessible() const{
	std::unique_lock<std::mutex> lock;
	if(try_with(lock)){
		return state != state_e::PARSING;
	}
	return false;
}


bool TranslationUnit::is_accessible(const std::unique_lock<std::mutex> &lock) const{
	check_lock(lock);
	return (state != state_e::PARSING);
}


cursor_t TranslationUnit::get_location(const std::string &file, size_t row, size_t column) const{
	return get_location(with(), file, row, column);
}


cursor_t TranslationUnit::get_location(const std::unique_lock<std::mutex> &lock, const std::string &file, size_t row, size_t column) const{
	check_lock(lock);
	ensure_ready();
	CXFile handle = clang_getFile(unit, file.c_str());
	if(!handle){
		throw std::runtime_error("no file handle for file " + file);
	}

	auto loc = clang_getLocation(unit, handle, row, column);
	return convert(clang_getCursor(unit, loc));
}


cursor_t TranslationUnit::get_location(const source_location_t &location) const{
	return get_location(location.file, location.row, location.column);
}


cursor_t TranslationUnit::get_location(const std::unique_lock<std::mutex> &lock, const source_location_t &location) const{
	return get_location(lock, location.file, location.row, location.column);
}


std::vector<completion_t> TranslationUnit::code_complete(const std::string &file, size_t row, size_t column) const{
	std::lock_guard<std::mutex> lock(mutex);

	std::vector<CXUnsavedFile> unsaved_files_for_clang = get_unsaved_files_for_clang();
	return convert(clang_codeCompleteAt(unit, file.c_str(), row, column, unsaved_files_for_clang.data(), unsaved_files_for_clang.size(), complete_options));
}


std::vector<completion_t> TranslationUnit::code_complete(const source_location_t &location) const{
	return code_complete(location.file, location.row, location.column);
}


std::vector<diagnostic_t> TranslationUnit::get_diagnostics() const{
	std::lock_guard<std::mutex> lock(mutex_parsed_info);
	return diagnostics;
}


ASTReferences TranslationUnit::get_references(const cursor_t &cursor, const std::string &file) const{
	return get_references(with(), cursor, file);
}


ASTReferences TranslationUnit::get_references(const std::unique_lock<std::mutex> &lock, const cursor_t &cursor, const std::string &file) const{
	check_lock(lock);
	ensure_ready();
	visit_find_references_data_t client_data;
	client_data.file = file;
	client_data.find_usr = cursor.get_usr_ref();
	if(!client_data.find_usr.empty()){
		clang_visitChildren(root_cursor_internal(), visit_find_references, &client_data);
	}

	return client_data.references;
}


ASTReferences TranslationUnit::get_all_references(const cursor_t &cursor) const{
	return get_references(cursor, "");
}


std::vector<cursor_t> TranslationUnit::get_cursors(const std::string &file) const{
	return get_cursors(with(), file);
}


std::vector<cursor_t> TranslationUnit::get_cursors(const std::unique_lock<std::mutex> &lock, const std::string &file) const{
	check_lock(lock);
	ensure_ready();
	visit_get_cursors_data_t client_data;
	client_data.file = file;
	clang_visitChildren(root_cursor_internal(), visit_get_cursors, &client_data);

	return client_data.result;
}


std::vector<cursor_t> TranslationUnit::get_all_cursors() const{
	return get_cursors("");
}


CXChildVisitResult TranslationUnit::visit_get_cursors(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data){
	auto &data = *reinterpret_cast<visit_get_cursors_data_t *>(client_data);

	if(!data.file.empty()){
		// clang_getCursorLocation() seems to have a bug, see cursor_t.cpp
		const auto location = convert(clang_getRangeStart(clang_getCursorExtent(cursor)));
		if(location.file != data.file){
			return CXChildVisit_Continue;
		}
	}

	data.result.push_back(convert(cursor));

	return CXChildVisit_Recurse;
}


CXChildVisitResult TranslationUnit::visit_find_references(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data){
	auto &data = *reinterpret_cast<visit_find_references_data_t *>(client_data);

	if(!data.file.empty()){
		// clang_getCursorLocation() seems to have a bug, see cursor_t.cpp
		std::string cursor_file = convert(clang_getCursorExtent(cursor)).begin.file;
		if(cursor_file != data.file){
			return CXChildVisit_Continue;
		}
	}

	cursor_t curs = convert(cursor);
	if(curs.get_usr_ref() == data.find_usr){
		data.references.add(std::move(curs));
	}

	return CXChildVisit_Recurse;
}


void TranslationUnit::visit_includes(CXFile included_file, CXSourceLocation */*inclusion_stack*/, unsigned int /*include_len*/, CXClientData client_data){
	auto &data = *reinterpret_cast<visit_includes_data_t *>(client_data);

	std::string filename = convert(clang_getFileName(included_file));
	if(filename != data.main_file){
		data.included_files->insert(filename);
	}
}


void TranslationUnit::check_lock(const std::unique_lock<std::mutex> &lock) const{
	if(lock.mutex() != &mutex){
		throw std::logic_error("Wrong lock");
	}
	if(!lock.owns_lock()){
		throw std::logic_error("Lock not owned");
	}
}


// Internal function. Mutex must be locked.
void TranslationUnit::switch_state(const state_e &new_state){
	const bool was_init = (state != state_e::NOT_INITIALIZED);
	const bool was_ready = (state == state_e::READY);
	state = new_state;
	const bool is_ready = (state == state_e::READY);
	const bool is_init = (state != state_e::NOT_INITIALIZED);
	if(parent){
		parent->state_changed(this, was_init, was_ready, is_init, is_ready);
	}
}


// Internal function. Mutex must be locked.
void TranslationUnit::ensure_ready() const{
	if(state == state_e::NOT_INITIALIZED){
		throw std::logic_error("Translation unit is not initialized");
	}else if(state == state_e::SUSPENDED){
		throw std::logic_error("Translation unit is suspended");
	}else if(state == state_e::OUTDATED){
		throw std::runtime_error("Translation unit is outdated");
	}
}


// Internal function. Mutex must be locked and translation unit must be ready.
CXCursor TranslationUnit::root_cursor_internal() const{
	return clang_getTranslationUnitCursor(unit);
}


// Internal function. Mutex must be locked.
// The contents of the returned vector is unsafe to use when unsaved_files changes.
std::vector<CXUnsavedFile> TranslationUnit::get_unsaved_files_for_clang() const{
	std::vector<CXUnsavedFile> result;
	for(const auto &entry : unsaved_files){
		CXUnsavedFile file;
		file.Filename = entry.first.c_str();
		file.Contents = entry.second.c_str();
		file.Length = entry.second.size();
		result.push_back(file);
	}
	return result;
}


// Internal function. Mutex must be locked and translation unit must be ready.
void TranslationUnit::read_parsed_info(){
	// diagnostics
	const std::vector<diagnostic_t> diagnostics_tmp = convert(clang_getDiagnosticSetFromTU(unit));

	//inclusions
	std::set<std::string> included_files_tmp;
	visit_includes_data_t client_data{
		main_file,
		&included_files_tmp,
	};
	clang_getInclusions(unit, visit_includes, &client_data);

	{
		std::lock_guard<std::mutex> lock(mutex_parsed_info);
		diagnostics = std::move(diagnostics_tmp);
		included_files = std::move(included_files_tmp);
	}
}


