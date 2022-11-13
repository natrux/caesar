#pragma once


class TUParent{
public:
	virtual void state_changed(TranslationUnit *unit, bool was_init, bool was_ready, bool is_init, bool is_ready) = 0;
};

