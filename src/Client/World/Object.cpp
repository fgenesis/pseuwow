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
    if(_uint32values)
        delete [] _uint32values;
}

void Object::_InitValues()
{
    _uint32values = new uint32[ _valuescount ];
    memset(_uint32values, 0, _valuescount*sizeof(uint32));
}

void Object::_Create( uint64 guid )
{
    if(!_uint32values)
        _InitValues();

    SetUInt32Value( OBJECT_FIELD_GUID, GUID_LOPART(guid) );
    SetUInt32Value( OBJECT_FIELD_GUID+1, GUID_HIPART(guid) );
    SetUInt32Value( OBJECT_FIELD_TYPE, _type );
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

void WorldSession::_HandleDestroyObjectOpcode(WorldPacket& recvPacket)
{
    uint64 guid;
    recvPacket >> guid;
    logdebug("Destroy Object "I64FMT,guid);
    objmgr.RemoveObject(guid);
}

