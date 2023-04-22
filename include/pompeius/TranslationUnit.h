#pragma once

#include <pompeius/ASTReferences.h>
#include <pompeius/TUParser.h>
#include <pompeius/TUParent.h>

#include <brutus/Index.h>
#include <brutus/CXTranslationUnit_Wrapper.h>
#include <brutus/diagnostic_t.h>
#include <brutus/completion_t.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <mutex>

#include <clang-c/Index.h>


class TranslationUnit{
public:
	static const unsigned int default_parse_options;
	static const unsigned int complete_options;

	TranslationUnit(std::shared_ptr<const Index> index, const std::string &main_file_, const std::vector<std::string> &compile_commands_);
	std::unique_lock<std::mutex> lock() const;
	std::unique_ptr<std::unique_lock<std::mutex>> try_lock() const;
	void set_parser(TUParser *p);
	void set_parent(TUParent *p);
	std::string get_main_file() const;
	std::vector<std::string> get_compile_commands() const;
	std::set<std::string> get_included_files() const;
	void file_modified(const std::string &file, const std::string &content);
	void file_unmodified(const std::string &file);
	void parse();
	void parse_async();
	void suspend();

	bool is_initialized() const;
	bool is_ready() const;
	bool is_ready(const std::unique_lock<std::mutex> &lock) const;
	bool is_accessible() const;
	bool is_accessible(const std::unique_lock<std::mutex> &lock) const;

	cursor_t get_location(const std::string &file, size_t row, size_t column) const;
	cursor_t get_location(const std::unique_lock<std::mutex> &lock, const std::string &file, size_t row, size_t column) const;
	cursor_t get_location(const source_location_t &location) const;
	cursor_t get_location(const std::unique_lock<std::mutex> &lock, const source_location_t &location) const;
	std::vector<completion_t> code_complete(const std::string &file, size_t row, size_t column) const;
	std::vector<completion_t> code_complete(const source_location_t &location) const;
	std::vector<diagnostic_t> get_diagnostics() const;
	ASTReferences get_references(const cursor_t &cursor, const std::string &file) const;
	ASTReferences get_references(const std::unique_lock<std::mutex> &lock, const cursor_t &cursor, const std::string &file) const;
	ASTReferences get_all_references(const cursor_t &cursor) const;
	std::vector<cursor_t> get_cursors(const std::string &file) const;
	std::vector<cursor_t> get_cursors(const std::unique_lock<std::mutex> &lock, const std::string &file) const;
	std::vector<cursor_t> get_all_cursors() const;

private:
	struct visit_find_references_data_t{
		std::string file;
		std::string find_usr;
		ASTReferences references;
	};
	struct visit_includes_data_t{
		std::string main_file;
		std::set<std::string> *included_files;
	};
	struct visit_get_cursors_data_t{
		std::string file;
		std::vector<cursor_t> result;
	};
	enum class state_e{
		NOT_INITIALIZED,
		READY,
		SUSPENDED,
		OUTDATED,
		PARSING,
	};
	mutable std::mutex mutex;
	mutable std::mutex mutex_parsed_info;
	state_e state = state_e::NOT_INITIALIZED;
	std::shared_ptr<const Index> index;
	CXTranslationUnit_Wrapper unit;
	const std::string main_file;
	const std::vector<std::string> compile_commands;
	std::map<std::string, std::string> unsaved_files;
	std::set<std::string> included_files;
	std::vector<diagnostic_t> diagnostics;
	TUParser *parser = nullptr;
	TUParent *parent = nullptr;

	static CXChildVisitResult visit_find_references(CXCursor cursor, CXCursor parent, CXClientData client_data);
	static CXChildVisitResult visit_get_cursors(CXCursor cursor, CXCursor parent, CXClientData client_data);
	static void visit_includes(CXFile included_file, CXSourceLocation *inclusion_stack, unsigned int include_len, CXClientData client_data);
	void check_lock(const std::unique_lock<std::mutex> &lock) const;
	void switch_state(const state_e &new_state);
	void ensure_ready() const;
	CXCursor root_cursor_internal() const;
	std::vector<CXUnsavedFile> get_unsaved_files_for_clang() const;
	void read_parsed_info();
};


