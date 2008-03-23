#include "PseuWoW.h"
#include "WorldSession.h"
#include "World.h"
#include "MovementMgr.h"
#include "Player.h"

MovementMgr::MovementMgr()
{
    _moveFlags = 0;
    _instance = NULL;
    _optime = 0;
    _updatetime = 0;
}

MovementMgr::~MovementMgr()
{
}

void MovementMgr::SetInstance(PseuInstance *inst)
{
    _movemode = MOVEMODE_MANUAL;
    _instance = inst;
    _mychar = inst->GetWSession()->GetMyChar();
}

void MovementMgr::_BuildPacket(uint16 opcode)
{
    WorldPacket wp(opcode,4+1+4+12); // it can be larger, if we are juming, on transport or swimming
    wp << _moveFlags;
    wp << (uint8)0; // unk
    wp << getMSTime();
    wp << _mychar->GetPosition();
    // TODO: transport not yet handled/done
    if(_moveFlags & MOVEMENTFLAG_ONTRANSPORT)
    {
        wp << (uint64)0; // transport guid
        wp << WorldPosition(); // transport position
        wp << getMSTime(); // transport time (??)
    }
    // TODO: swimming not yet done
    if(_moveFlags & MOVEMENTFLAG_SWIMMING)
    {
        wp << (float)0; // angle; 1.55=looking up, -1.55=looking down, 0=looking forward
    }
    wp << (uint32)0; // last fall time (also used when jumping)
    // TODO: jumping not yet done
    // TODO: spline not yet done


    _optime = getMSTime();

}

void MovementMgr::Update(bool calcpos)
{
    uint32 curtime = getMSTime();
    uint32 timediff = curtime - _updatetime;
    _updatetime = curtime;

    if(_movemode == MOVEMODE_AUTO)
    {
        WorldPosition& pos = _mychar->GetPosition();
        float turnspeed = _mychar->GetSpeed(MOVE_TURN) / 1000.0f * timediff;
        float runspeed = _mychar->GetSpeed(MOVE_RUN) / 1000.0f * timediff;
        float movespeed = runspeed; // or use walkspeed, depending on setting. for now use only runspeed
        // TODO: calc other speeds as soon as implemented

        /*
        if(_moveFlags & MOVEMENTFLAG_FORWARD)
        {
            pos.x += movespeed * sin(pos.o);
            pos.y += movespeed * cos(pos.o);
        }
        // ...
        if(_moveFlags & MOVEMENTFLAG_LEFT)
        {
            pos.o -= turnspeed;
        }
        if(_moveFlags & MOVEMENTFLAG_RIGHT)
        {
            pos.o += turnspeed;
        }
        if(pos.o < 0)
            pos.o += 2 * M_PI;
        else if(pos.o > 2 * M_PI)
            pos.o -= 2 * M_PI;

        pos.z = _instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y);
        */
        // ^ It should look like this later on, but its not finished, and formulas are not tested.
        // see it as some future plans that need a lot of finetuning ;)
    }

    // if we are moving, and 500ms have passed, send an heartbeat packet
    if( (_moveFlags & MOVEMENTFLAG_ANY_MOVE) && _optime + MOVE_HEARTBEAT_DELAY < getMSTime())
    {
        _BuildPacket(MSG_MOVE_HEARTBEAT);
    }

    // TODO: apply gravity, handle falling, swimming, etc.
}

void MovementMgr::MoveStop(void)
{
    _moveFlags &= ~(MOVEMENTFLAG_ANY_MOVE);
    Update(false);
    _BuildPacket(MSG_MOVE_STOP);
}

void MovementMgr::MoveStartForward(void)
{
    _moveFlags |= MOVEMENTFLAG_FORWARD;
    _moveFlags &= ~MOVEMENTFLAG_BACKWARD;
    Update(false);
    _BuildPacket(MSG_MOVE_START_FORWARD);
}

void MovementMgr::MoveStartBackward(void)
{
    _moveFlags |= MOVEMENTFLAG_BACKWARD;
    _moveFlags &= ~MOVEMENTFLAG_FORWARD;
    Update(false);
    _BuildPacket(MSG_MOVE_START_BACKWARD);
}

void MovementMgr::MoveStartStrafeLeft(void)
{
    _moveFlags |= MOVEMENTFLAG_STRAFE_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_RIGHT;
    Update(false);
    _BuildPacket(MSG_MOVE_START_STRAFE_LEFT);
}

void MovementMgr::MoveStartStrafeRight(void)
{
    _moveFlags |= MOVEMENTFLAG_STRAFE_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_LEFT;
    Update(false);
    _BuildPacket(MSG_MOVE_START_STRAFE_RIGHT);
}

void MovementMgr::MoveStartTurnLeft(void)
{
    _moveFlags |= MOVEMENTFLAG_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_RIGHT;
    Update(false);
    _BuildPacket(MSG_MOVE_START_TURN_LEFT);
}

void MovementMgr::MoveStartTurnRight(void)
{
    _moveFlags |= MOVEMENTFLAG_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_LEFT;
    Update(false);
    _BuildPacket(MSG_MOVE_START_TURN_RIGHT);
}

void MovementMgr::MoveStopTurn(void)
{
    _moveFlags &= ~(MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT);
    Update(false);
    _BuildPacket(MSG_MOVE_STOP_TURN);
}

void MovementMgr::MoveSetFacing(float o)
{
    _mychar->SetPosition(_mychar->GetX(), _mychar->GetY(), _mychar->GetZ(), o);
    Update(true);
    _BuildPacket(MSG_MOVE_SET_FACING);
}

