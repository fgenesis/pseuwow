#ifndef _OBJMGR_H
#define _OBJMGR_H

#include <list>
#include "Object.h"
#include "Player.h"
#include "Unit.h"
#include "Item.h"

typedef std::vector<ItemProto*> ItemProtoList;
//typedef std::list<Npc*> NpcList;
typedef std::list<Player*> PlayerList;
typedef std::list<Item*> ItemList;
//typedef std::list<Gameobject*> GOList;
//typedef std::list<DynamicObject*> DOList;
//typedef std::list<Corpse*> CorpseList;



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

    // Player functions
    void Add(Player*);
    Player *GetPlayer(uint64 guid);
    uint32 GetPlayersCount(void) { return _players.size(); }

    // Item functions
    void Add(Item*);
    Item *GetItem(uint64);
    uint32 GetItemsCount(void) { return _items.size(); }

    // generic functions
    void Remove(uint64); // remove all objects with that guid (should be only 1 object in total anyway)
    Object *GetObj(uint64 guid, uint8 type = 0);


private:
    void _RemovePlayer(uint64);
    void _RemoveItem(uint64);
    //void _RemoveGO(uint64);
    //void _RemoveDO(uint64);
    //void _RemoveNpc(uint64);
    //void _RemoveCorpse(uint64);








private:
    ItemProtoList _iproto;
    PlayerList _players;
    ItemList _items;

    std::vector<uint32> _noitem;

};

#endif