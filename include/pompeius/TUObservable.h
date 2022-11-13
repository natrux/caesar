#pragma once


#include <pompeius/TUObserver.h>


class TUObservable{
public:
	virtual void register_observer(TUObserver *obs, bool send_digest) = 0;
	virtual void unregister_observer(TUObserver *obs) = 0;
};

