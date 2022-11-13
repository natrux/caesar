#pragma once

#include <memory>


class TranslationUnit;


class TUParser{
public:
	virtual ~TUParser() = default;
	virtual void parse_this(TranslationUnit *unit) = 0;

protected:
	void parse_translation_unit(std::shared_ptr<TranslationUnit> unit);
	void parse_translation_unit(TranslationUnit *unit);
};

