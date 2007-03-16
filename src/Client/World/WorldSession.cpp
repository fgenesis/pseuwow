#include "common.h"

#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "Auth/AuthCrypt.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "NameTables.h"
#include "RealmSocket.h"
#include "Channel.h"

#include "WorldSession.h"


WorldSession::WorldSession(PseuInstance *in)
{
    logdebug("-> Starting WorldSession from instance 0x%X",in); // should never output a null ptr
    _instance = in;
    _valid=_authed=_logged=false;
    _socket=new WorldSocket(_sh,this);
    _targetGUID=0; // no target
    _followGUID=0; // dont follow anything
    _myGUID=0; // i dont have a guid yet
    plrNameCache.ReadFromFile(); // load names/guids of known players
    ItemProtoCache_InsertDataToSession(this);
    _deleteme = false;
    _channels = new Channel(this);
    //...
}

WorldSession::~WorldSession()
{
    WorldPacket *packet;
    // clear the queue
    while(!pktQueue.empty())
    {
        packet = pktQueue.next();
        delete packet;
    }
    _OnLeaveWorld();

    delete _channels;
    //delete _socket; the socket will be deleted by its handler!!
}

void WorldSession::Start(void)
{
    log("Connecting to '%s' on port %u",GetInstance()->GetConf()->worldhost.c_str(),GetInstance()->GetConf()->worldport);
    _socket->Open(GetInstance()->GetConf()->worldhost,GetInstance()->GetConf()->worldport);
    GetInstance()->GetRSession()->SetCloseAndDelete(); // realm socket is no longer needed
    _valid=true;
    _sh.Add(_socket);
    _socket->SetDeleteByHandler();
    _sh.Select(1,0);
}

bool WorldSession::DeleteMe(void)
{
    return _deleteme;
}

void WorldSession::SetSocket(WorldSocket *sock)
{
    _socket = sock;
}

void WorldSession::AddToPktQueue(WorldPacket *pkt)
{
    pktQueue.add(pkt);
}

void WorldSession::SendWorldPacket(WorldPacket &pkt)
{
    if(GetInstance()->GetConf()->showmyopcodes)
        logcustom(0,BROWN,"<< Opcode %u [%s]", pkt.GetOpcode(), LookupName(pkt.GetOpcode(),g_worldOpcodeNames));
    _socket->SendWorldPacket(pkt);
}

void WorldSession::Update(void)
{
    if (!IsValid())
        return;

    if( _socket && _sh.GetCount() )
        _sh.Select(0,0);

    if(!_socket)
    {
        if(_valid)
        {
            _deleteme = true;
        }
        _logged=_authed=_valid=false;
        return;
    }


    OpcodeHandler *table = _GetOpcodeHandlerTable();
    bool known=false;
	while(pktQueue.size())
	{
		WorldPacket *packet = pktQueue.next();

		for (uint16 i = 0; table[i].handler != NULL; i++)
		{
			if (table[i].opcode == packet->GetOpcode())
			{
                try
                {
    				(this->*table[i].handler)(*packet);
                }
                catch (...)
                {
                    logerror("Exception while handling opcode %u!",packet->GetOpcode());
                    logerror("Data: pktsize=%u, handler=0x%X queuesize=%u",packet->size(),table[i].handler,pktQueue.size());
                }

				known=true;
				break;
			}
		}

		bool hideOpcode = false;

		// TODO: Maybe make table or something with all the frequently opcodes
		if (packet->GetOpcode() == SMSG_MONSTER_MOVE)
		{
			hideOpcode = true;
		}

		if( (known && GetInstance()->GetConf()->showopcodes==1)
			|| ((!known) && GetInstance()->GetConf()->showopcodes==2)
			|| (GetInstance()->GetConf()->showopcodes==3) )
		{
            if(!(GetInstance()->GetConf()->hidefreqopcodes && hideOpcode)) 
			    logcustom(1,YELLOW,">> Opcode %u [%s] (%s)", packet->GetOpcode(), LookupName(packet->GetOpcode(),g_worldOpcodeNames), known ? "Known" : "UNKNOWN");
		}
		delete packet;
        known=false;
	}

    _DoTimedActions();
    
}


