#pragma once


#include <brutus/cursor_t.h>

#include <clang-c/Index.h>

#include <string>


/*
 * Transparent memory managed wrapper for CXTranslationUnit.
 */
class CXTranslationUnit_Wrapper{
public:
	CXTranslationUnit_Wrapper() = default;
	CXTranslationUnit_Wrapper(CXTranslationUnit &&unit);

	CXTranslationUnit_Wrapper(const CXTranslationUnit_Wrapper &other) = delete;
	CXTranslationUnit_Wrapper(CXTranslationUnit_Wrapper &&other);

	CXTranslationUnit_Wrapper &operator=(const CXTranslationUnit_Wrapper &other) = delete;
	CXTranslationUnit_Wrapper &operator=(CXTranslationUnit_Wrapper &&other);

	operator CXTranslationUnit() const;
	operator CXTranslationUnit *();

	~CXTranslationUnit_Wrapper();

private:
	CXTranslationUnit translation_unit = NULL;
};


