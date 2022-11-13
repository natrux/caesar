#pragma once

#include <clang-c/Index.h>


class Index{
public:
	Index(bool exclude_declarations_from_pch, bool display_diagnostics);
	Index(const Index &other) = delete;
	Index &operator=(const Index &other) = delete;
	~Index();

	operator CXIndex() const;

private:
	CXIndex index;
};