OpcodeHandler *WorldSession::_GetOpcodeHandlerTable() const
{
    static OpcodeHandler table[] =
    {
        { SMSG_AUTH_CHALLENGE,       &WorldSession::_HandleAuthChallengeOpcode     },
        { SMSG_AUTH_RESPONSE,        &WorldSession::_HandleAuthResponseOpcode      },
        {SMSG_CHAR_ENUM,     &WorldSession::_HandleCharEnumOpcode},
        {SMSG_SET_PROFICIENCY, &WorldSession::_HandleSetProficiencyOpcode},
        {SMSG_ACCOUNT_DATA_MD5,  &WorldSession::_HandleAccountDataMD5Opcode},
        {SMSG_MESSAGECHAT, &WorldSession::_HandleMessageChatOpcode},
        {SMSG_NAME_QUERY_RESPONSE, &WorldSession::_HandleNameQueryResponseOpcode},
        {SMSG_PONG, &WorldSession::_HandlePongOpcode},
        {SMSG_TRADE_STATUS, &WorldSession::_HandleTradeStatusOpcode},
        {SMSG_GROUP_INVITE, &WorldSession::_HandleGroupInviteOpcode},
        {SMSG_CHANNEL_NOTIFY, &WorldSession::_HandleChannelNotifyOpcode},

        // movement opcodes
        {MSG_MOVE_SET_FACING, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_FORWARD, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_BACKWARD, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_STOP, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_STRAFE_LEFT, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_STRAFE_RIGHT, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_STOP_STRAFE, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_JUMP, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_TURN_LEFT, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_TURN_RIGHT, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_STOP_TURN, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_START_SWIM, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_STOP_SWIM, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_HEARTBEAT, &WorldSession::_HandleMovementOpcode},
        {MSG_MOVE_FALL_LAND, &WorldSession::_HandleMovementOpcode},

		{MSG_MOVE_TELEPORT_ACK, &WorldSession::_HandleTelePortAckOpcode},
        {SMSG_COMPRESSED_UPDATE_OBJECT, &WorldSession::_HandleCompressedUpdateObjectOpcode},
		{SMSG_UPDATE_OBJECT, &WorldSession::_HandleUpdateObjectOpcode},
	    {SMSG_CAST_RESULT, &WorldSession::_HandleCastResultOpcode},	
        {SMSG_ITEM_QUERY_SINGLE_RESPONSE, &WorldSession::_HandleItemQuerySingleResponseOpcode},
        {SMSG_DESTROY_OBJECT, &WorldSession::_HandleDestroyObjectOpcode},
        {SMSG_INITIAL_SPELLS, &WorldSession::_HandleInitialSpellsOpcode},

        // table termination
        { 0,                         NULL }
    };
    return table;
}

void WorldSession::SetTarget(uint64 guid)
{
    _targetGUID=guid;
    // TODO: update the UpdateFields once implemented
}

void WorldSession::SetFollowTarget(uint64 guid)
{
    _followGUID=guid;
}

void WorldSession::_OnEnterWorld(void)
{
    if(!_logged)
    {
        _logged=true;
        GetInstance()->GetScripts()->variables.Set("@inworld","true");
        GetInstance()->GetScripts()->RunScript("_enterworld",NULL);
        
    }
}

void WorldSession::_OnLeaveWorld(void)
{
    if(_logged)
    {
        _logged=false;
        GetInstance()->GetScripts()->RunScript("_leaveworld",NULL);
        GetInstance()->GetScripts()->variables.Set("@inworld","false");
    }
}

void WorldSession::_DoTimedActions(void)
{
    static clock_t pingtime=0;
    if(_logged)
    {
        if(pingtime < clock())
        {
            pingtime=clock() + 30*CLOCKS_PER_SEC;
            SendPing(clock());
        }
        //...
    }
}



///////////////////////////////////////////////////////////////
// Opcode Handlers
///////////////////////////////////////////////////////////////

