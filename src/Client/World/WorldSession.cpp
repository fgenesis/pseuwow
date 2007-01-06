#include "common.h"

#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "Auth/AuthCrypt.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "WorldSession.h"
#include "NameTables.h"


struct ClientPktHeader
{
    uint16 size;
    uint16 cmd;
	uint16 nil;
};

struct ServerPktHeader
{
    uint16 size;
    uint16 cmd;
};


WorldSession::WorldSession(PseuInstance *in)
{
    _instance = in;
    _valid=_authed=_logged=false;
    _socket=new WorldSocket(_sh,this);
    _socket->SetDeleteByHandler();
    _sh.Add(_socket);
    _targetGUID=0; // no target
    _followGUID=0; // dont follow anything
    plrNameCache.ReadFromFile(); // load names/guids of known players
    //...
}

WorldSession::~WorldSession()
{
    //delete _socket; the socket will be deleted by its handler!!
}

void WorldSession::AddToDataQueue(uint8 *data, uint32 len)
{
    for (uint32 i=0;i<len;i++)
        pktQueue.push_back(data[i]);
}

void WorldSession::SendWorldPacket(WorldPacket &pkt)
{
    ClientPktHeader hdr;
    memset(&hdr,0,sizeof(ClientPktHeader));
    hdr.size = ntohs(pkt.size()+4);
    hdr.cmd = pkt.GetOpcode();
    _crypt.EncryptSend((uint8*)&hdr, 6);
    ByteBuffer final(pkt.size()+6);
    final.append((uint8*)&hdr,sizeof(ClientPktHeader));
    final.append(pkt.contents(),pkt.size());
    _socket->SendBuf((char*)final.contents(),final.size());
}

void WorldSession::Update(void)
{
    if (GetInstance()->GetConf()->worldhost.empty() || GetInstance()->GetConf()->worldport==0)
        return;
    WorldPacket packet;
    OpcodeHandler *table = _GetOpcodeHandlerTable();
    while(pktQueue.size()>5)
    {
        packet = BuildWorldPacket();
        
        for (uint16 i = 0; table[i].handler != NULL; i++)
            if (table[i].opcode == packet.GetOpcode())
                (this->*table[i].handler)(packet);

        packet.clear();
    }
    // do more stuff here
}

