#include "WorldSession.h"
#include "UpdateFields.h"

#include "Item.h"

Item::Item()
{
    _type |= TYPE_ITEM;
    _typeid = TYPEID_ITEM;

    _valuescount = ITEM_END;
    _slot = 0;
    //_bag = NULL; // not yet implemented
}