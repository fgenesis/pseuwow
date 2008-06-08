#ifndef _OBJECT_H
#define _OBJECT_H

#include "UpdateFields.h"
#include "ObjectDefines.h"
#include "common.h"
#include "HelperDefs.h"
#include "World.h"

enum TYPE
{
    TYPE_OBJECT         = 1,
    TYPE_ITEM           = 2,
    TYPE_CONTAINER      = 6, // a container is ALWAYS an item!
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
    virtual ~Object();
    inline const uint64 GetGUID() const { return GetUInt64Value(0); }
    inline const uint32 GetGUIDLow() const { return GetUInt32Value(0); }
    inline const uint32 GetGUIDHigh() const { return GetUInt32Value(1); }
    inline uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
    inline uint16 GetValuesCount(void) { return _valuescount; }

    inline const uint8 GetTypeId() { return _typeid; }
    inline const uint8 GetTypeMask() { return _type; }
    inline bool IsType(uint8 mask) { return (mask & _type) ? true : false; }
    inline bool IsPlayer(void) { return _typeid == TYPEID_PLAYER; }           // specific
    inline bool IsUnit(void) { return _type & TYPE_UNIT; }                    // generic (unit = creature or player)
    inline bool IsCreature(void) { return IsUnit() && !IsPlayer(); }          // specific
    inline bool IsItem(void) { return _type & TYPE_ITEM; }                    // generic (item or container)
    inline bool IsContainer(void) { return _typeid == TYPEID_CONTAINER; }     // specific
    inline bool IsCorpse(void) { return _typeid == TYPEID_CORPSE; }           // specific
    inline bool IsDynObject(void) { return _typeid == TYPEID_DYNAMICOBJECT; } // specific
    inline bool IsGameObject(void) { return _typeid == TYPEID_GAMEOBJECT; }   // specific
    inline bool IsWorldObject(void) { return _type & (TYPE_PLAYER | TYPE_UNIT | TYPE_CORPSE | TYPE_DYNAMICOBJECT | TYPE_GAMEOBJECT); }
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
    inline void SetUInt32Value( uint16 index, uint32 value )
    {
        _uint32values[ index ] = value;
    }
    inline void SetUInt64Value( uint16 index, uint64 value )
    {
        *((uint64*)&(_uint32values[ index ])) = value;
    }

    inline void SetName(std::string name) { _name = name; }
    inline std::string GetName(void) { return _name; }

    inline float GetObjectSize() const
    {
        return ( _valuescount > UNIT_FIELD_BOUNDINGRADIUS ) ? _floatvalues[UNIT_FIELD_BOUNDINGRADIUS] : 0.39f;
    }

    void Create(uint64 guid);
    inline bool _IsDepleted(void) { return _depleted; }
    inline void _SetDepleted(void) { _depleted = true; }
    
protected:
    Object();
    void _InitValues(void);
        
    uint16 _valuescount;
    union
    {
        uint32 *_uint32values;
        float *_floatvalues;
    };
    uint8 _type;
    uint8 _typeid;
    std::string _name;
    bool _depleted : 1; // true if the object was deleted from the objmgr, but not from memory
};

class WorldObject : public Object
{
public:
    virtual ~WorldObject ( ) {}
    void SetPosition(float x, float y, float z, float o, uint16 _map);
    void SetPosition(float x, float y, float z, float o);
    inline void SetPosition(WorldPosition& wp) { _wpos = wp; }
    inline void SetPosition(WorldPosition& wp, uint16 mapid) { SetPosition(wp); _m = mapid; }
    inline WorldPosition GetPosition(void) {return _wpos; }
    inline WorldPosition *GetPositionPtr(void) {return &_wpos; }
    inline float GetX(void) { return _wpos.x; }
    inline float GetY(void) { return _wpos.y; }
    inline float GetZ(void) { return _wpos.z; }
    inline float GetO(void) { return _wpos.o; }
    float GetDistance(WorldObject *obj);
    float GetDistance2d(float x, float y);
    float GetDistance(float x, float y, float z);
    float GetDistance2d(WorldObject *obj);
    float GetDistanceZ(WorldObject *obj);

protected:
    WorldObject();
    WorldPosition _wpos; // coords, orientation
    uint16 _m; // map

};

inline uint32 GetValuesCountByTypeId(uint8 tid)
{
    switch(tid)
    {
    case TYPEID_OBJECT: return OBJECT_END;
    case TYPEID_UNIT: return UNIT_END;
    case TYPEID_PLAYER: return PLAYER_END;
    case TYPEID_ITEM: return ITEM_END;
    case TYPEID_CONTAINER: return CONTAINER_END;
    case TYPEID_GAMEOBJECT: return GAMEOBJECT_END;
    case TYPEID_DYNAMICOBJECT: return DYNAMICOBJECT_END;
    case TYPEID_CORPSE: return CORPSE_END;
    }
    return 0;
}

inline uint8 GetTypeIdByGuid(uint64 guid)
{
    switch(GUID_HIPART(guid))
    {
    case HIGHGUID_PLAYER:
        return TYPEID_PLAYER;
    case HIGHGUID_CORPSE:
        return TYPEID_CORPSE;
    case HIGHGUID_ITEM: // == HIGHGUID_CONTAINER
        return TYPEID_ITEM;
    case HIGHGUID_DYNAMICOBJECT:
        return TYPEID_DYNAMICOBJECT;
    case HIGHGUID_GAMEOBJECT:
    case HIGHGUID_TRANSPORT: // not sure
        return TYPEID_GAMEOBJECT;
    case HIGHGUID_UNIT:
        return TYPEID_UNIT;
    }
    return TYPEID_OBJECT;
}



#endif
