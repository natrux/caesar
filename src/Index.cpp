#include <brutus/Index.h>

#include <utility>


Index::Index(bool exclude_declarations_from_pch, bool display_diagnostics){
	index = clang_createIndex(exclude_declarations_from_pch, display_diagnostics);
}


Index::~Index(){
	clang_disposeIndex(index);
}


Index::operator CXIndex() const{
	return index;
}



