#include <brutus/cursor_t.h>
#include <brutus/string.h>

#include <stdexcept>


/*
Cursors that have a USR:
	CXXConstructor
	ParmDecl
	CXXMethod
	FieldDecl
	ClassDecl
	FunctionDecl
	VarDecl
	...?

Cursors that don't have a USR:
	inclusion directive
	IntegerLiteral
	UnexposedExpr
	C++ base class specifier
	TypeRef
	CompoundStmt
	DeclStmt
	BinaryOperator
	FloatingLiteral
	CallExpr
	StringLiteral
	MemberRefExpr
	DeclRefExpr
	CharacterLiteral
	CXXFunctionalCastExpr
	ReturnStmt
	...?

Cursors that have a useless USR ("c:"):
	CXXAccessSpecifier
	...?
*/


std::string cursor_t::kind_spelling() const{
	return convert(clang_getCursorKindSpelling(kind));
}


std::string cursor_t::get_usr_ref() const{
	if(!usr.empty()){
		return usr;
	}
	if(reference.exists){
		return reference.get()->get_usr_ref();
	}
	return "";
}


std::shared_ptr<const cursor_t> cursor_t::has_cursor_t::get() const{
	if(!exists){
		throw std::logic_error("Cursor does not exist");
	}
	if(!cached){
		cached = std::make_shared<cursor_t>(convert(ptr));
	}
	return cached;
}


cursor_t convert(const CXCursor &cursor){
	if(clang_Cursor_isNull(cursor)){
		throw std::runtime_error("null cursor");
	}
	if(cursor.kind == CXCursor_NoDeclFound){
		throw std::runtime_error("invalid cursor");
	}

	cursor_t result;
	result.kind = cursor.kind;
	result.is_declaration = clang_isDeclaration(cursor.kind);
	result.is_definition = clang_isCursorDefinition(cursor);
	result.hash = clang_hashCursor(cursor);
	result.usr = convert(clang_getCursorUSR(cursor));
	//result.location = convert(clang_getCursorLocation(cursor));  // bug?
	result.extent = convert(clang_getCursorExtent(cursor));
	result.display_name = convert(clang_getCursorDisplayName(cursor));
	result.location = result.extent.begin;  // see above
	result.spelling = convert(clang_getCursorSpelling(cursor));
	result.spelling_range = convert(clang_Cursor_getSpellingNameRange(cursor, 0, 0));
	result.type_spelling = convert(clang_getTypeSpelling(clang_getCursorType(cursor)));
	result.reference.ptr = clang_getCursorReferenced(cursor);
	result.reference.exists = !clang_Cursor_isNull(result.reference.ptr) && result.reference.ptr.kind != CXCursor_NoDeclFound && !clang_equalCursors(cursor, result.reference.ptr);
	result.semantic_parent.ptr = clang_getCursorSemanticParent(cursor);
	result.semantic_parent.exists = !clang_Cursor_isNull(result.semantic_parent.ptr) && result.semantic_parent.ptr.kind != CXCursor_NoDeclFound && !clang_equalCursors(cursor, result.semantic_parent.ptr);
	result.lexical_parent.ptr = clang_getCursorSemanticParent(cursor);
	result.lexical_parent.exists = !clang_Cursor_isNull(result.lexical_parent.ptr) && result.lexical_parent.ptr.kind != CXCursor_NoDeclFound && !clang_equalCursors(cursor, result.lexical_parent.ptr);
	result.canonical_declaration.ptr = clang_getCanonicalCursor(cursor);
	result.canonical_declaration.exists = !clang_Cursor_isNull(result.canonical_declaration.ptr) && result.canonical_declaration.ptr.kind != CXCursor_NoDeclFound;
	result.definition.ptr = clang_getCursorDefinition(cursor);
	result.definition.exists = !clang_Cursor_isNull(result.definition.ptr) && result.definition.ptr.kind != CXCursor_NoDeclFound;

	return result;
}

