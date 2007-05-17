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

		case UPDATETYPE_CREATE_OBJECT:
		case UPDATETYPE_CREATE_OBJECT2:
			{
				uguid = recvPacket.GetPackedGuid();
				uint8 objtypeid;
				recvPacket >> objtypeid;
				logdebug("Create Object type %u with guid "I64FMT,objtypeid,uguid);
                // dont create objects if already present in memory.
                // recreate every object except ourself!
                if( uguid != GetGuid() && objmgr.GetObj(uguid))
                {
                    logdev("- already exists, deleting old , creating new object");
                    objmgr.Remove(uguid);
                }

                switch(objtypeid)
                {
                case TYPEID_OBJECT: // no data to read
                    {
                        logerror("Recieved wrong UPDATETYPE_CREATE_OBJECT to create Object base type!");
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

                this->_MovementUpdate(objtypeid, uguid, recvPacket);
                this->_ValuesUpdate(uguid, recvPacket);

                _QueryObjectInfo(uguid);
			}
			break;


		case UPDATETYPE_OUT_OF_RANGE_OBJECTS:
			recvPacket >> usize;
			for(uint16 i=0;i<usize;i++)
			{
				uguid = recvPacket.GetPackedGuid(); // not 100% sure if this is correct
				logdebug("GUID "I64FMT" out of range",uguid);
				objmgr.Remove(uguid);
			}
			break;

		default:
            logerror("UPDATE_OBJECT: Got unk updatetype 0x%X",utype);
            logerror("UPDATE_OBJECT: Read %u / %u bytes, skipped rest",recvPacket.rpos(),recvPacket.size());
            return;
        } // switch
    } // while

} // func

void WorldSession::_MovementUpdate(uint8 objtypeid, uint64 uguid, WorldPacket& recvPacket)
{
    uint8 flags;
    uint32 unk32,flags2,time,transtime,higuid;
    float unkf,x,y,z,o,tx,ty,tz,to;
    uint64 transguid;
	// uint64 fullguid; // see below
    float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn, speedFly, speedFlyBack;

	Object *obj = (Object*)objmgr.GetObj(uguid);
	Unit *u = (Unit*)obj; // only use for Unit:: functions!!

    recvPacket >> flags;
	flags2 = 0; // not sure if its correct to set it to 0 (needs some starting flag?)

	if(flags & UPDATEFLAG_LIVING)
	{
			recvPacket >> flags2 >> time;
	}

	logdev("MovementUpdate TypeID=%u GUID="I64FMT" pObj=%X flags=%u flags2=%u",objtypeid,uguid,obj,flags,flags2);

	if(flags & UPDATEFLAG_HASPOSITION)
	{
		if(flags & UPDATEFLAG_TRANSPORT)
		{
			recvPacket >> unkf >> unkf >> unkf >> o; // 3x (float)0 followed by orientation
		}
		else
		{
			recvPacket >> x >> y >> z >> o;
		}
		
		if(flags2 & FLAGS2_TRANSPORT)
		{
			recvPacket >> transguid >> tx >> ty >> tz >> to;
			recvPacket >> unk32; // added in 2.0.3
		}
	}
	if(flags & UPDATEFLAG_LIVING)
	{
		recvPacket >> unk32;
		if(flags2 & 0x2000) // 0x2000 = ??
		{
			recvPacket >> unkf >> unkf >> unkf >> unkf;
		}
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
			logcustom(2,RED,"WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
		}
	}

	if(flags & UPDATEFLAG_ALL)
	{
		recvPacket >> unk32;
	}

	if(flags & UPDATEFLAG_HIGHGUID)
	{
		recvPacket >>  higuid;             // 2.0.6 - high guid was there, unk for 2.0.12
		// not sure if this is correct
		obj->SetUInt32Value(OBJECT_FIELD_GUID+1,higuid); // note that this sets only the high part of the guid
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
                recvPacket >> value;

                // TODO: what to do here?!
                /*if( obj->isType(TYPE_UNIT) && (
                    i >= UNIT_FIELD_POWER1         && i <= UNIT_FIELD_MAXPOWER5 ||
                    i >= UNIT_FIELD_BASEATTACKTIME && i <= UNIT_FIELD_RANGEDATTACKTIME ||
                    i >= UNIT_FIELD_STR            && i <= UNIT_FIELD_RESISTANCES + 6 )
                    || obj->isType(TYPE_PLAYER) &&
                    i >= PLAYER_FIELD_POSSTAT0 && i <= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6 )
                {
                    obj->SetFloatValue(i, (float)value);
                }
                else
                {*/

                    obj->SetUInt32Value(i, value);
                //}
					// still nee to find out which values to interpret as floats
					logdev("-> Field[%u] = %u",i,value);
            }
        }
    }
    else
    {
        logcustom(2,RED,"Got UpdateObject_Values for unknown object "I64FMT,uguid);
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
                std::string name = plrNameCache.GetName(guid);
                if(name.empty())
                {
                    SendQueryPlayerName(guid);
                }
                else
                {
                    ((WorldObject*)obj)->SetName(name);
                }
                break;
            }
        //case...
        }
    }
}