#ifndef _OBJECT_H
#define _OBJECT_H

#include "UpdateFields.h"

enum TYPE
{
    TYPE_OBJECT         = 1,
    TYPE_ITEM           = 2,
    TYPE_CONTAINER      = 6,
    TYPE_UNIT           = 8,
    TYPE_PLAYER         = 16,
    TYPE_GAMEOBJECT     = 32,
    TYPE_DYNAMICOBJECT  = 64,
    TYPE_CORPSE         = 128,
    TYPE_AIGROUP        = 256,
    TYPE_AREATRIGGER    = 512
};

enum TYPEID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7,
    TYPEID_AIGROUP       = 8,
    TYPEID_AREATRIGGER   = 9
};

class Object
{
public:
    Object();
    inline const uint64 GetGUID() const { return GetUInt64Value(0); }
    inline const uint32 GetGUIDLow() const { return GetUInt32Value(0); }
    inline const uint32 GetGUIDHigh() const { return GetUInt32Value(1); }
    inline uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
    inline uint16 GetValuesCount(void) { return _valuescount; }

    inline const uint8 GetTypeId() { return _typeid; }
    inline bool isType(uint8 mask) { return (mask & _type) ? true : false; }
    inline const uint32 GetUInt32Value( uint16 index ) const
    {
        return _uint32values[ index ];
    }

    inline const uint64 GetUInt64Value( uint16 index ) const
    {
        return *((uint64*)&(_uint32values[ index ]));
    }

    inline bool HasFlag( uint16 index, uint32 flag ) const
    {
        return (_uint32values[ index ] & flag) != 0;
    }
    inline const float GetFloatValue( uint16 index ) const
    {
        return _floatvalues[ index ];
    }
    inline void SetFloatValue( uint16 index, float value )
    {
        _floatvalues[ index ] = value;
    }
    
protected:
    ~Object();
        
    uint16 _valuescount;
    union
    {
        uint8 *_uint32values;
        float *_floatvalues;
    };
    uint8 _type;
    uint8 _typeid;
};

class WorldObject : public Object
{
public:
    WorldObject();
    void SetPosition(float x, float y, float z, float o, uint16 _map);
    inline float GetX(void) { return _x; }
    inline float GetY(void) { return _y; }
    inline float GetZ(void) { return _z; }
    inline float GetO(void) { return _o; }

protected:
    float _x,_y,_z,_o; // coords, orientation
    uint16 _m; // map

};



#endif
