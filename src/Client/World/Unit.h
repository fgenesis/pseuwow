#ifndef _UNIT_H
#define _UNIT_H

#include "Object.h"

enum UnitMoveType
{
    MOVE_WALK       =0,
    MOVE_RUN        =1,
    MOVE_WALKBACK   =2,
    MOVE_SWIM       =3,
    MOVE_SWIMBACK   =4,
    MOVE_TURN       =5,
    MAX_MOVE_TYPE   =6
};

enum UnitFlags
{
    UNIT_FLAG_NONE           = 0x00000000,
    UNIT_FLAG_DISABLE_MOVE   = 0x00000004,
    UNIT_FLAG_UNKNOWN1       = 0x00000008,                  // essential for all units..
    UNIT_FLAG_RENAME         = 0x00000010,                  // rename creature
    UNIT_FLAG_RESTING        = 0x00000020,
    UNIT_FLAG_PVP            = 0x00001000,
    UNIT_FLAG_MOUNT          = 0x00002000,
    UNIT_FLAG_DISABLE_ROTATE = 0x00040000,
    UNIT_FLAG_IN_COMBAT      = 0x00080000,
    UNIT_FLAG_SKINNABLE      = 0x04000000,
    UNIT_FLAG_SHEATHE        = 0x40000000
};



class Unit : public WorldObject
{
public:
    Unit();
    void Create(uint64);
    uint8 GetGender(void);
    void SetSpeed(uint8 speednr, float speed) { _speed[speednr] = speed; }
    float GetSpeed(uint8 speednr) { return _speed[speednr]; }
protected:
    float _speed[MAX_MOVE_TYPE];

};


#endif