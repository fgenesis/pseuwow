#ifndef _OBJMGR_H
#define _OBJMGR_H

#include "common.h"
#include <list>
#include "Object.h"
#include "Item.h"

typedef std::vector<ItemProto*> ItemProtoList;
typedef std::map<uint64,Object*> ObjectMap;

class PseuInstance;

class ObjMgr
{
public:
    ObjMgr();
    ~ObjMgr();
    void SetInstance(PseuInstance*);
    void RemoveAll(void); // TODO: this needs to be called on SMSG_LOGOUT_COMPLETE once implemented.

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
    inline uint32 GetObjectCount(void) { return _obj.size(); }

private:
    ItemProtoList _iproto;
    ObjectMap _obj;
    std::vector<uint32> _noitem;
    PseuInstance *_instance;

};

#endif