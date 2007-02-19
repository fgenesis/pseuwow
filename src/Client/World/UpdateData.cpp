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
	while(true) // TODO: find out correct packet structure or this loop will fail!
	{
		recvPacket >> utype;
		logdebug("-UpdateObject: utype=%u",utype);
		switch(utype)
		{
		case UPDATETYPE_VALUES:
			{
				uint8 blockcount, masksize;
                uint32 value, valuesCount = GetValuesCountByTypeId(utype);
				uguid = recvPacket.GetPackedGuid();
                
                Object *obj = objmgr.GetObj(uguid,utype);

				if (obj)
				{
					recvPacket >> blockcount;
					masksize = blockcount * 4;
					logdebug("--UPDATETYPE_VALUES: guid="I64FMT" blockcount=%u masksize=%d",uguid,blockcount, masksize);

					uint32 *updateMask = new uint32[100];
					UpdateMask umask;
					umask.SetCount(masksize);
					recvPacket.read((uint8*)updateMask, masksize);
					umask.SetMask(updateMask);

					for (uint32 i = 0; i < valuesCount; i++) // How do we get valuesCount?
					{
						if (umask.GetBit(i))
						{
							recvPacket >> value;

							// These values are sent by the server as uint32 but must be stored at the client as float values
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

							//logdebug("Value (%d): %d", i, value);
						}
					}
				}
				else
				{
					logerror("--Got UpdateObject_Values for unknown object "I64FMT,uguid);
				}
			}
			break;

		case UPDATETYPE_MOVEMENT:
			{
                recvPacket >> uguid; // the guid is NOT packed here!
                Object *obj = objmgr.GetObj(uguid,utype);
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
                        logerror("--Got UPDATE_OBJECT for Object!"); // getting this should not be the case
                    }
                case TYPEID_ITEM:
                    {
                        Item *item = new Item();
                        // item needs to be created, e.g. item->Create(uguid);
                        objmgr.Add(item);
                        break;
                    }
                 //case TYPEID_CONTAINER: // not yet handled
                case TYPEID_UNIT:
                    {
                        logerror("--Got UPDATE_OBJECT for Unit!");
                        break;
                    }
                case TYPEID_PLAYER:
                    {
                    logdetail("--DEBUG: create player");
                    recvPacket.hexlike();
                    Player *player = new Player();
                    player->Create(uguid);
                    objmgr.Add(player);
                    break;
                    }
                }

                this->_MovementUpdate(objtypeid, uguid, recvPacket);
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
            return;
        } // switch
    } // while
} // func

void WorldSession::_MovementUpdate(uint8 objtypeid, uint64 uguid, WorldPacket& recvPacket)
{
	if(objtypeid==TYPEID_PLAYER)
	{
		uint32 flags, flags2, time;
		uint64 tguid;
		float nul;
		float x, y, z, o;
		float tx, ty, tz, to;
		float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn;

		recvPacket >> flags >> flags2 >> time;

		if (flags2 & 0x02000000) // On a transport
		{
			recvPacket >> x >> y >> z >> o >> tguid >> tx >> ty >> tz >> to; 
		}
		else
		{
			recvPacket >> x >> y >> z >> o;
		}

		recvPacket >> nul;

		if(flags2 & 0x2000) // Self update
		{
			// What is this data used for?
			recvPacket >> nul;
			recvPacket >> nul;
			recvPacket >> nul;
			recvPacket >> nul;
		}

		recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack >> speedTurn;

		logdebug("TYPEID_PLAYER: OnTransport=%s x=%d y=%d z=%d o=%d", flags2 & 0x02000000 ? "true" : "false", x, y, z, o);
	}
	if(objtypeid==TYPEID_UNIT)
	{
		uint32 flags2, unk, posCount;
		float nul, unkf;
		float x, y, z, o;
		float speedWalk, speedRun, speedSwimBack, speedSwim, speedWalkBack, speedTurn;

		recvPacket >> flags2 >> unk >> x >> y >> z >> o >> nul;
		recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack >> speedTurn;

		if (flags2 & 0x400000)
		{
			recvPacket >> unk >> unk >> unk >> unk >> posCount;

			for (uint8 i = 0; i < posCount + 1; i++)
			{
				recvPacket >> unkf >> unkf >> unkf; // Some x, y, z value
			}
		}

		logdebug("TYPEID_UNIT: 0x400000 flag=%s x=%d y=%d z=%d o=%d", flags2 & 0x400000 ? "true" : "false", x, y, z, o);
	}
	if( (objtypeid==TYPEID_CORPSE) || (objtypeid==TYPEID_GAMEOBJECT) || (objtypeid==TYPEID_DYNAMICOBJECT))
	{
		float x, y, z, o;

		recvPacket >> x >> y >> z >> o;

		// TODO: Check for transport and corpse extra data
	}
}