WorldPacket WorldSession::BuildWorldPacket(void)
{
    ServerPktHeader hdr;
    WorldPacket wp;
    uint16 _remaining;
    for (uint8 i=0;i<sizeof(ServerPktHeader);i++)
    {
        ((uint8*)&hdr)[i] = pktQueue.front();
        pktQueue.pop_front();
    }
    _crypt.DecryptRecv((uint8*)&hdr,sizeof(ServerPktHeader));
    _remaining = ntohs(hdr.size)-2;
    wp.SetOpcode(hdr.cmd);
    for (uint16 i=0;i<_remaining;i++)
    {
        wp << pktQueue.front();
        pktQueue.pop_front();
    }
    return wp;
    
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

void WorldSession::OnEnterWorld(void)
{
    if(!_logged)
    {
        _logged=true;
        GetInstance()->GetScripts()->RunScriptByName("_enterworld",NULL,255);
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
	//DEBUG3(printf("W:auth: serverseed=0x%X\n",serverseed);)
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

	//DEBUG3(
	//	printf("CMSG_AUTH_SESSION=");
	//	printchex((char*)outpkt.contents(),outpkt.size(),true);
	//)
		SendWorldPacket(auth);
		_crypt.SetKey(GetInstance()->GetSessionKey().AsByteArray(), 40);
		_crypt.Init();
        _authed=true;

}

void WorldSession::_HandleAuthResponseOpcode(WorldPacket& recvPacket)
{
    uint8 errcode;
    recvPacket >> errcode;
    if(errcode==0xC){
	    //DEBUG1(printf("World Authentication successful, preparing for char list request...\n"););
        WorldPacket pkt;
        pkt.SetOpcode(CMSG_CHAR_ENUM);
	    SendWorldPacket(pkt);
    } else {
	    printf("World Authentication failed, errcode=0x%X\n",(unsigned char)errcode);
    }
}

void WorldSession::_HandleCharEnumOpcode(WorldPacket& recvPacket)
{
    uint8 num;
	PlayerEnum plr[10]; // max characters per realm is 10
	uint8 dummy8;
	uint32 dummy32;

	recvPacket >> num;
	if(num==0){
		printf("W:No chars found!\n");
		GetInstance()->Stop();
		return;
	}
	printf("W: Chars in list: %u\n",num);
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
		for(unsigned int inv=0;inv<20;inv++){
			recvPacket >> dummy32 >> dummy8; // item data are not relevant yet ( (uint32)itemID , (uint8)inventorytype )
		}
        plrNameCache.AddInfo(plr[i]._guid, plr[i]._name);
	}
	bool char_found=false;
	for(unsigned int i=0;i<num;i++){
		printf("## %s (%u) [%s/%s]\n",
			plr[i]._name.c_str(),plr[i]._level,raceName[plr[i]._race],className[plr[i]._class]);
		//DEBUG1(printf("-> coords: map=%u zone=%u x=%f y=%f z=%f\n",
		//	plr[i]._mapId,plr[i]._zoneId,plr[i]._x,plr[i]._y,plr[i]._z);)
        if(plr[i]._name==GetInstance()->GetConf()->charname){
			char_found=true;
			_myGUID=plr[i]._guid;
		}

	}
	if(!char_found){
		printf("W: Character \"%s\" was not found on char list!\n",GetInstance()->GetConf()->charname.c_str());
		GetInstance()->Stop();
		return;
	} else {
		printf("W: Entering World with Character \"%s\"...\n",GetInstance()->GetConf()->charname.c_str());
		WorldPacket pkt;
        pkt.SetOpcode(CMSG_PLAYER_LOGIN);
		pkt << _myGUID;
        _targetGUID=0;
        _followGUID=0;
		SendWorldPacket(pkt);
	}
}


void WorldSession::_HandleSetProficiencyOpcode(WorldPacket& recvPacket)
{
    OnEnterWorld();
}

void WorldSession::_HandleAccountDataMD5Opcode(WorldPacket& recvPacket)
{
    OnEnterWorld();
}

void WorldSession::_HandleMessageChatOpcode(WorldPacket& recvPacket)
{
    uint8 type=0;
	uint32 lang=0;
	uint64 target_guid=0;
	uint32 msglen=0;
	std::string msg,ext;
    bool isCmd=false;

	recvPacket >> type >> lang;
	
	if (type == CHAT_MSG_CHANNEL)
		recvPacket >> ext; // extract channel name
		
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
	/*defScp.variables.Set("@lastmsg_name",defScp.variables.Get("@thismsg_name"));
	defScp.variables.Set("@lastmsg",defScp.variables.Get("@lastmsg"));
	defScp.variables.Set("@thismsg_name",plrname);
	defScp.variables.Set("@thismsg",toString(target_guid));*/
	
	
	if(type == CHAT_MSG_SAY || type == CHAT_MSG_YELL || type == CHAT_MSG_PARTY)
		recvPacket >> target_guid;
	
	recvPacket >> msglen >> msg;
	if (type == CHAT_MSG_SYSTEM){
		printf("W:SYSMSG: \"%s\"\n",msg.c_str());
	} else if (type==CHAT_MSG_WHISPER ){
        printf("W:WHISP: %s [%s]: %s\n",plrname.c_str(),LookupName(lang,langNames),msg.c_str());                
    } else {
        printf("W:CHAT: %s [%s]: %s\n",plrname.c_str(),LookupName(lang,langNames),msg.c_str());
	}

    if(target_guid!=_myGUID && msg.length()>1 && msg.at(0)=='-')
        isCmd=true;

    // some fun code :P
    if(type==CHAT_MSG_SAY && target_guid!=_myGUID && !isCmd)
    {           
        /*if(msg=="lol")
            SendChatMessage(CHAT_MSG_SAY,lang,"say \"lol\" if you have nothing else to say... lol xD","");
        else if(msg.length()>4 && msg.find("you?")!=std::string::npos)
            SendChatMessage(CHAT_MSG_SAY,lang,std::string(ver).append(" -- i am a bot, made by False.Genesis, my master."),"");
        else if(msg=="hi")
            SendChatMessage(CHAT_MSG_SAY,lang,"Hi, wadup?",""); 
        else if(msg.length()<12 && msg.find("wtf")!=std::string::npos)
            SendChatMessage(CHAT_MSG_SAY,lang,"Yeah, WTF is a good way to say you dont understand anything... :P","");
        else if(msg.length()<15 && (msg.find("omg")!=std::string::npos || msg.find("omfg")!=std::string::npos) )
            SendChatMessage(CHAT_MSG_SAY,lang,"OMG a bot logged in, you don't believe it :O","");
        else if(msg.find("from")!=std::string::npos || msg.find("download")!=std::string::npos)
            SendChatMessage(CHAT_MSG_SAY,lang,"you can dl me from http://my.opera.com/PseuWoW","");
        else if(msg.find("Genesis")!=std::string::npos || msg.find("genesis")!=std::string::npos)
            SendChatMessage(CHAT_MSG_YELL,lang,"False.Genesis, they are calling you!! Come here, master xD","");*/         
    }

    if(isCmd)
    {
		/*defScp.variables.Set("@lastcmd_name",defScp.variables.Get("@thiscmd_name"));
		defScp.variables.Set("@lastcmd",defScp.variables.Get("@lastcmd"));
		defScp.variables.Set("@thiscmd_name",plrname);
		defScp.variables.Set("@thiscmd",toString(target_guid));
        std::string lin=msg.substr(1,msg.length()-1);
        uint8 perm=atoi(playerPermissions.Get(plrname).c_str());
        try{
            if(!defScp.RunSingleLine(lin,perm))
				defScp.RunScriptByName("_nopermission",NULL,255);
        } catch (...) {
            SendChatMessage(CHAT_MSG_SAY,0,"Exception while trying to execute: [ "+lin+" ]","");
        }*/
        
    }
    if(type==CHAT_MSG_WHISPER && !isCmd)
    {
		/*defScp.variables.Set("@lastwhisper_name",defScp.variables.Get("@thiswhisper_name"));
		defScp.variables.Set("@lastwhisper",defScp.variables.Get("@thiswhisper"));
		defScp.variables.Set("@lastwhisper_lang",defScp.variables.Get("@thiswhisper_lang"));
        defScp.variables.Set("@thiswhisper_name",plrname);
		defScp.variables.Set("@thiswhisper",toString(target_guid));
        defScp.variables.Set("@thiswhisper_lang",toString((uint64)lang));
        defScp.RunScriptByName("_onwhisper",NULL,255);*/
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
        printf("CACHE: Assigned new player name: '%s'",pname.c_str());
        SendChatMessage(CHAT_MSG_SAY,0,"Player "+pname+" added to cache.","");
        //DEBUG2(printf(" to guid "I64FMTD,pguid););
        printf("\n");
    }
}

void WorldSession::_HandlePongOpcode(WorldPacket& recvPacket)
{
    uint32 pong;
    recvPacket >> pong;
    printf("Recieved Ping reply: %u ms latency.\n",clock()-pong);
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
    if(guid){
        plrname=plrNameCache.GetName(guid);
        if(plrname.empty())
        {
            SendQueryPlayerName(guid);
            plrname="Unknown Entity";
        }
    }
    // for follow:
    //if(_followGUID==guid){
    //    ByteBuffer bb;
    //    bb << time << flags << x << y << z << o;
    //    SendWorldPacket(opcode,&bb);
    //}
    // more to come
}