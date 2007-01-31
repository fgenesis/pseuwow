#include "common.h"
#include "WorldSession.h"

#include "Object.h"

Object::Object()
{
    _uint32values=NULL;
    _type=TYPE_OBJECT;
    _typeid=TYPEID_OBJECT;
    _valuescount=0;
}

Object::~Object()
{
    // TODO: unregister object from object mgr
    if(_uint32values)
        delete [] _uint32values;
}
   
WorldObject::WorldObject()
{
    _x = _y = _z = _o = 0;
    _m = 0;
}

void WorldObject::SetPosition(float x, float y, float z, float o, uint16 _map)
{
    _x = x;
    _y = y;
    _z = z;
    _o = o;
    _m = _map;
}