void WorldSession::_HandleAuthChallengeOpcode(WorldPacket& recvPacket)
{
    std::string acc = stringToUpper(GetInstance()->GetConf()->accname);
	uint32 serverseed;
	recvPacket >> serverseed;
	logdebug("Auth: serverseed=0x%X",serverseed);
	Sha1Hash digest;
	digest.UpdateData(acc);
	uint32 unk=0;
	digest.UpdateData((uint8*)&unk,sizeof(uint32));
	BigNumber clientseed;
	clientseed.SetRand(8*4);
	uint32 clientseed_uint32=clientseed.AsDword();
	digest.UpdateData((uint8*)&clientseed_uint32,sizeof(uint32));
	digest.UpdateData((uint8*)&serverseed,sizeof(uint32));
	digest.UpdateBigNumbers(&(GetInstance()->GetSessionKey()),NULL);
	digest.Finalize();
	WorldPacket auth;
	auth<<(uint32)(GetInstance()->GetConf()->clientbuild)<<unk<<acc<<clientseed_uint32;
	auth.append(digest.GetDigest(),20);
	// recvPacket << real_size
	// recvPacket << ziped_UI_Plugins_Info
	// TODO: add addon data, simulate no addons.
	auth<<(uint32)0; // no addons? no idea, but seems to work. MaNGOS doesnt accept without this.
    auth.SetOpcode(CMSG_AUTH_SESSION);

	SendWorldPacket(auth);

    // note that if the sessionkey/auth is wrong or failed, the server sends the following packet UNENCRYPTED!
    // so its not 100% correct to init the crypt here, but it should do the job if authing was correct
	_socket->InitCrypt(GetInstance()->GetSessionKey().AsByteArray(), 40); 

    _authed=true;

}

void WorldSession::_HandleAuthResponseOpcode(WorldPacket& recvPacket)
{
    uint8 errcode;
    recvPacket >> errcode;
    if(errcode==0xC){
	    logdetail("World Authentication successful, preparing for char list request...");
        WorldPacket pkt;
        pkt.SetOpcode(CMSG_CHAR_ENUM);
	    SendWorldPacket(pkt);
    } else {
	    logcritical("World Authentication failed, errcode=0x%X",(unsigned char)errcode);
        GetInstance()->SetError();
    }
}

void WorldSession::_HandleCharEnumOpcode(WorldPacket& recvPacket)
{
    uint8 num;
	PlayerEnum plr[10]; // max characters per realm is 10
	uint8 dummy8;

	recvPacket >> num;
	if(num==0){
		logerror("No chars found!");
		GetInstance()->SetError();
		return;
	}
	logdetail("W: Chars in list: %u\n",num);
	for(unsigned int i=0;i<num;i++){
		recvPacket >> plr[i]._guid;
		recvPacket >> plr[i]._name;
		recvPacket >> plr[i]._race;
		recvPacket >> plr[i]._class;
		recvPacket >> plr[i]._gender;
		recvPacket >> plr[i]._bytes1;
		recvPacket >> plr[i]._bytes2;
		recvPacket >> plr[i]._bytes3;
		recvPacket >> plr[i]._bytes4;
		recvPacket >> plr[i]._bytesx;
		recvPacket >> plr[i]._level;
		recvPacket >> plr[i]._zoneId;
		recvPacket >> plr[i]._mapId;
		recvPacket >> plr[i]._x;
		recvPacket >> plr[i]._y;
		recvPacket >> plr[i]._z;
		recvPacket >> plr[i]._guildId;
		recvPacket >> dummy8;
		recvPacket >> plr[i]._flags;
		recvPacket >> dummy8 >> dummy8 >> dummy8;
		recvPacket >> plr[i]._petInfoId;
		recvPacket >> plr[i]._petLevel;
		recvPacket >> plr[i]._petFamilyId;
		for(unsigned int inv=0;inv<20;inv++)
        {
			recvPacket >> plr[i]._items[inv].displayId >> plr[i]._items[inv].inventorytype;
        }
        plrNameCache.AddInfo(plr[i]._guid, plr[i]._name);
	}
	bool char_found=false;
	int playerNum = 0;

	for(unsigned int i=0;i<num;i++){
		logcustom(0,LGREEN,"## %s (%u) [%s/%s]",
			plr[i]._name.c_str(),plr[i]._level,raceName[plr[i]._race],className[plr[i]._class]);
		logdetail("-> coords: map=%u zone=%u x=%f y=%f z=%f",
			plr[i]._mapId,plr[i]._zoneId,plr[i]._x,plr[i]._y,plr[i]._z);
        for(unsigned int inv=0;inv<20;inv++)
        {
            if(plr[i]._items[inv].displayId)
                logdebug("-> Has Item: Model=%u InventoryType=%u",plr[i]._items[inv].displayId,plr[i]._items[inv].inventorytype);
        }
        if(plr[i]._name==GetInstance()->GetConf()->charname)
        {
			char_found=true;
			_myGUID=plr[i]._guid;
            GetInstance()->GetScripts()->variables.Set("@myrace",toString(plr[i]._race));
			playerNum = i;
		}

	}
	if(!char_found){
		logerror("Character \"%s\" was not found on char list!",GetInstance()->GetConf()->charname.c_str());
		GetInstance()->SetError();
		return;
	} else {
		log("Entering World with Character \"%s\"...",GetInstance()->GetConf()->charname.c_str());
        // create the character and add it to the objmgr.
        MyCharacter *my = new MyCharacter();
        my->Create(_myGUID);
        objmgr.Add(my);

		WorldPacket pkt;
        pkt.SetOpcode(CMSG_PLAYER_LOGIN);
		pkt << _myGUID;
		SendWorldPacket(pkt);
	}
}


