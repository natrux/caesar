#pragma once

#include <brutus/cursor_t.h>

#include <vector>


class ASTReferences{
	using list_t = std::vector<cursor_t>;
public:
	void add(cursor_t &&cursor);

	list_t get_declarations() const;
	list_t get_definitions() const;
	list_t get_uses() const;
	list_t get_all() const;

private:
	list_t declarations;
	list_t definitions;
	list_t uses;
};

