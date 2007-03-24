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
    uint32 unk32,flags2,time,posCount,transtime;
    float unkf,x,y,z,o,tx,ty,tz,to;
    uint64 transguid;
    float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn;

    recvPacket >> flags;

	if(objtypeid==TYPEID_PLAYER)
	{
        Unit *u = (Unit*)objmgr.GetObj(uguid);
		recvPacket >> flags2 >> time;

		if (flags2 & 0x02000000) // On a transport
		{
			recvPacket >> x >> y >> z >> o >> transguid >> tx >> ty >> tz >> to; 
		}
		else
		{
			recvPacket >> x >> y >> z >> o;
		}

		recvPacket >> unkf;

		if(flags2 & 0x2000) // Self update
		{
			// What is this data used for?
			recvPacket >> unkf;
			recvPacket >> unkf;
			recvPacket >> unkf;
			recvPacket >> unkf;
		}
		recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack >> speedTurn;
        if(u)
        {
            u->SetPosition(x,y,z,o);
            u->SetSpeed(MOVE_WALK,speedWalk);
            u->SetSpeed(MOVE_RUN,speedRun);
            u->SetSpeed(MOVE_SWIMBACK,speedSwimBack);
            u->SetSpeed(MOVE_SWIM,speedSwim);
            u->SetSpeed(MOVE_WALKBACK,speedWalkBack);
            u->SetSpeed(MOVE_TURN,speedTurn);
        }
        else
        {
            logcustom(2,RED,"WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
        }
	}
	if(objtypeid==TYPEID_UNIT)
	{
        Unit *u = (Unit*)objmgr.GetObj(uguid);
		recvPacket >> flags2 >> unk32 >> x >> y >> z >> o >> unkf;
		recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack >> speedTurn;

		if (flags2 & 0x400000)
		{
			recvPacket >> unk32 >> unk32 >> unk32 >> unk32 >> posCount;

			for (uint8 i = 0; i < posCount + 1; i++)
			{
				recvPacket >> unkf >> unkf >> unkf; // Some x, y, z value
			}
		}
        if(u)
        {
            u->SetPosition(x,y,z,o);
            u->SetSpeed(MOVE_WALK,speedWalk);
            u->SetSpeed(MOVE_RUN,speedRun);
            u->SetSpeed(MOVE_SWIMBACK,speedSwimBack);
            u->SetSpeed(MOVE_SWIM,speedSwim);
            u->SetSpeed(MOVE_WALKBACK,speedWalkBack);
            u->SetSpeed(MOVE_TURN,speedTurn);
        }
        else
        {
            logcustom(2,RED,"WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
        }
	}
	if( (objtypeid==TYPEID_CORPSE) || (objtypeid==TYPEID_GAMEOBJECT) || (objtypeid==TYPEID_DYNAMICOBJECT))
	{
        Unit *u = (Unit*)objmgr.GetObj(uguid);
        if(GUID_HIPART(uguid) != HIGHGUID_TRANSPORT)
        {
            recvPacket >> x >> y >> z;
            if(u)
                u->SetPosition(x,y,z,u->GetO());
            else
                logcustom(2,RED,"WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
        }
        else
        {
            recvPacket >> unk32 >> unk32 >> unk32; // should be 0?
        }
		recvPacket >> o;
        if(u)
            u->SetPosition(u->GetX(),u->GetY(),u->GetZ(),o);
        else
            logcustom(2,RED,"WorldSession::_MovementUpdate for unknown guid "I64FMT" typeid=%u",uguid,objtypeid);
	}

    recvPacket >> unk32; // (uint32)0x1

    if ((GUID_HIPART(uguid) == HIGHGUID_TRANSPORT))
    {
        recvPacket >> transtime;
    }

    if(  GUID_HIPART(uguid) == HIGHGUID_PLAYER_CORPSE)
        recvPacket >> unk32; // ??

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

        for (uint32 i = 0; i < valuesCount; i++)
        {
            if (umask.GetBit(i))
            {
                recvPacket >> value;

                // TODO: which values are float and which values are uin32 ??!
                if( obj->isType(TYPE_UNIT) && (
                    i >= UNIT_FIELD_POWER1         && i <= UNIT_FIELD_MAXPOWER5 ||
                    i >= UNIT_FIELD_BASEATTACKTIME && i <= UNIT_FIELD_RANGEDATTACKTIME ||
                    i >= UNIT_FIELD_STR            && i <= UNIT_FIELD_RESISTANCES + 6 )
                    || obj->isType(TYPE_PLAYER) &&
                    i >= PLAYER_FIELD_POSSTAT0 && i <= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6 )
                {
                    obj->SetFloatValue(i, (float)value);
                }
                else
                {

                    obj->SetUInt32Value(i, value);
                }
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