#include "common.h"
#include "Item.h"
#include "ObjMgr.h"

ObjMgr::~ObjMgr()
{
    for(ObjList::iterator i = _obj.begin(); i!=_obj.end(); i++)
    {
        delete *i;
    }
    for(ItemProtoList::iterator i = _iproto.begin(); i!=_iproto.end(); i++)
    {
        delete *i;
    }
}


void ObjMgr::Add(Object *o)
{
    _obj.push_back(o);
}

Object *ObjMgr::GetObject(uint64 guid)
{
    for(ObjList::iterator i = _obj.begin(); i!=_obj.end(); i++)
        if((*i)->GetGUID() == guid)
            return (*i);
    return NULL;
}

void ObjMgr::RemoveObject(uint64 guid)
{
    for(ObjList::iterator i = _obj.begin(); i!=_obj.end(); i++)
        if((*i)->GetGUID() == guid)
        {
            delete *i;
            _obj.erase(i);
        }
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
    _noitem.push_back(id);
}

bool ObjMgr::ItemNonExistent(uint32 id)
{
    for(std::vector<uint32>::iterator i=_noitem.begin(); i != _noitem.end(); i++)
    {
        if(*i == id)
            return true;
    }
    return false;
}
    

    
