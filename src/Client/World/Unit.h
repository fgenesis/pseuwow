#ifndef _UNIT_H
#define _UNIT_H

#include "Object.h"

enum UnitMoveType
{
	MOVE_WALK       = 0,
	MOVE_RUN        = 1,
	MOVE_WALKBACK   = 2,
	MOVE_SWIM       = 3,
	MOVE_SWIMBACK   = 4,
	MOVE_TURN       = 5,
	MOVE_FLY        = 6,
	MOVE_FLYBACK    = 7,
	MOVE_PITCH_RATE = 8,
	MAX_MOVE_TYPE   = 9
};

enum UnitFlags
{
	UNIT_FLAG_NONE           = 0x00000000,
	UNIT_FLAG_DISABLE_MOVE   = 0x00000004,
	UNIT_FLAG_UNKNOWN1       = 0x00000008,                  // essential for all units..
	UNIT_FLAG_RENAME         = 0x00000010,                  // rename creature, not working in 2.0.8
	UNIT_FLAG_RESTING        = 0x00000020,
	UNIT_FLAG_UNKNOWN2       = 0x00000100,                  // 2.0.8
	UNIT_FLAG_UNKNOWN3       = 0x00000800,                  // in combat ?2.0.8
	UNIT_FLAG_PVP            = 0x00001000,
	UNIT_FLAG_MOUNT          = 0x00002000,
	UNIT_FLAG_UNKNOWN4       = 0x00004000,                  // 2.0.8
	UNIT_FLAG_PACIFIED       = 0x00020000,
	UNIT_FLAG_DISABLE_ROTATE = 0x00040000,                  // may be it's stunned flag?
	UNIT_FLAG_IN_COMBAT      = 0x00080000,
	UNIT_FLAG_DISARMED       = 0x00200000,                  // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
	UNIT_FLAG_CONFUSED       = 0x00400000,
	UNIT_FLAG_FLEEING        = 0x00800000,
	UNIT_FLAG_UNKNOWN5       = 0x01000000,					// used in spell Eyes of the Beast for pet...
	UNIT_FLAG_NOT_SELECTABLE = 0x02000000,
	UNIT_FLAG_SKINNABLE      = 0x04000000,
	UNIT_FLAG_UNKNOWN6       = 0x20000000,                  // used in Feing Death spell
	UNIT_FLAG_SHEATHE        = 0x40000000
};

#define MAX_KILL_CREDIT 2

struct CreatureTemplate
{
    uint32 entry;
    std::string name;
    std::string subname;
    std::string directions;
    uint32 flag1;
    uint32 type;
    uint32 family;
    uint32 rank;
    //uint32 unk1;
    uint32 SpellDataId;
    uint32 killCredit[MAX_KILL_CREDIT];
    uint32 displayid_A;
    uint32 displayid_H;
    uint32 displayid_AF;
    uint32 displayid_HF;
    //float unkf1;
    //float unkf2;
    uint8 RacialLeader;
    uint32 questItems[4];
    uint32 movementId;
};




class Unit : public WorldObject
{
public:
    Unit();
    void Create(uint64);
    uint8 GetGender(void);
    void SetSpeed(uint8 speednr, float speed) { _speed[speednr] = speed; }
    float GetSpeed(uint8 speednr) { return _speed[speednr]; }
    uint8 GetRace() const { return (uint8)(GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFF); };
    uint8 GetClass() const { return (uint8)((GetUInt32Value(UNIT_FIELD_BYTES_0) >> 8) & 0xFF); };
protected:
    float _speed[MAX_MOVE_TYPE];

};


#endif