void WorldSession::_HandleSetProficiencyOpcode(WorldPacket& recvPacket)
{
    _OnEnterWorld();
}

void WorldSession::_HandleAccountDataMD5Opcode(WorldPacket& recvPacket)
{
    _OnEnterWorld();
}

void WorldSession::_HandleMessageChatOpcode(WorldPacket& recvPacket)
{
    uint8 type=0;
	uint32 lang=0;
	uint64 target_guid=0;
	uint32 msglen=0,unk;
	std::string msg,channel="";
    bool isCmd=false;

	recvPacket >> type >> lang;
	
	if (type == CHAT_MSG_CHANNEL)
    {
		recvPacket >> channel; // extract channel name
        recvPacket >> unk;
    }
		
	recvPacket >> target_guid;
    std::string plrname;
    if(target_guid){
        plrname=plrNameCache.GetName(target_guid);
        if(plrname.empty())
        {
            SendQueryPlayerName(target_guid);
            plrname="Unknown Entity";
        }
    }
	GetInstance()->GetScripts()->variables.Set("@thismsg_name",plrname);
	GetInstance()->GetScripts()->variables.Set("@thismsg",toString(target_guid));
	
	
	if(type == CHAT_MSG_SAY || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
		recvPacket >> target_guid;
	
	recvPacket >> msglen >> msg;
	if (type == CHAT_MSG_SYSTEM)
    {
		logcustom(0,WHITE,"SYSMSG: \"%s\"",msg.c_str());
	}
    else if (type==CHAT_MSG_WHISPER )
    {
        logcustom(0,WHITE,"WHISP: %s [%s]: %s",plrname.c_str(),LookupName(lang,langNames),msg.c_str());                
    }
    else if (type==CHAT_MSG_CHANNEL )
    {
        logcustom(0,WHITE,"CHANNEL: [%s]: %s [%s]: %s",channel.c_str(),plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else if (type==CHAT_MSG_SAY )
    {
        logcustom(0,WHITE,"CHAT: %s [%s]: %s",plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else if (type==CHAT_MSG_YELL )
    {
        logcustom(0,WHITE,"CHAT: %s yells [%s]: %s ",plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else if (type==CHAT_MSG_WHISPER_INFORM )
    {
        logcustom(0,WHITE,"TO %s [%s]: %s",plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else if (type==CHAT_MSG_GUILD )
    {
        logcustom(0,WHITE,"GUILD: %s [%s]: %s",plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else if (type==CHAT_MSG_PARTY )
    {
        logcustom(0,WHITE,"PARTY: %s [%s]: %s",plrname.c_str(),LookupName(lang,langNames),msg.c_str());  
    }
    else
    {
        logcustom(0,WHITE,"UNK CHAT TYPE (%u): %s [%s]: %s",type,plrname.c_str(),LookupName(lang,langNames),msg.c_str());
	}

    if(target_guid!=GetGuid() && msg.length()>1 && msg.at(0)=='-' && GetInstance()->GetConf()->allowgamecmd)
        isCmd=true;

    // some fun code :P
    if(type==CHAT_MSG_SAY && target_guid!=_myGUID && !isCmd)
    {
        // TODO: insert a good ChatAI here.
		if (GetInstance()->GetConf()->enablechatai)
		{
		    if(msg=="lol")
                SendChatMessage(CHAT_MSG_SAY,lang,"say \"lol\" if you have nothing else to say... lol xD","");
            else if(msg.length()>4 && msg.find("you?")!=std::string::npos)
                SendChatMessage(CHAT_MSG_SAY,lang,GetInstance()->GetScripts()->variables.Get("@version").append(" -- i am a bot, made by False.Genesis, my master."),"");
            else if(msg=="hi")
                SendChatMessage(CHAT_MSG_SAY,lang,"Hi, wadup?",""); 
            else if(msg.length()<12 && msg.find("wtf")!=std::string::npos)
                SendChatMessage(CHAT_MSG_SAY,lang,"Yeah, WTF is a good way to say you dont understand anything... :P","");
            else if(msg.length()<15 && (msg.find("omg")!=std::string::npos || msg.find("omfg")!=std::string::npos) )
                SendChatMessage(CHAT_MSG_SAY,lang,"OMG a bot logged in, you don't believe it :O","");
            else if(msg.find("from")!=std::string::npos || msg.find("download")!=std::string::npos)
                SendChatMessage(CHAT_MSG_SAY,lang,"you can dl me from http://my.opera.com/PseuWoW","");
            else if(msg.find("Genesis")!=std::string::npos || msg.find("genesis")!=std::string::npos)
                SendChatMessage(CHAT_MSG_YELL,lang,"False.Genesis, they are calling you!! Come here, master xD","");
        }
    }

    if(isCmd)
    {
		GetInstance()->GetScripts()->variables.Set("@thiscmd_name",plrname);
		GetInstance()->GetScripts()->variables.Set("@thiscmd",toString(target_guid));
        std::string lin=msg.substr(1,msg.length()-1);
        try
        {
            GetInstance()->GetScripts()->My_Run(lin,plrname);
        }
        catch (...)
        {
            SendChatMessage(CHAT_MSG_SAY,0,"Exception while trying to execute: [ "+lin+" ]","");
        }
        
    }
    if(type==CHAT_MSG_WHISPER && (!isCmd) && target_guid!=GetGuid())
    {
        GetInstance()->GetScripts()->variables.Set("@thiswhisper_name",plrname);
		GetInstance()->GetScripts()->variables.Set("@thiswhisper",toString(target_guid));
        GetInstance()->GetScripts()->variables.Set("@thiswhisper_lang",toString((uint64)lang));
        GetInstance()->GetScripts()->RunScript("_onwhisper",NULL);
    }

    // the following block searches for items in chat and queries them if they are unknown
    if(!isCmd && target_guid!=_myGUID && msg.length()>strlen(CHAT_ITEM_BEGIN_STRING))
    {
        for(uint32 pos=0;pos<msg.length()-strlen(CHAT_ITEM_BEGIN_STRING);pos++)
        {
            if(!memcmp(msg.c_str()+pos,CHAT_ITEM_BEGIN_STRING,strlen(CHAT_ITEM_BEGIN_STRING)))
            {
                std::string itemid;
                uint32 id;

                while(msg[pos] != ':')
                {
                    itemid += msg[pos+strlen(CHAT_ITEM_BEGIN_STRING)];
                    pos++;
                }
                id = atoi(itemid.c_str());
                logdebug("Found Item in chat message: %u",id);
                if(objmgr.GetItemProto(id)==NULL)
                    SendQueryItem(id,0);
            }
        }
    }

}
void WorldSession::_HandleNameQueryResponseOpcode(WorldPacket& recvPacket)
{
    uint64 pguid;
    std::string pname;
    recvPacket >> pguid >> pname;
    if(pname.length()>12 || pname.length()<2)
        return; // playernames maxlen=12, minlen=2
    // rest of the packet is not interesting for now
    if(plrNameCache.AddInfo(pguid,pname))
    {
        logdetail("CACHE: Assigned new player name: '%s' = " I64FMTD ,pname.c_str(),pguid);
        if(GetInstance()->GetConf()->debug > 1)
            SendChatMessage(CHAT_MSG_SAY,0,"Player "+pname+" added to cache.","");
    }
    WorldObject *wo = (WorldObject*)objmgr.GetObj(pguid);
    if(wo)
        wo->SetName(pname);
}

void WorldSession::_HandlePongOpcode(WorldPacket& recvPacket)
{
    uint32 pong;
    recvPacket >> pong;
    if(GetInstance()->GetConf()->notifyping)
        log("Recieved Ping reply: %u ms latency.",clock()-pong);
}
void WorldSession::_HandleTradeStatusOpcode(WorldPacket& recvPacket)
{
    recvPacket.hexlike();
    uint8 unk;
    recvPacket >> unk;
    if(unk==1)
    {
        SendChatMessage(CHAT_MSG_SAY,0,"It has no sense trying to trade with me, that feature is not yet implemented!","");
        WorldPacket pkt;
        pkt.SetOpcode(CMSG_CANCEL_TRADE);
        SendWorldPacket(pkt);
    }
}

void WorldSession::_HandleGroupInviteOpcode(WorldPacket& recvPacket)
{
    recvPacket.hexlike();
    WorldPacket pkt;
    pkt.SetOpcode(CMSG_GROUP_DECLINE);
    SendWorldPacket(pkt);
}

void WorldSession::_HandleMovementOpcode(WorldPacket& recvPacket)
{
    uint32 flags, time;
    float x, y, z, o;
    uint64 guid;
    std::string plrname;
    guid = recvPacket.GetPackedGuid();
    recvPacket >> flags >> time >> x >> y >> z >> o;
    /*if(guid){
        plrname=plrNameCache.GetName(guid);
        if(plrname.empty())
        {
            SendQueryPlayerName(guid);
            plrname="Unknown Entity";
        }
    }*/
}

void WorldSession::_HandleTelePortAckOpcode(WorldPacket& recvPacket)
{
	uint8 unk;
	uint16 unk1, unk2;
	uint32 unk3, unk4;
	uint64 guid;

	float x, y, z, o, ang;

	recvPacket >> unk >> guid >> unk3 >> unk1 >> unk2 >> o >> x >> y >> z >> ang >> unk4;

	logdetail("Got teleported, data: x: %f, y: %f, z: %f, o: %f, guid: "I64FMT, x, y, z, o, guid);

	WorldPacket response;
	response.SetOpcode(MSG_MOVE_FALL_LAND);
	response << uint32(0) << uint32(0) << x << y << z << o << uint32(0);
	SendWorldPacket(response);
}

void WorldSession::_HandleChannelNotifyOpcode(WorldPacket& recvPacket)
{
	_channels->HandleNotifyOpcode(recvPacket);
}

void WorldSession::_HandleCastResultOpcode(WorldPacket& recvPacket)
{
    uint32 spellid;
    uint8 flag,result;
    recvPacket >> spellid >> flag;
    if(flag)
    {
        recvPacket >> result;
        logdetail("Cast of spell %u failed. flag=%u, result=%u",spellid,flag,result);
    }
    else
    {
        logdetail("Cast of spell %u successful.",spellid);
    }
}

void WorldSession::_HandleInitialSpellsOpcode(WorldPacket& recvPacket)
{
    // suggestion for later: what about MyCharacter->AddSpellBookEntry() ?
	GetMyChar()->SetSpells(recvPacket);
}

