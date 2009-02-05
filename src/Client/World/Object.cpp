#include "common.h"
#include "WorldSession.h"

#include "Object.h"

Object::Object()
{
    _depleted = false;
    _uint32values=NULL;
    _type=TYPE_OBJECT;
    _typeid=TYPEID_OBJECT;
    _valuescount=OBJECT_END; // base class. this value will be set by derived classes
}

Object::~Object()
{
    ASSERT(_valuescount > 0);
    DEBUG(logdebug("~Object() GUID="I64FMT,GetGUID()));
    if(_uint32values)
        delete [] _uint32values;
}

void Object::_InitValues()
{
    _uint32values = new uint32[ _valuescount ];
    memset(_uint32values, 0, _valuescount*sizeof(uint32));
}

void Object::Create( uint64 guid )
{
    ASSERT(_valuescount > 0);
    if(!_uint32values)
        _InitValues();

    SetUInt64Value( OBJECT_FIELD_GUID, guid );
    SetUInt32Value( OBJECT_FIELD_TYPE, _type );
}

   
WorldObject::WorldObject()
{
    _depleted = false;
    _m = 0;
}

void WorldObject::SetPosition(float x, float y, float z, float o)
{
    _wpos.x = x;
    _wpos.y = y;
    _wpos.z = z;
    _wpos.o = o;
}

void WorldObject::SetPosition(float x, float y, float z, float o, uint16 _map)
{
    SetPosition(x,y,z,o);
    _m = _map;
}

float WorldObject::GetDistance(WorldObject* obj)
{
    float dx = GetX() - obj->GetX();
    float dy = GetY() - obj->GetY();
    float dz = GetZ() - obj->GetZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(float x, float y)
{
    float dx = GetX() - x;
    float dy = GetY() - y;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance(float x, float y, float z)
{
    float dx = GetX() - x;
    float dy = GetY() - y;
    float dz = GetZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(WorldObject* obj)
{
    float dx = GetX() - obj->GetX();
    float dy = GetY() - obj->GetY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(WorldObject* obj)
{
    float dz = fabs(GetZ() - obj->GetZ());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return ( dist > 0 ? dist : 0);
}



void WorldSession::_HandleDestroyObjectOpcode(WorldPacket& recvPacket)
{
    uint64 guid;
    uint8 dummy;

    recvPacket >> guid >> dummy;
    logdebug("Destroy Object "I64FMT,guid);

    // call script just before object removal
    if(GetInstance()->GetScripts()->ScriptExists("_onobjectdelete"))
    {
        Object *o = objmgr.GetObj(guid);
        CmdSet Set;
        Set.defaultarg = toString(guid);
        Set.arg[0] = o ? toString(o->GetTypeId()) : "";
        Set.arg[1] = "false"; // out of range = false
        GetInstance()->GetScripts()->RunScript("_onobjectdelete", &Set);
    }

    objmgr.Remove(guid, false);
}


