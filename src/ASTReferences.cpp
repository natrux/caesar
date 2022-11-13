#include <pompeius/ASTReferences.h>


void ASTReferences::add(cursor_t &&cursor){
	if(cursor.kind != CXCursor_CallExpr && cursor.kind != CXCursor_UnexposedExpr){
		list_t *target = &uses;
		if(cursor.is_declaration){
			target = &declarations;
			if(cursor.is_definition){
				target = &definitions;
			}
		}
		target->push_back(std::move(cursor));
	}
}


ASTReferences::list_t ASTReferences::get_declarations() const{
	return declarations;
}


ASTReferences::list_t ASTReferences::get_definitions() const{
	return definitions;
}


ASTReferences::list_t ASTReferences::get_uses() const{
	return uses;
}


ASTReferences::list_t ASTReferences::get_all() const{
	list_t result;
	{
		auto plus = get_declarations();
		result.insert(result.end(), plus.begin(), plus.end());
	}
	{
		auto plus = get_definitions();
		result.insert(result.end(), plus.begin(), plus.end());
	}
	{
		auto plus = get_uses();
		result.insert(result.end(), plus.begin(), plus.end());
	}

	return result;
}


