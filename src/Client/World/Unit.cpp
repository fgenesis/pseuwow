#include "common.h"
#include "Unit.h"

Unit::Unit() : WorldObject()
{
    _type |= TYPE_UNIT;
    _typeid = TYPEID_UNIT;
    _valuescount = UNIT_END;
}

void Unit::Create(uint64 guid)
{
    Object::Create(guid);
}

uint8 Unit::GetGender(void)
{
    uint32 temp = GetUInt32Value(UNIT_FIELD_BYTES_0);
    return ((uint8*)&temp)[2];
}

