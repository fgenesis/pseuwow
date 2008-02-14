#include "common.h"
#include "log.h"
#include "PseuWoW.h"
#include "ObjMgr.h"
#include "GUI/PseuGUI.h"

ObjMgr::ObjMgr()
{
    DEBUG(logdebug("DEBUG: ObjMgr created"));
}

ObjMgr::~ObjMgr()
{
    RemoveAll();
}

void ObjMgr::SetInstance(PseuInstance *i)
{
    _instance = i;
    DEBUG(logdebug("DEBUG: ObjMgr instance set to 0x%X",i));
}

void ObjMgr::RemoveAll(void)
{
    for(ItemProtoMap::iterator i = _iproto.begin(); i!=_iproto.end(); i++)
    {
        delete i->second;
    }
    for(CreatureTemplateMap::iterator i = _creature_templ.begin(); i!=_creature_templ.end(); i++)
    {
        delete i->second;
    }
    while(_obj.size())
    {
        Remove(_obj.begin()->first);
    }
}

void ObjMgr::Remove(uint64 guid)
{
    Object *o = GetObj(guid);
    if(o) 
    {
        PseuGUI *gui = _instance->GetGUI();
        if(gui)
            gui->NotifyObjectDeletion(guid); // we have a gui, which must delete linked DrawObject
        _obj.erase(guid); // now delete the obj from the mgr
        delete o; // and delete the obj itself
    }
}

// -- Object part --

void ObjMgr::Add(Object *o)
{
    _obj[o->GetGUID()] = o;

    PseuGUI *gui = _instance->GetGUI();
    if(gui)
        gui->NotifyObjectCreation(o);
}

Object *ObjMgr::GetObj(uint64 guid)
{
    if(!guid)
        return NULL;
    for(ObjectMap::iterator i = _obj.begin(); i!=_obj.end(); i++)
        if(i->second->GetGUID() == guid)
            return i->second;
    return NULL;
}

// iterate over all objects and assign a name to all matching the entry and typeid
uint32 ObjMgr::AssignNameToObj(uint32 entry, uint8 type, std::string name)
{
    uint32 changed = 0;
    for(ObjectMap::iterator it = _obj.begin(); it != _obj.end(); it++)
    {
        if(it->second->GetEntry() && (it->second->GetTypeId() == type))
        {
            it->second->SetName(name);
            changed++;
        }
    }
    return changed;
}

// -- Item part --

void ObjMgr::Add(ItemProto *proto)
{
    _iproto[proto->Id] = proto;
}

ItemProto *ObjMgr::GetItemProto(uint32 entry)
{
    ItemProtoMap::iterator it = _iproto.find(entry);
    if(it != _iproto.end())
        return it->second;
    return NULL;
}

void ObjMgr::AddNonexistentItem(uint32 id)
{
    _noitem.insert(id);
}

bool ObjMgr::ItemNonExistent(uint32 id)
{
    return _noitem.find(id) != _noitem.end();
}

// -- Creature part --

void ObjMgr::Add(CreatureTemplate *cr)
{
    _creature_templ[cr->entry] = cr;
}

CreatureTemplate *ObjMgr::GetCreatureTemplate(uint32 entry)
{
    CreatureTemplateMap::iterator it = _creature_templ.find(entry);
    if(it != _creature_templ.end())
        return it->second;
    return NULL;
}

void ObjMgr::AddNonexistentCreature(uint32 id)
{
    _nocreature.insert(id);
}

bool ObjMgr::CreatureNonExistent(uint32 id)
{
    return _nocreature.find(id) != _nocreature.end();
}

// -- misc part --

void ObjMgr::AddRequestedPlayerGUID(uint32 loguid)
{
    _reqpnames.insert(loguid);
}

bool ObjMgr::IsRequestedPlayerGUID(uint32 loguid)
{
    return _reqpnames.find(loguid) != _reqpnames.end();
}
