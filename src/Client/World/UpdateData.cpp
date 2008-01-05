#include "common.h"
#include "ZCompressor.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateFields.h"
#include "Object.h"
#include "Unit.h"
#include "Bag.h"
#include "GameObject.h"
#include "Corpse.h"
#include "DynamicObject.h"
#include "ObjMgr.h"
#include "UpdateMask.h"


void WorldSession::_HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket)
{
    uint32 realsize;
    recvPacket >> realsize;
    ZCompressor z;
    z.append(recvPacket.contents() + sizeof(uint32),recvPacket.size() - sizeof(uint32));
    z.Compressed(true);
    z.RealSize(realsize);
    z.Inflate();
    if(z.Compressed())
    {
        logerror("_HandleCompressedUpdateObjectOpcode(): Inflate() failed! size=%u realsize=%u",z.size(),realsize);
        return;
    }
    WorldPacket wp;
    wp.SetOpcode(recvPacket.GetOpcode());
    wp.append(z.contents(),z.size());

    _HandleUpdateObjectOpcode(wp);
}

void WorldSession::_HandleUpdateObjectOpcode(WorldPacket& recvPacket)
{
    uint8 utype;
    uint8 unk8;
    uint32 usize, ublocks;
    uint64 uguid;
    recvPacket >> ublocks >> unk8;
    while(recvPacket.rpos() < recvPacket.size())
    {
        recvPacket >> utype;
        switch(utype)
        {
            case UPDATETYPE_VALUES:
            {
                uguid = recvPacket.GetPackedGuid();
                _ValuesUpdate(uguid,recvPacket);
            }
            break;

            case UPDATETYPE_MOVEMENT:
            {
                recvPacket >> uguid; // the guid is NOT packed here!
                Object *obj = objmgr.GetObj(uguid);
                if(obj)
                    this->_MovementUpdate(obj->GetTypeId(),uguid,recvPacket);
                else
                    logcustom(2,RED,"Got UpdateObject_Movement for unknown object "I64FMT,uguid);
            }
            break;

            case UPDATETYPE_CREATE_OBJECT2: // will be sent when our very own character is created
            case UPDATETYPE_CREATE_OBJECT: // will be sent on any other object creation
            {
                uguid = recvPacket.GetPackedGuid();
                uint8 objtypeid;
                recvPacket >> objtypeid;
                logdebug("Create Object type %u with guid "I64FMT,objtypeid,uguid);
                // dont create objects if already present in memory.
                // recreate every object except ourself!
                if(objmgr.GetObj(uguid))
                {
                    if(uguid != GetGuid())
                    {
                        logdev("- already exists, deleting old , creating new object");
                        objmgr.Remove(uguid);
                        // do not call script here, since the object does not really get deleted
                    }
                    else
                    {
                        logdev("- already exists, but not deleted (has our current GUID)");
                    }
                }

                // only if the obj didnt exist or was just deleted above, create it....
                if(!objmgr.GetObj(uguid))
                {
                    switch(objtypeid)
                    {
                    case TYPEID_OBJECT: // no data to read
                        {
                            logerror("Recieved wrong UPDATETYPE_CREATE_OBJECT to create Object base type!");
                            logerror("%s",toHexDump((uint8*)recvPacket.contents(),recvPacket.size(),true).c_str());
                        }
                    case TYPEID_ITEM:
                        {
                            Item *item = new Item();
                            item->Create(uguid);
                            objmgr.Add(item);
                            break;
                        }
                    case TYPEID_CONTAINER:
                        {
                            Bag *bag = new Bag();
                            bag->Create(uguid);
                            objmgr.Add(bag);
                            break;
                        }
                    case TYPEID_UNIT:
                        {
                            Unit *unit = new Unit();
                            unit->Create(uguid);
                            objmgr.Add(unit);
                            break;
                        }
                    case TYPEID_PLAYER:
                        {
                            Player *player = new Player();
                            player->Create(uguid);
                            objmgr.Add(player);
                            break;
                        }
                    case TYPEID_GAMEOBJECT:
                        {
                            GameObject *go = new GameObject();
                            go->Create(uguid);
                            objmgr.Add(go);
                            break;
                        }
                    case TYPEID_CORPSE:
                        {
                            Corpse *corpse = new Corpse();
                            corpse->Create(uguid);
                            objmgr.Add(corpse);
                            break;
                        }
                    case TYPEID_DYNAMICOBJECT:
                        {
                            DynamicObject *dobj = new DynamicObject();
                            dobj->Create(uguid);
                            objmgr.Add(dobj);
                            break;
                        }
                    }
                }
                else
                {
                    logdebug("Obj "I64FMT" not created, already exists",uguid);
                }
                // ...regardless if it was freshly created or already present, update its values and stuff now...
                this->_MovementUpdate(objtypeid, uguid, recvPacket);
                this->_ValuesUpdate(uguid, recvPacket);

                // ...and ask the server for eventually missing data.
                _QueryObjectInfo(uguid);


                // call script "_OnObjectCreate"
                if(GetInstance()->GetScripts()->ScriptExists("_onobjectcreate"))
                {
                    CmdSet Set;
                    Set.defaultarg = toString(uguid);
                    Set.arg[0] = toString(objtypeid);
                    GetInstance()->GetScripts()->RunScript("_onobjectcreate", &Set);
                }

                // if our own character got finally created, we have successfully entered the world,
                // and should have gotten all info about our char already.
            }
            break;

            case UPDATETYPE_OUT_OF_RANGE_OBJECTS:
            {
                recvPacket >> usize;
                for(uint16 i=0;i<usize;i++)
                {
                    uguid = recvPacket.GetPackedGuid(); // not 100% sure if this is correct
                    logdebug("GUID "I64FMT" out of range",uguid);

                    // call script just before object removal
                    if(GetInstance()->GetScripts()->ScriptExists("_onobjectdelete"))
                    {
                        CmdSet Set;
                        Set.defaultarg = toString(uguid);
                        Set.arg[0] = "true"; // out of range = true
                        GetInstance()->GetScripts()->RunScript("_onobjectdelete", &Set);
                    }

                    objmgr.Remove(uguid);
                }
            }
            break;

            default:
            {
                logerror("UPDATE_OBJECT: Got unk updatetype 0x%X",utype);
                logerror("UPDATE_OBJECT: Read %u / %u bytes, skipped rest",recvPacket.rpos(),recvPacket.size());
                logerror("%s",toHexDump((uint8*)recvPacket.contents(),recvPacket.size(),true).c_str());
                return;
            }
        } // switch
    } // while

} // func

