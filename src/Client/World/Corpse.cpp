#include "Corpse.h"

Corpse::Corpse()
{
    _uint32values=NULL;
    _type=TYPE_CORPSE;
    _typeid=TYPEID_CORPSE;
    _valuescount=CORPSE_END;
}

void Corpse::Create(uint64 guid)
{
    Object::Create(guid);
}