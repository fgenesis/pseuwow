#include "Bag.h"

Bag::Bag() : Item()
{
    _type |= TYPE_CONTAINER;
    _typeid = TYPEID_CONTAINER;
    _valuescount = CONTAINER_END;
    _slot = 0;
}

void Bag::Create(uint64 guid)
{
    Item::Create(guid);
}
