#include "common.h"
#include "ZCompressor.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateFields.h"
#include "Object.h"
#include "Unit.h"
#include "ObjMgr.h"
#include "UpdateMask.h"


void WorldSession::_HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket)
{
    //printf("-> COMPRESSED_UPDATE_OBJECT, pktlen=%u\n",recvPacket.size());
    //recvPacket.hexlike();
    uint32 realsize;
    recvPacket >> realsize;
    ZCompressor z;
    z.append(recvPacket.contents() + sizeof(uint32),recvPacket.size() - sizeof(uint32));
    z.Compressed(true);
    z.RealSize(realsize);
    z.Inflate();
    if(z.Compressed())
    {
        logerror("_HandleCompressedUpdateObjectOpcode(): Inflate() failed!");
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
	logdebug("UpdateObject: ublocks=%u unk=%u",ublocks,unk8);
	while(recvPacket.rpos() < recvPacket.size())
	{
		recvPacket >> utype;
		logdebug("-UpdateObject: utype=%u",utype);
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
                    logerror("--Got UpdateObject_Movement for unknown object "I64FMT,uguid);
			}
			break;

		case UPDATETYPE_CREATE_OBJECT:
		case UPDATETYPE_CREATE_OBJECT2:
			{
				uguid = recvPacket.GetPackedGuid();
				uint8 objtypeid;
				recvPacket >> objtypeid;
				logdebug("--Create Object type %u with guid "I64FMT,objtypeid,uguid);

                

                switch(objtypeid)
                {
                case TYPEID_OBJECT: // no data to read
                    {
                        Object *obj = new Object();
                        obj->Create(uguid);
                        objmgr.Add(obj);
                        break;
                    }
                case TYPEID_ITEM:
                    {
                        Item *item = new Item();
                        item->Create(uguid);
                        objmgr.Add(item);
                        break;
                    }
                 //case TYPEID_CONTAINER: // not yet handled
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
                }

                this->_MovementUpdate(objtypeid, uguid, recvPacket);
                this->_ValuesUpdate(uguid, recvPacket);
			}
			break;


		case UPDATETYPE_OUT_OF_RANGE_OBJECTS:
			recvPacket >> usize;
			for(uint16 i=0;i<usize;i++)
			{
				uguid = recvPacket.GetPackedGuid(); // not 100% sure if this is correct
				logdebug("--GUID "I64FMT" out of range",uguid);
				objmgr.Remove(uguid);
			}
			break;

		default:
            logerror("-Got unk updatetype 0x%X",utype);
            logerror("## Read %u / %u bytes",recvPacket.rpos(),recvPacket.size());
            return;
        } // switch
    } // while
    logdebug("## Read %u / %u bytes",recvPacket.rpos(),recvPacket.size());
    logdebug("## Parsing successful!!");
} // func

void WorldSession::_MovementUpdate(uint8 objtypeid, uint64 uguid, WorldPacket& recvPacket)
{
    uint8 flags;
    uint32 unk32,flags2,time,posCount,transtime;
    float unkf,x,y,z,o,tx,ty,tz,to;
    uint64 transguid;
    float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn;

    recvPacket >> flags;
    logdebug("--- flags:%X",flags);

	if(objtypeid==TYPEID_PLAYER)
	{
		recvPacket >> flags2 >> time;
        logdebug("--- flags2=0x%X time=%u",flags2,time);

		if (flags2 & 0x02000000) // On a transport
		{
			recvPacket >> x >> y >> z >> o >> transguid >> tx >> ty >> tz >> to; 
		}
		else
		{
			recvPacket >> x >> y >> z >> o;
		}

		recvPacket >> unkf;
        logdebug("--- DEBUG: %f == 0 ?",unkf);

		if(flags2 & 0x2000) // Self update
		{
			// What is this data used for?
			recvPacket >> unkf;
			recvPacket >> unkf;
			recvPacket >> unkf;
			recvPacket >> unkf;
		}

		recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack >> speedTurn;

        logdebug("--- TYPEID_PLAYER: walk=%f run=%f swimb=%f swim=%f walkb=%f turn=%f",speedWalk,speedRun,speedSwimBack,speedSwim,speedWalkBack,speedTurn);
		logdebug("--- TYPEID_PLAYER: OnTransport=%s x=%f y=%f z=%f o=%f", flags2 & 0x02000000 ? "true" : "false", x, y, z, o);

	}
	if(objtypeid==TYPEID_UNIT)
	{
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
        logdebug("--- TYPEID_UNIT: walk=%f run=%f swimb=%f swim=%f walkb=%f turn=%f",speedWalk,speedRun,speedSwimBack,speedSwim,speedWalkBack,speedTurn);
		logdebug("--- TYPEID_UNIT: flag=%s x=%f y=%f z=%f o=%f", flags2 & 0x400000 ? "true" : "false", x, y, z, o);
	}
	if( (objtypeid==TYPEID_CORPSE) || (objtypeid==TYPEID_GAMEOBJECT) || (objtypeid==TYPEID_DYNAMICOBJECT))
	{
        if(GUID_HIPART(uguid) != HIGHGUID_TRANSPORT)
        {
            recvPacket >> x >> y >> z;
        }
        else
        {
            recvPacket >> unk32 >> unk32 >> unk32; // should be 0?
        }
		recvPacket >> o;
	}

    recvPacket >> unk32; // (uint32)0x1
    logdebug("--- REFERENCE: %u (should be = 1)",unk32);

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
        logdebug("--UPDATETYPE_VALUES: guid="I64FMT" values=%u blockcount=%u masksize=%d",uguid,valuesCount,blockcount, masksize);

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

                logdebug("--- Value (%d): %d", i, value);
            }
        }
    }
    else
    {
        logerror("--Got UpdateObject_Values for unknown object "I64FMT,uguid);
    }

}