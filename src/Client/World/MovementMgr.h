#ifndef MOVEMENTMGR_H
#define MOVEMENTMGR_H

#include "common.h"
#include "UpdateData.h"

#define MOVE_HEARTBEAT_DELAY 500
#define MOVE_TURN_UPDATE_DIFF 0.15f // not sure about original/real value, but this seems good

// --
// -- MovementFlags and MovementInfo can be found in UpdateData.h
// --

enum MovementFlagsEx
{
    // custom flags
    MOVEMENTFLAG_ANY_MOVE = (MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT
    | MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT),
    MOVEMENTFLAG_ANY_MOVE_NOT_TURNING = (MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT)
};

enum MoveModes
{
    MOVEMODE_AUTO, // CPU controlling movement, MyCharacter must be updated by MovementMgr
    MOVEMODE_MANUAL, // user controlling movement, MyCharacter must be updated by the GUI
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
    inline uint8 GetMoveMode(void) { return _movemode; }
    void Update(bool);
    void MoveStartForward(void);
    void MoveStartBackward(void);
    void MoveStop(void);
    void MoveStartStrafeLeft(void);
    void MoveStartStrafeRight(void);
    void MoveStopStrafe(void);
    void MoveStartTurnLeft(void);
    void MoveStartTurnRight(void);
    void MoveStopTurn(void);
    void MoveFallLand(void);
    void MoveSetFacing(void);
    void MoveJump(void);
    //bool IsJumping(void);
    inline bool GetMoveFlags(void) { return _moveFlags; }
    inline bool HasMoveFlag(uint32 flag) { return _moveFlags & flag; }
    bool IsMoved(void) { bool m = _moved; _moved = false; return m; } // true if the character moved since last call
    bool IsMoving(void); // any move?
    bool IsTurning(void); // spinning around?
    bool IsWalking(void); // walking straight forward/backward?
    bool IsStrafing(void); // strafing left/right?



private:
    void _BuildPacket(uint16);
    PseuInstance *_instance;
    MyCharacter *_mychar;
    uint32 _moveFlags; // server relevant flags (move forward/backward/swim/fly/jump/etc)
    uint32 _updatetime; // timeMS of last update cycle
    uint32 _optime; // timeMS when last opcode was sent
    uint8 _movemode; // automatic or manual
    float _movespeed; // current xy movement speed
    float _jumptime;
    UnitMoveType _movetype; // index used for speed selection
    bool _moved;


};

#endif
