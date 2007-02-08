#ifndef _DEF_DYNAMICEVENT_H
#define _DEF_DYNAMICEVENT_H

#include <time.h>
#include <list>
#include <string>

struct DefScript_DynamicEvent;
class DefScript;
class DefScriptPackage;
typedef std::list<DefScript_DynamicEvent*> DefDynamicEventList;

class DefScript_DynamicEventMgr
{
public:
    DefScript_DynamicEventMgr(DefScriptPackage *pack);
    ~DefScript_DynamicEventMgr();
    void Add(std::string name, std::string script, clock_t interval, const char *parent);
	void Remove(std::string name);
	void Update(void);
	
private:
	DefDynamicEventList _storage;
	clock_t _lastclock;
    DefScriptPackage *_pack;
};

#endif