void WorldSession::_MovementUpdate(uint8 objtypeid, uint64 uguid, WorldPacket& recvPacket)
{
    uint8 flags,unk8;
    uint32 unk32,flags2,time,transtime,higuid;
    float unkfx,unkfy,unkfz,x,y,z,o,tx,ty,tz,to;
    uint64 transguid;
    // uint64 fullguid; // see below
    float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn, speedFly, speedFlyBack;

    Object *obj = (Object*)objmgr.GetObj(uguid);
    Unit *u = (Unit*)obj; // only use for Unit:: functions!!

    recvPacket >> flags;
    flags2 = 0; // not sure if its correct to set it to 0 (needs some starting flag?)

    if(flags & UPDATEFLAG_LIVING)
    {
        recvPacket >> flags2 >> unk8 >> time;
    }

    logdev("MovementUpdate TypeID=%u GUID="I64FMT" pObj=%X flags=%u flags2=%u",objtypeid,uguid,obj,flags,flags2);

    if(flags & UPDATEFLAG_HASPOSITION)
    {
        if(flags & UPDATEFLAG_TRANSPORT)
        {
            recvPacket >> unkfx >> unkfy >> unkfz >> o; // 3x (float)0 followed by orientation
            logdev("TRANSPORT_FLOATS @ flags: x=%f y=%f z=%f o=%f",unkfx,unkfy,unkfz,o);
        }
        else
        {
            recvPacket >> x >> y >> z >> o;
            logdev("FLOATS: x=%f y=%f z=%f o=%f",x,y,z,o);
        }
    }

    if(flags & UPDATEFLAG_LIVING)
    {
        if(flags2 & FLAGS2_TRANSPORT)
        {
            recvPacket >> transguid >> tx >> ty >> tz >> to;
            recvPacket >> unk32; // added in 2.0.3
            logdev("TRANSPORT_FLOATS @ flags2: x=%f y=%f z=%f o=%f",tx,ty,tz,to);
        }

        recvPacket >> unk32;

        /*
        // not sure if this is/was correct, MaNGOS doesnt use it anymore
                if(flags2 & 0x2000) // 0x2000 = ??
                {
                        recvPacket >> unkf >> unkf >> unkf >> unkf;
                }
        */

        recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim;
        recvPacket >> speedWalkBack >> speedFly >> speedFlyBack >> speedTurn; // fly added in 2.0.x
        if(u)
        {
                u->SetPosition(x,y,z,o);
                u->SetSpeed(MOVE_WALK,speedWalk);
                u->SetSpeed(MOVE_RUN,speedRun);
                u->SetSpeed(MOVE_SWIMBACK,speedSwimBack);
                u->SetSpeed(MOVE_SWIM,speedSwim);
                u->SetSpeed(MOVE_WALKBACK,speedWalkBack);
                u->SetSpeed(MOVE_TURN,speedTurn);
                u->SetSpeed(MOVE_FLY,speedFly);
                u->SetSpeed(MOVE_FLYBACK,speedFlyBack);
        }
        else
        {
                logerror("WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
        }
    }

    if(flags & UPDATEFLAG_ALL)
    {
        recvPacket >> unk32;
    }

    if(flags & UPDATEFLAG_HIGHGUID)
    {
        recvPacket >>  higuid;             // 2.0.6 - high guid was there, unk for 2.0.12
        // not sure if this is correct, MaNGOS sends 0 always.
        //obj->SetUInt32Value(OBJECT_FIELD_GUID+1,higuid); // note that this sets only the high part of the guid
    }

    if(flags & UPDATEFLAG_FULLGUID)
    {
        // unused in mangos? but what if its needed?
        // recvPacket >> fullguid;
    }

    if(flags & UPDATEFLAG_TRANSPORT)
    {
        recvPacket >>  transtime; // whats this used for?
    }

}

void WorldSession::_ValuesUpdate(uint64 uguid, WorldPacket& recvPacket)
{
    Object *obj = objmgr.GetObj(uguid);
    uint8 blockcount;
    uint32 value, masksize, valuesCount;
    float fvalue;

    if (obj)
    {
        valuesCount = obj->GetValuesCount();
        recvPacket >> blockcount;
        masksize = blockcount << 2; // each sizeof(uint32) == <4> * sizeof(uint8) // 1<<2 == <4>
        UpdateMask umask;
        uint32 *updateMask = new uint32[blockcount];
        umask.SetCount(masksize);
        recvPacket.read((uint8*)updateMask, masksize);
        umask.SetMask(updateMask);
        //delete [] updateMask; // will be deleted at ~UpdateMask() !!!!
        logdev("ValuesUpdate TypeId=%u GUID="I64FMT" pObj=%X Blocks=%u Masksize=%u",obj->GetTypeId(),uguid,obj,blockcount,masksize);

        for (uint32 i = 0; i < valuesCount; i++)
        {
            if (umask.GetBit(i))
            {
                if(IsFloatField(obj->GetTypeId(),i))
                {
                    recvPacket >> fvalue;
                    obj->SetFloatValue(i, fvalue);
                    logdev("-> Field[%u] = %f",i,fvalue);
                }
                else
                {
                    recvPacket >> value;
                    obj->SetUInt32Value(i, value);
                    logdev("-> Field[%u] = %u",i,value);
                }
                
            }
        }
    }
    else
    {
        logerror("Got UpdateObject_Values for unknown object "I64FMT,uguid);
    }

}

void WorldSession::_QueryObjectInfo(uint64 guid)
{
    Object *obj = objmgr.GetObj(guid);
    if(obj)
    {
        switch(obj->GetTypeId())
        {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            {
                ItemProto *proto = objmgr.GetItemProto(obj->GetEntry());
                if(!proto)
                {
                    logdebug("Found unknown item: GUID="I64FMT" entry=%u",obj->GetGUID(),obj->GetEntry());
                    SendQueryItem(obj->GetEntry(),obj->GetGUID()); // not sure if sending GUID is correct
                }
                break;
            }
        case TYPEID_PLAYER:
            {
                std::string name = GetOrRequestPlayerName(obj->GetGUID());
                if(!name.empty())
                {
                    ((WorldObject*)obj)->SetName(name);
                }
                // else: name will be set when server answers (_HandleNameQueryResponseOpcode)
                break;
            }
        //case...
        }
    }
}

// helper to determine if an updatefield should store float or int values, depending on TypeId
bool IsFloatField(uint8 tyid, uint32 f)
{
    static bool first_use = true;
    static uint32 *ty[TYPEID_CORPSE+1];

    static uint32 floats_object[] =
    {
        (uint32)OBJECT_FIELD_SCALE_X,
        (uint32)-1
    };
    static uint32 floats_item[] =
    {
        (uint32)-1
    };
    static uint32 floats_container[] =
    {
        (uint32)-1
    };
    static uint32 floats_unit[] =
    {
        (uint32)UNIT_FIELD_BOUNDINGRADIUS,
        (uint32)UNIT_FIELD_COMBATREACH,
        (uint32)UNIT_FIELD_MINDAMAGE,
        (uint32)UNIT_FIELD_MAXDAMAGE,
        (uint32)UNIT_FIELD_MINOFFHANDDAMAGE,
        (uint32)UNIT_FIELD_MINOFFHANDDAMAGE,
        (uint32)UNIT_MOD_CAST_SPEED,
        (uint32)UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER,
        (uint32)UNIT_FIELD_ATTACK_POWER_MULTIPLIER,
        (uint32)UNIT_FIELD_MINRANGEDDAMAGE,
        (uint32)UNIT_FIELD_MAXRANGEDDAMAGE,
        (uint32)UNIT_FIELD_POWER_COST_MULTIPLIER,
        (uint32)-1
    };
    static uint32 floats_player[] =
    {
        (uint32)PLAYER_BLOCK_PERCENTAGE,
        (uint32)PLAYER_DODGE_PERCENTAGE,
        (uint32)PLAYER_PARRY_PERCENTAGE,
        (uint32)PLAYER_RANGED_CRIT_PERCENTAGE,
        (uint32)PLAYER_OFFHAND_CRIT_PERCENTAGE,
        (uint32)PLAYER_SPELL_CRIT_PERCENTAGE1,
        (uint32)PLAYER_HOLY_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_FIRE_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_NATURE_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_FROST_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_SHADOW_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_ARCANE_SPELL_CRIT_PERCENTAGE,
        (uint32)PLAYER_FIELD_MOD_MANA_REGEN,
        (uint32)PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT,
        (uint32)-1
    };
    static uint32 floats_gameobject[] =
    {
        (uint32)GAMEOBJECT_ROTATION,
        (uint32)GAMEOBJECT_POS_X,
        (uint32)GAMEOBJECT_POS_Y,
        (uint32)GAMEOBJECT_POS_Z,
        (uint32)GAMEOBJECT_FACING,
        (uint32)-1
    };
    static uint32 floats_dynobject[] =
    {
        (uint32)DYNAMICOBJECT_RADIUS,
        (uint32)DYNAMICOBJECT_POS_X,
        (uint32)DYNAMICOBJECT_POS_Y,
        (uint32)DYNAMICOBJECT_POS_Z,
        (uint32)-1
    };
    static uint32 floats_corpse[] =
    {
        (uint32)CORPSE_FIELD_FACING,
        (uint32)CORPSE_FIELD_POS_X,
        (uint32)CORPSE_FIELD_POS_Y,
        (uint32)CORPSE_FIELD_POS_Z,
        (uint32)-1
    };

    if(first_use)
    {
        first_use = true;
        ty[TYPEID_OBJECT] = &floats_object[0];
        ty[TYPEID_ITEM] = &floats_item[0];
        ty[TYPEID_CONTAINER] = &floats_container[0];
        ty[TYPEID_UNIT] = &floats_unit[0];
        ty[TYPEID_PLAYER] = &floats_player[0];
        ty[TYPEID_GAMEOBJECT] = &floats_gameobject[0];
        ty[TYPEID_DYNAMICOBJECT] = &floats_dynobject[0];
        ty[TYPEID_CORPSE] = &floats_corpse[0];
    }

    for(uint32 i = 0; ty[tyid][i] != (-1); i++)
        if(ty[tyid][i] == f)
            return true;
    
    return false;
}
