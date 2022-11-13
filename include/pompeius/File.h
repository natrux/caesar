#pragma once


#include <pompeius/TranslationUnit.h>
#include <pompeius/TUObserver.h>
#include <pompeius/tu_file_match_e.h>

#include <string>
#include <vector>
#include <set>
#include <memory>


class File : public TUObserver{
public:
	File(const std::string &file_path);
	std::string get_path() const;
	void load();
	void save();
	void set_content(const std::string &content);
	std::string get_content() const;
	bool is_dirty() const;
	std::unique_lock<std::mutex> with_tu() const;
	bool try_with_tu(std::unique_lock<std::mutex> &lock) const;
	bool has_translation_unit() const;
	bool has_translation_unit(const std::shared_ptr<TranslationUnit> &unit) const;
	bool has_translation_unit_ready() const;
	bool has_translation_unit_ready(const std::unique_lock<std::mutex> &lock) const;
	tu_file_match_e match_translation_unit(const std::shared_ptr<const TranslationUnit> &unit) const;
	void set_translation_unit(std::shared_ptr<TranslationUnit> unit);
	cursor_t get_cursor_at(const source_location_t &location) const;
	cursor_t get_cursor_at(const std::unique_lock<std::mutex> &lock, const source_location_t &location) const;
	std::vector<cursor_t> get_cursors() const;
	std::vector<cursor_t> get_cursors(const std::unique_lock<std::mutex> &lock) const;
	ASTReferences get_references(const cursor_t &cursor) const;
	ASTReferences get_references(const std::unique_lock<std::mutex> &lock, const cursor_t &cursor) const;
	std::vector<completion_t> code_complete(const source_location_t &location) const;
	std::vector<diagnostic_t> get_diagnostics() const;

	void notify_new_tu(std::shared_ptr<TranslationUnit> unit) override;
	void notify_deleted_tu(std::shared_ptr<TranslationUnit> unit) override;
	void notify_updated_tu(std::shared_ptr<TranslationUnit> unit) override;

private:
#ifdef _WIN32
	const char path_sep = '\\';
#else
	const char path_sep = '/';
#endif
	const std::hash<std::string> hasher;
	size_t file_size = 0;
	size_t file_hash = -1;
	bool content_differs_file = false;
	std::string path;
	std::string content;
	bool report_modified_buffered = false;
	std::shared_ptr<TranslationUnit> current_unit;

	void report_modified();
};

