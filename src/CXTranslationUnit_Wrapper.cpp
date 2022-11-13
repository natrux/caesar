#include <brutus/CXTranslationUnit_Wrapper.h>
#include <brutus/error.h>

#include <memory>


CXTranslationUnit_Wrapper::CXTranslationUnit_Wrapper(CXTranslationUnit &&unit):
	translation_unit(unit)
{}


CXTranslationUnit_Wrapper::CXTranslationUnit_Wrapper(CXTranslationUnit_Wrapper &&other){
	*this = std::move(other);
}


CXTranslationUnit_Wrapper &CXTranslationUnit_Wrapper::operator=(CXTranslationUnit_Wrapper &&other){
	if(this == &other){
		return *this;
	}
	if(translation_unit){
		clang_disposeTranslationUnit(translation_unit);
	}
	translation_unit = other.translation_unit;
	other.translation_unit = NULL;
	return *this;
}


CXTranslationUnit_Wrapper::operator CXTranslationUnit() const{
	return translation_unit;
}


CXTranslationUnit_Wrapper::operator CXTranslationUnit *(){
	return &translation_unit;
}


CXTranslationUnit_Wrapper::~CXTranslationUnit_Wrapper(){
	if(translation_unit){
		clang_disposeTranslationUnit(translation_unit);
	}
}



