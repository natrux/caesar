#include <pompeius/TUParser.h>
#include <pompeius/TranslationUnit.h>


void TUParser::parse_translation_unit(std::shared_ptr<TranslationUnit> unit){
	unit->parse();
}


void TUParser::parse_translation_unit(TranslationUnit *unit){
	unit->parse();
}
