#ifndef _DEF_DYNAMICEVENT_H
#define _DEF_DYNAMICEVENT_H

#include <time.h>
#include <list>
#include <string>

#include "TypeStorage.h"

struct DefScript_DynamicEvent;
class DefScript;
class DefScriptPackage;
typedef TypeStorage<DefScript_DynamicEvent> DefDynamicEventStorage;

class DefScript_DynamicEventMgr
{
public:
    DefScript_DynamicEventMgr(DefScriptPackage *pack);
    ~DefScript_DynamicEventMgr();
    void Add(std::string name, std::string script, clock_t interval, const char *parent, bool force = false);
	void Remove(std::string name);
	void Update(void);
	
private:
	DefDynamicEventStorage _storage;
	clock_t _lastclock;
    DefScriptPackage *_pack;
    bool _events_changed; // this one helps to detect if a script called from an event has modified the event storage (risk of iterator invalidation!)
};

#endif
