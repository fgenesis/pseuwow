#ifndef _OBJMGR_H
#define _OBJMGR_H

#include <list>

class Object;
struct ItemProto;

typedef std::list<Object*> ObjList;
typedef std::vector<ItemProto*> ItemProtoList;


class ObjMgr
{
public:
    ~ObjMgr();
    void Add(Object*);
    void Add(ItemProto*);
    void AddNonexistentItem(uint32);
    bool ItemNonExistent(uint32);
    Object *GetObject(uint64);
    void RemoveObject(uint64);
    uint32 GetOjectCount(void) { return _obj.size(); }
    uint32 GetItemProtoCount(void) { return _iproto.size(); }
    ItemProto *GetItemProto(uint32);
    ItemProto *GetItemProtoByPos(uint32);

private:
    ObjList _obj;
    ItemProtoList _iproto;
    std::vector<uint32> _noitem;

};

#endif