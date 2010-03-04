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
    _moved = false;
}

MovementMgr::~MovementMgr()
{
}

void MovementMgr::SetInstance(PseuInstance *inst)
{
    _movemode = MOVEMODE_MANUAL;
    _instance = inst;
    _mychar = inst->GetWSession()->GetMyChar();
    if(!_mychar)
    {
        logerror("MovementMgr: MyCharacter doesn't exist!");
        // it will likely crash somewhere after outputting this message, but in case it really appears,
        // we can be sure there is something absolutely not ok...
    }
}

void MovementMgr::_BuildPacket(uint16 opcode)
{
    WorldPacket *wp = new WorldPacket(opcode,4+2+4+16); // it can be larger, if we are jumping, on transport or swimming
    wp->appendPackGUID(_mychar->GetGUID());
    *wp << _moveFlags;
    *wp << (uint16)0; // flags2 , safe to set 0 for now (shlainn)
    *wp << getMSTime();
    *wp << _mychar->GetPosition();
    // TODO: transport not yet handled/done
    if(_moveFlags & MOVEMENTFLAG_ONTRANSPORT)
    {
        *wp << (uint64)0; // transport guid
        *wp << WorldPosition(); // transport position
        *wp << getMSTime(); // transport time (??)
    }
    // TODO: swimming not yet done
    if(_moveFlags & MOVEMENTFLAG_SWIMMING)
    {
        *wp << (float)0; // angle; 1.55=looking up, -1.55=looking down, 0=looking forward
    }
    *wp << (uint32)0; // last fall time (also used when jumping)
    if(_moveFlags & MOVEMENTFLAG_PENDINGSTOP)
    {
        *wp << (float)0; //unk value, or as mangos calls it: j_unk ^^
        *wp << sin(_mychar->GetO()+ (M_PI/2));
        *wp << cos(_mychar->GetO()+ (M_PI/2));
        *wp << _movespeed;
    }

    // TODO: spline not yet done

    DEBUG(logdebug("Move flags: 0x%X (packet: %u bytes)",_moveFlags,wp->size()));
    // send the packet, threadsafe
    _instance->GetWSession()->AddSendWorldPacket(wp);
    _moved = true;
    _optime = getMSTime();

}

void MovementMgr::Update(bool sendDirect)
{
    uint32 curtime = getMSTime();
    uint32 timediff = curtime - _updatetime;
    _updatetime = curtime;

    WorldPosition pos = _mychar->GetPosition();
    float turnspeed = _mychar->GetSpeed(MOVE_TURN) / 1000.0f * timediff;
    float runspeed = _mychar->GetSpeed(MOVE_RUN) / 1000.0f * timediff;
    _movespeed = runspeed; // or use walkspeed, depending on setting. for now use only runspeed
    // TODO: calc other speeds as soon as implemented
/*
    if(_movemode == MOVEMODE_MANUAL)
    {
        if(_moveFlags & MOVEMENTFLAG_JUMPING)
        {
            // approx. jumping formula
            _jumptime += timediff / 1000.0f;
            if (_jumptime < 0.4f)
                pos.z += 2.0f / 1000.0f * timediff;
            if (_jumptime >= 0.4f && _jumptime < 0.8f)
            {
                pos.z -= 2.0f / 1000.0f * timediff;
            }
            if (_jumptime >= 0.8f)
            {
                _jumptime = 0.0f;
                _moveFlags &= ~MOVEMENTFLAG_JUMPING;
            }
        }

        if(_movemode == MOVEMODE_AUTO)
        {
            if(_moveFlags & MOVEMENTFLAG_FORWARD)
            {

                WorldPosition oldpos = pos;
                pos.x += _movespeed * sin(pos.o + (M_PI/2));
                pos.y -= _movespeed * cos(pos.o + (M_PI/2));
                if (_instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y) > 5.0f + pos.z)
                {
                    pos = oldpos;
                }
            }
            if(_moveFlags & MOVEMENTFLAG_BACKWARD)
            {

                WorldPosition oldpos = pos;
                pos.x -= _movespeed * sin(pos.o + (M_PI/2));
                pos.y += _movespeed * cos(pos.o + (M_PI/2));
                if (_instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y) > 5.0f + pos.z)
                {
                    pos = oldpos;
                }
            }
        }
        // ...
        if(_moveFlags & MOVEMENTFLAG_LEFT)
        {
            pos.o += turnspeed;
        }
        if(_moveFlags & MOVEMENTFLAG_RIGHT)
        {
            pos.o -= turnspeed;
        }
        if(pos.o < 0)
            pos.o += float(2 * M_PI);
        else if(pos.o > 2 * M_PI)
            pos.o -= float(2 * M_PI);
        //pos.z = _instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y);
        
        if(_movemode == MOVEMODE_AUTO)
        {
            _mychar->SetPosition(pos);
        }
    }*/

    // if we are moving, and 500ms have passed, send an heartbeat packet. just in case 500ms have passed but the packet is sent by another function, do not send here
    if( !sendDirect && (_moveFlags & MOVEMENTFLAG_ANY_MOVE_NOT_TURNING) && _optime + MOVE_HEARTBEAT_DELAY < getMSTime())
    {
        _BuildPacket(MSG_MOVE_HEARTBEAT);

        // also need to tell the world map mgr that we moved; maybe maps need to be loaded
        // the main thread will take care of really loading the maps; here we just tell our updated position
        if(World *world = _instance->GetWSession()->GetWorld())
        {
            world->UpdatePos(pos.x, pos.y, world->GetMapId());
        }
    }
    // TODO: apply gravity, handle falling, swimming, etc.
}

