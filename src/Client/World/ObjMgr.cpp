#include "common.h"
#include "Item.h"
#include "ObjMgr.h"

ObjMgr::~ObjMgr()
{
    for(ItemProtoList::iterator i = _iproto.begin(); i!=_iproto.end(); i++)
    {
        delete *i;
    }
    // need to remove other objects also
}

void ObjMgr::Remove(uint64 guid)
{
    _RemovePlayer(guid);
    _RemoveItem(guid);
    //_RemoveGO(guid);
    //_RemoveDO(guid);
    //_RemoveNpc(guid);
    //_RemoveCorpse(guid);
}

void ObjMgr::_RemovePlayer(uint64 guid)
{
    for(PlayerList::iterator i = _players.begin(); i!=_players.end(); i++)
    if((*i)->GetGUID() == guid)
    {
        delete *i;
        _players.erase(i);
    }
}

void ObjMgr::Add(Player *o)
{
    _players.push_back(o);
}

Player *ObjMgr::GetPlayer(uint64 guid)
{
    for(PlayerList::iterator i = _players.begin(); i!=_players.end(); i++)
        if((*i)->GetGUID() == guid)
            return (*i);
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

void ObjMgr::_RemoveItem(uint64 guid)
{
    for(ItemList::iterator i = _items.begin(); i!=_items.end(); i++)
        if((*i)->GetGUID() == guid)
        {
            delete *i;
            _items.erase(i);
        }
}

void ObjMgr::Add(Item *o)
{
    _items.push_back(o);
}

Item *ObjMgr::GetItem(uint64 guid)
{
    for(ItemList::iterator i = _items.begin(); i!=_items.end(); i++)
        if((*i)->GetGUID() == guid)
            return (*i);
    return NULL;
}

Object *ObjMgr::GetObj(uint64 guid, uint8 type)
{
    switch (type)
    {
    case TYPEID_ITEM: return (Object*)GetItem(guid);
    case TYPEID_PLAYER: return (Object*)GetPlayer(guid);
    // TODO: need the other object types here
    }
    return NULL;
}


// TODO: add more object class functions here

    
