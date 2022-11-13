#pragma once


#include <pompeius/TranslationUnit.h>


class TUObserver{
public:
	virtual ~TUObserver() = default;

	virtual void notify_new_tu(std::shared_ptr<TranslationUnit> unit) = 0;
	virtual void notify_deleted_tu(std::shared_ptr<TranslationUnit> unit) = 0;
	virtual void notify_updated_tu(std::shared_ptr<TranslationUnit> unit) = 0;
};

