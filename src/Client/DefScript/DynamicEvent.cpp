#include "DefScript.h"
#include "DynamicEvent.h"

struct DefScript_DynamicEvent
{
	std::string name, cmd, parent;
	clock_t counter, interval;
};

DefScript_DynamicEventMgr::DefScript_DynamicEventMgr(DefScriptPackage *pack)
{
	_pack = pack;
	_lastclock = clock();
}

DefScript_DynamicEventMgr::~DefScript_DynamicEventMgr()
{
	for(DefDynamicEventList::iterator i = _storage.begin(); i != _storage.end(); i++)
	{
		delete *i;
	}
}

void DefScript_DynamicEventMgr::Add(std::string name, std::string script, clock_t interval, const char *parent)
{
    _DEFSC_DEBUG( printf("DEFSCRIPT: Add Event %s, interval=%u, parent=%s\n",name.c_str(),interval,parent?parent:""); printf("DEFSCRIPT: EventRun='%s'\n",script.c_str()); )
    if(name.empty() || script.empty() || interval==0)
        return;
    for(DefDynamicEventList::iterator i = _storage.begin(); i != _storage.end(); i++)
        if((*i)->name == name)
            return; // event with that name is already registered
    DefScript_DynamicEvent *e = new DefScript_DynamicEvent;
    e->name = name;
    e->cmd = script;
    e->interval = interval;
    e->parent = parent?parent:"";
    e->counter = 0;
    _storage.push_back(e);
}

void DefScript_DynamicEventMgr::Remove(std::string name)
{
    if(name.empty())
        return;
    for(DefDynamicEventList::iterator i = _storage.begin(); i != _storage.end(); i++)
    {
        delete *i;
        _storage.erase(i);
        break;
    }
    return;
}

void DefScript_DynamicEventMgr::Update(void)
{
    clock_t diff = clock() - _lastclock;
    _lastclock = clock();
    DefScript *sc;
    for(DefDynamicEventList::iterator i = _storage.begin(); i != _storage.end(); i++)
    {
        sc = NULL;
        (*i)->counter += diff;
        if((*i)->counter >= (*i)->interval)
        {
            (*i)->counter %= (*i)->interval;

            if(!(*i)->parent.empty())
                sc = _pack->GetScript((*i)->parent);

            if(sc)
                _pack->RunSingleLineFromScript((*i)->cmd,sc);
            else
                _pack->RunSingleLine((*i)->cmd);
        }
    }
}
	