#include "DefScript.h"
#include "DynamicEvent.h"

struct DefScript_DynamicEvent
{
	std::string name, cmd, parent;
	clock_t counter, interval;
    bool ran; // if a script has an execution time longer then the interval, and modifies an event, need to check if the event was already triggered, to prevent endless loop
};

DefScript_DynamicEventMgr::DefScript_DynamicEventMgr(DefScriptPackage *pack)
{
	_pack = pack;
	_lastclock = clock();
    _events_changed = false;
}

DefScript_DynamicEventMgr::~DefScript_DynamicEventMgr()
{
    _storage.Clear();
}

void DefScript_DynamicEventMgr::Add(std::string name, std::string script, clock_t interval, const char *parent, bool force)
{
    _DEFSC_DEBUG( printf("DEFSCRIPT: Add Event %s, interval=%u, parent=%s\n",name.c_str(),interval,parent?parent:""); printf("DEFSCRIPT: EventRun='%s'\n",script.c_str()); )
    if(name.empty() || script.empty() || interval==0)
        return;
    if(_storage.Exists(name) && !force)
        return;

    DefScript_DynamicEvent *e = _storage.Get(name);
    e->name = name;
    e->cmd = script;
    e->interval = interval;
    e->parent = parent?parent:"";
    e->counter = 0;
    e->ran = false;
    _events_changed = true;
}

void DefScript_DynamicEventMgr::Remove(std::string name)
{
    _storage.Delete(name);
    _events_changed = true;
}

void DefScript_DynamicEventMgr::Update(void)
{
    bool decr_timer = false;
    clock_t diff = clock() - _lastclock;
    _lastclock = clock();
    DefScript *sc;
    DefScript_DynamicEvent *e;
    _events_changed = false;

    for(std::map<std::string,DefScript_DynamicEvent*>::iterator i = _storage.GetMap().begin(); i != _storage.GetMap().end(); i++ )
    {
        i->second->ran = false;
    }

    for(std::map<std::string,DefScript_DynamicEvent*>::iterator i = _storage.GetMap().begin(); i != _storage.GetMap().end(); )
    {
        e = i->second;
        sc = NULL;
		try
		{
			e->counter += diff;
			if(e->counter >= e->interval && !e->ran)
			{
				decr_timer = true;

				if(!e->parent.empty())
					sc = _pack->GetScript(e->parent);

				if(sc)
					_pack->RunSingleLineFromScript(e->cmd,sc);
				else
					_pack->RunSingleLine(e->cmd);
			}
		}
		catch (...)
		{
			printf("Error in DefScript_DynamicEventMgr::Update()\n");
            return;
		}
        if(decr_timer)
        {
            e->counter %= e->interval;
            e->ran = true;
            decr_timer = false;
        }
        if(_events_changed)
        {
            _events_changed = false;
            i = _storage.GetMap().begin();
        }
        else
        {
            i++;
        }
    }
}
