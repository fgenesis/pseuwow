#include "common.h"
#include "Unit.h"

Unit::Unit() : WorldObject()
{
    _type = TYPE_UNIT;
    _typeid = TYPEID_UNIT;
}

