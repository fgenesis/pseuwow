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
    for(GOTemplateMap::iterator i = _go_templ.begin(); i!=_go_templ.end(); i++)
    {
        delete i->second;
    }
    while(_obj.size())
    {
        Remove(_obj.begin()->first, true);
    }
    if(PseuGUI *gui = _instance->GetGUI())
    {
        // necessary that the pending-to-delete GUIDs just stored by deleting the objects above will be cleared
        // so that newly added DrawObjects with uncleared pending-to-delete GUIDs will not get deleted again immediately.
        gui->NotifyAllObjectsDeletion();
    }
}

void ObjMgr::Remove(uint64 guid, bool del)
{
    Object *o = GetObj(guid, true); // here get also depleted objs and delete if necessary
    if(o) 
    {
        o->_SetDepleted();
        if(!del)
            logdebug("ObjMgr: "I64FMT" '%s' -> depleted.",guid,o->GetName().c_str()); 
        PseuGUI *gui = _instance->GetGUI();
        if(gui)
            gui->NotifyObjectDeletion(guid); // we have a gui, which must delete linked DrawObject
        if(del)
        {
            _obj.erase(guid); // now delete the obj from the mgr
            delete o; // and delete the obj itself
        }
    }
    else
    {
        _obj.erase(guid); // we can safely erase an object that does not exist
                          // - if we reach this point there was a bug anyway
        logcustom(2,LRED,"ObjMgr::Remove("I64FMT") - not existing",guid); 
    }        
}

// -- Object part --

void ObjMgr::Add(Object *o)
{
    Object *ox = GetObj(o->GetGUID(),true); // if an object already exists in the mgr, store old ptr...
    if(o == ox)
        return; // if both pointers are the same, do nothing (already added and happy)
    _obj[o->GetGUID()] = o; // ...assign new one...
    if(ox) // and if != NULL, delete the old object (completely, from memory)
    {
        delete ox; // only delete pointer, everything else is already reserved for the just added new obj
    }

    if(PseuGUI *gui = _instance->GetGUI())
        gui->NotifyObjectCreation(o);
}

Object *ObjMgr::GetObj(uint64 guid, bool also_depleted)
{
    if(!guid)
        return NULL;
    for(ObjectMap::iterator i = _obj.begin(); i!=_obj.end(); i++)
        if(i->second->GetGUID() == guid)
        {
            if(i->second->_IsDepleted() && !also_depleted)
                return NULL;
            return i->second;
        }
    return NULL;
}

// iterate over all objects and assign a name to all matching the entry and typeid
uint32 ObjMgr::AssignNameToObj(uint32 entry, uint8 type, std::string name)
{
    uint32 changed = 0;
    for(ObjectMap::iterator it = _obj.begin(); it != _obj.end(); it++)
    {
        if(it->second->GetEntry() == entry && (it->second->GetTypeId() == type))
        {
            it->second->SetName(name);
            changed++;
        }
    }
    return changed;
}

void ObjMgr::ReNotifyGUI(void)
{
    PseuGUI *gui = _instance->GetGUI();
    if(!gui)
        return;
    for(ObjectMap::iterator it = _obj.begin(); it != _obj.end(); it++)
    {
        Object *o = it->second;
        if(o->_IsDepleted())
            continue;
        gui->NotifyObjectCreation(o);
    }
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

// -- Gameobject part --

void ObjMgr::Add(GameobjectTemplate *go)
{
    _go_templ[go->entry] = go;
}

GameobjectTemplate *ObjMgr::GetGOTemplate(uint32 entry)
{
    GOTemplateMap::iterator it = _go_templ.find(entry);
    if(it != _go_templ.end())
        return it->second;
    return NULL;
}

void ObjMgr::AddNonexistentGO(uint32 id)
{
    _nogameobj.insert(id);
}

bool ObjMgr::GONonExistent(uint32 id)
{
    return _nogameobj.find(id) != _nogameobj.end();
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
