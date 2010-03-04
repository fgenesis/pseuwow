#ifndef _UPDATEDATA_H
#define _UPDATEDATA_H

enum OBJECT_UPDATE_TYPE
{
	UPDATETYPE_VALUES               = 0,
	UPDATETYPE_MOVEMENT             = 1,
	UPDATETYPE_CREATE_OBJECT        = 2,
	UPDATETYPE_CREATE_OBJECT2       = 3,
	UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
	UPDATETYPE_NEAR_OBJECTS         = 5
};

enum OBJECT_UPDATE_FLAGS
{
    UPDATEFLAG_NONE         = 0x0000,
    UPDATEFLAG_SELF         = 0x0001,
    UPDATEFLAG_TRANSPORT    = 0x0002,
    UPDATEFLAG_HAS_TARGET   = 0x0004,
    UPDATEFLAG_LOWGUID      = 0x0008,
    UPDATEFLAG_HIGHGUID     = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040,
    UPDATEFLAG_VEHICLE      = 0x0080,
    UPDATEFLAG_POSITION     = 0x0100,
    UPDATEFLAG_ROTATION     = 0x0200
};

enum MovementFlags
{
    MOVEMENTFLAG_NONE               = 0x00000000,
    MOVEMENTFLAG_FORWARD            = 0x00000001,
    MOVEMENTFLAG_BACKWARD           = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT        = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT       = 0x00000008,
    MOVEMENTFLAG_TURN_LEFT          = 0x00000010,
    MOVEMENTFLAG_TURN_RIGHT         = 0x00000020,
    MOVEMENTFLAG_PITCH_UP           = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN         = 0x00000080,
    MOVEMENTFLAG_WALK_MODE          = 0x00000100,               // Walking
    MOVEMENTFLAG_ONTRANSPORT        = 0x00000200,
    MOVEMENTFLAG_LEVITATING         = 0x00000400,
    MOVEMENTFLAG_ROOT               = 0x00000800,
    MOVEMENTFLAG_FALLING            = 0x00001000,
    MOVEMENTFLAG_FALLINGFAR         = 0x00002000,
    MOVEMENTFLAG_PENDINGSTOP        = 0x00004000,
    MOVEMENTFLAG_PENDINGSTRAFESTOP  = 0x00008000,
    MOVEMENTFLAG_PENDINGFORWARD     = 0x00010000,
    MOVEMENTFLAG_PENDINGBACKWARD    = 0x00020000,
    MOVEMENTFLAG_PENDINGSTRAFELEFT  = 0x00040000,
    MOVEMENTFLAG_PENDINGSTRAFERIGHT = 0x00080000,
    MOVEMENTFLAG_PENDINGROOT        = 0x00100000,
    MOVEMENTFLAG_SWIMMING           = 0x00200000,               // appears with fly flag also
    MOVEMENTFLAG_ASCENDING          = 0x00400000,               // swim up also
    MOVEMENTFLAG_DESCENDING         = 0x00800000,               // swim down also
    MOVEMENTFLAG_CAN_FLY            = 0x01000000,               // can fly in 3.3?
    MOVEMENTFLAG_FLYING             = 0x02000000,               // Actual flying mode
    MOVEMENTFLAG_SPLINE_ELEVATION   = 0x04000000,               // used for flight paths
    MOVEMENTFLAG_SPLINE_ENABLED     = 0x08000000,               // used for flight paths
    MOVEMENTFLAG_WATERWALKING       = 0x10000000,               // prevent unit from falling through water
    MOVEMENTFLAG_SAFE_FALL          = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEMENTFLAG_HOVER              = 0x40000000
};

struct MovementInfo
{
    // common
    uint32  flags;
    uint16  unkFlags;
    uint32  time;
    float   x, y, z, o;
    // transport
    uint64  t_guid;
    float   t_x, t_y, t_z, t_o;
    uint32  t_time;
    uint8    t_seat;
    // swimming and unk
    float   s_angle;
    // last fall time
    uint32  fallTime;
    // jumping
    float   j_unk, j_sinAngle, j_cosAngle, j_xyspeed;
    // spline
    float   u_unk1;

    MovementInfo()
    {
        flags = time = t_time = fallTime = unkFlags = 0;
        t_seat = 0;
        x = y = z = o = t_x = t_y = t_z = t_o = s_angle = j_unk = j_sinAngle = j_cosAngle = j_xyspeed = u_unk1 = 0.0f;
        t_guid = 0;
    }

    void SetMovementFlags(uint32 _flags)
    {
        flags = _flags;
    }
};

bool IsFloatField(uint8, uint32);

#endif
