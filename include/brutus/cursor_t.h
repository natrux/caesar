#pragma once

#include <brutus/source_location_t.h>
#include <brutus/source_range_t.h>

#include <clang-c/Index.h>

#include <memory>


struct cursor_t{
	struct has_cursor_t{
		bool exists = false;
		CXCursor ptr;
		mutable std::shared_ptr<cursor_t> cached;

		std::shared_ptr<const cursor_t> get() const;
	};

	CXCursorKind kind;
	bool is_declaration;
	bool is_definition;
	unsigned int hash;
	std::string usr;
	source_location_t location;
	source_range_t extent;
	std::string display_name;
	std::string spelling;
	source_range_t spelling_range;
	std::string type_spelling;
	has_cursor_t reference;
	has_cursor_t semantic_parent;
	has_cursor_t lexical_parent;
	has_cursor_t canonical_declaration;
	has_cursor_t definition;

	std::string kind_spelling() const;
	std::string get_usr_ref() const;
};

cursor_t convert(const CXCursor &cursor);


