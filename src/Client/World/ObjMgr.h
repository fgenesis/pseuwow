#ifndef _OBJMGR_H
#define _OBJMGR_H

#include "common.h"
#include <list>
#include "Object.h"
#include "Player.h"
#include "Unit.h"
#include "Item.h"

typedef std::vector<ItemProto*> ItemProtoList;
typedef std::list<Object*> ObjectList;

class ObjMgr
{
public:
    ~ObjMgr();

    // Item Prototype functions
    uint32 GetItemProtoCount(void) { return _iproto.size(); }
    ItemProto *GetItemProto(uint32);
    ItemProto *GetItemProtoByPos(uint32);
    void Add(ItemProto*);

    // nonexistent items handler
    void AddNonexistentItem(uint32);
    bool ItemNonExistent(uint32);

    // Object functions
    void Add(Object*);
    void Remove(uint64); // remove all objects with that guid (should be only 1 object in total anyway)
    Object *GetObj(uint64 guid);

private:
    ItemProtoList _iproto;
    ObjectList _obj;
    std::vector<uint32> _noitem;

};

#endif