// stops
void MovementMgr::MoveStop(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_WALK_MODE);
    Update(true);
    _BuildPacket(MSG_MOVE_STOP);
}

void MovementMgr::MoveStartForward(void)
{
    if(_moveFlags & MOVEMENTFLAG_FORWARD)
        return;
    _moveFlags |= MOVEMENTFLAG_FORWARD;
    _moveFlags &= ~MOVEMENTFLAG_BACKWARD;
    Update(true);
    _BuildPacket(MSG_MOVE_START_FORWARD);
}

void MovementMgr::MoveStartBackward(void)
{
    if(_moveFlags & MOVEMENTFLAG_BACKWARD)
        return;
    _moveFlags |= (MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_WALK_MODE); // backward walk is always slow; flag must be set, otherwise causing weird movement in other client
    _moveFlags &= ~MOVEMENTFLAG_FORWARD;
    Update(true);
    _BuildPacket(MSG_MOVE_START_BACKWARD);
}

void MovementMgr::MoveStartStrafeLeft(void)
{
    if(_moveFlags & MOVEMENTFLAG_STRAFE_LEFT)
        return;
    _moveFlags |= MOVEMENTFLAG_STRAFE_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_RIGHT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_LEFT);
}

void MovementMgr::MoveStartStrafeRight(void)
{
    if(_moveFlags & MOVEMENTFLAG_STRAFE_RIGHT)
        return;
    _moveFlags |= MOVEMENTFLAG_STRAFE_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_LEFT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_RIGHT);
}

void MovementMgr::MoveStopStrafe(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_STRAFE_RIGHT | MOVEMENTFLAG_STRAFE_LEFT)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_STRAFE_RIGHT | MOVEMENTFLAG_STRAFE_LEFT);
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_RIGHT);
}


void MovementMgr::MoveStartTurnLeft(void)
{
    if(_moveFlags & MOVEMENTFLAG_TURN_LEFT)
        return;
    _moveFlags |= MOVEMENTFLAG_TURN_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_TURN_RIGHT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_TURN_LEFT);
}

void MovementMgr::MoveStartTurnRight(void)
{
    if(_moveFlags & MOVEMENTFLAG_TURN_RIGHT)
        return;
    _moveFlags |= MOVEMENTFLAG_TURN_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_TURN_LEFT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_TURN_RIGHT);
}

void MovementMgr::MoveStopTurn(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT);
    Update(true);
    _BuildPacket(MSG_MOVE_STOP_TURN);
}

void MovementMgr::MoveSetFacing(void)
{
    Update(true);
    _BuildPacket(MSG_MOVE_SET_FACING);
}

void MovementMgr::MoveJump(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_PENDINGSTOP)))
        return;
    _moveFlags |= MOVEMENTFLAG_FALLING;
    Update(true);
    _BuildPacket(MSG_MOVE_JUMP);
}

bool MovementMgr::IsMoving(void)
{
    return _moveFlags & MOVEMENTFLAG_ANY_MOVE;
}

bool MovementMgr::IsTurning(void)
{
    return _moveFlags & (MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT);
}

bool MovementMgr::IsWalking(void)
{
    return _moveFlags & (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD);
}

bool MovementMgr::IsStrafing(void)
{
    return _moveFlags & (MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT);
}

