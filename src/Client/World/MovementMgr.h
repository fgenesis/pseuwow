#ifndef MOVEMENTMGR_H
#define MOVEMENTMGR_H

#include "common.h"

#define MOVE_HEARTBEAT_DELAY 500

enum MovementFlags
{
    MOVEMENTFLAG_NONE           = 0x00000000,
    MOVEMENTFLAG_FORWARD        = 0x00000001,
    MOVEMENTFLAG_BACKWARD       = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT    = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT   = 0x00000008,
    MOVEMENTFLAG_LEFT           = 0x00000010,
    MOVEMENTFLAG_RIGHT          = 0x00000020,
    MOVEMENTFLAG_PITCH_UP       = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN     = 0x00000080,
    MOVEMENTFLAG_WALK           = 0x00000100,
    MOVEMENTFLAG_ONTRANSPORT    = 0x00000200,
    MOVEMENTFLAG_UNK1           = 0x00000400,
    MOVEMENTFLAG_FLY_UNK1       = 0x00000800,
    MOVEMENTFLAG_JUMPING        = 0x00001000,
    MOVEMENTFLAG_UNK4           = 0x00002000,
    MOVEMENTFLAG_FALLING        = 0x00004000,
    // 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000
    MOVEMENTFLAG_SWIMMING       = 0x00200000,               // appears with fly flag also
    MOVEMENTFLAG_FLY_UP         = 0x00400000,
    MOVEMENTFLAG_CAN_FLY        = 0x00800000,
    MOVEMENTFLAG_FLYING         = 0x01000000,
    MOVEMENTFLAG_UNK5           = 0x02000000,
    MOVEMENTFLAG_SPLINE         = 0x04000000,               // probably wrong name
    MOVEMENTFLAG_SPLINE2        = 0x08000000,
    MOVEMENTFLAG_WATERWALKING   = 0x10000000,
    MOVEMENTFLAG_SAFE_FALL      = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEMENTFLAG_UNK3           = 0x40000000,

    // custom flags
    MOVEMENTFLAG_ANY_MOVE = (MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT
    | MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT)
};

enum MoveModes
{
    MOVEMODE_AUTO, // CPU controlling movement, MyCharacter must be updated
    MOVEMODE_MANUAL, // user controlling movement, MyCharacter is updated by the GUI already
};

class PseuInstance;
class MyCharacter;


class MovementMgr
{
public:
    MovementMgr();
    ~MovementMgr();
    void SetInstance(PseuInstance*);
    inline void SetMoveMode(uint8 mode) { _movemode = mode; }
    void Update(bool);
    void MoveStartForward(void);
    void MoveStartBackward(void);
    void MoveStop(void);
    void MoveStartStrafeLeft(void);
    void MoveStartStrafeRight(void);
    void MoveStartTurnLeft(void);
    void MoveStartTurnRight(void);
    void MoveStopTurn(void);
    void MoveFallLand(void);
    void MoveSetFacing(float);

private:
    void _BuildPacket(uint16);
    PseuInstance *_instance;
    MyCharacter *_mychar;
    uint32 _moveFlags; // server relevant flags (move forward/backward/swim/fly/jump/etc)
    uint32 _updatetime; // timeMS of last update cycle
    uint32 _optime; // timeMS when last opcode was sent
    uint8 _movemode; // automatic or manual
    UnitMoveType _movetype; // index used for speed selection


};

#endif