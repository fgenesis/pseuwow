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
    for(ItemProtoList::iterator i = _iproto.begin(); i!=_iproto.end(); i++)
    {
        delete *i;
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

void ObjMgr::Add(ItemProto *proto)
{
    _iproto.push_back(proto);
}

ItemProto *ObjMgr::GetItemProto(uint32 entry)
{
    for(ItemProtoList::iterator i = _iproto.begin(); i!=_iproto.end(); i++)
        if((*i)->Id == entry)
            return *i;
    return NULL;
}

ItemProto *ObjMgr::GetItemProtoByPos(uint32 pos)
{
    return _iproto[pos];
}

void ObjMgr::AddNonexistentItem(uint32 id)
{
    _noitem.insert(id);
}

bool ObjMgr::ItemNonExistent(uint32 id)
{
    return _noitem.find(id) != _noitem.end();
}

void ObjMgr::AddRequestedPlayerGUID(uint32 loguid)
{
    _reqpnames.insert(loguid);
}

bool ObjMgr::IsRequestedPlayerGUID(uint32 loguid)
{
    return _reqpnames.find(loguid) != _reqpnames.end();
}
