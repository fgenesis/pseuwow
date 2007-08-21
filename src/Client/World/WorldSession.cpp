#include "common.h"

#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "Auth/AuthCrypt.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "RealmSocket.h"
#include "Channel.h"
#include "ObjMgr.h"
#include "World.h"
#include "RealmSession.h"
#include "WorldSession.h"

struct OpcodeHandler
{
    uint16 opcode;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};


WorldSession::WorldSession(PseuInstance *in)
{
    logdebug("-> Starting WorldSession 0x%X from instance 0x%X",this,in); // should never output a null ptr
    _instance = in;
    _mustdie=false;
    _logged=false;
    _socket=NULL;
    _myGUID=0; // i dont have a guid yet
    _channels = new Channel(this);
    _world = NULL;
    _sh.SetAutoCloseSockets(false);
    objmgr.SetInstance(in);
    //...

    DEBUG(logdebug("WorldSession 0x%X constructor finished",this));
}

WorldSession::~WorldSession()
{
    WorldPacket *packet;
    // clear the queue
    while(pktQueue.size())
    {
        packet = pktQueue.next();
        delete packet;
    }

    if(_channels)
        delete _channels;
    if(_socket)
        delete _socket;
    if(_world)
        delete _world;
    DEBUG(logdebug("~WorldSession() this=0x%X _instance=0x%X",this,_instance));
}

void WorldSession::Start(void)
{
    log("Connecting to '%s' on port %u",GetInstance()->GetConf()->worldhost.c_str(),GetInstance()->GetConf()->worldport);
    _socket=new WorldSocket(_sh,this);
    _socket->Open(GetInstance()->GetConf()->worldhost,GetInstance()->GetConf()->worldport);
    if(GetInstance()->GetRSession())
    {
        GetInstance()->GetRSession()->SetMustDie(); // realm session is no longer needed
    }
    _sh.Add(_socket);

    // if we cant connect, wait until the socket gives up (after 5 secs)
    while( (!MustDie()) && (!_socket->IsOk()) && (!GetInstance()->Stopped()) )
    {
        _sh.Select(3,0);
        GetInstance()->Sleep(100);
    }
}

void WorldSession::_LoadCache(void)
{
    logdetail("Loading Cache...");
    plrNameCache.ReadFromFile(); // load names/guids of known players
    ItemProtoCache_InsertDataToSession(this);
    //...
}

void WorldSession::AddToPktQueue(WorldPacket *pkt)
{
    pktQueue.add(pkt);
}

void WorldSession::SendWorldPacket(WorldPacket &pkt)
{
    if(GetInstance()->GetConf()->showmyopcodes)
        logcustom(0,BROWN,"<< Opcode %u [%s]", pkt.GetOpcode(), GetOpcodeName(pkt.GetOpcode()));
    if(_socket && _socket->IsOk())
        _socket->SendWorldPacket(pkt);
    else
    {
        logerror("WorldSession: Can't send WorldPacket, socket doesn't exist or is not ready.");
    }
}

void WorldSession::Update(void)
{
    if( _sh.GetCount() ) // the socket will remove itself from the handler if it got closed
        _sh.Select(0,0);
    else // so we just need to check if the socket doesnt exist or if it exists but isnt valid anymore.
    {    // if thats the case, we dont need the session anymore either
        if(!_socket || (_socket && !_socket->IsOk()))
        {
            _OnLeaveWorld();
            SetMustDie();
        }
    }


    OpcodeHandler *table = _GetOpcodeHandlerTable();

    uint16 hpos;
    bool known=false;
        while(pktQueue.size())
        {
                WorldPacket *packet = pktQueue.next();

                for (hpos = 0; table[hpos].handler != NULL; hpos++)
                {
                        if (table[hpos].opcode == packet->GetOpcode())
                        {
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
                            logcustom(1,YELLOW,">> Opcode %u [%s] (%s, %u bytes)", packet->GetOpcode(), GetOpcodeName(packet->GetOpcode()), known ? "Known" : "UNKNOWN", packet->size());
                }

        if(known)
        {
            try
            {
                (this->*table[hpos].handler)(*packet);
            }
            catch (...)
            {
                logerror("Exception while handling opcode %u!",packet->GetOpcode());
                logerror("Data: pktsize=%u, handler=0x%X queuesize=%u",packet->size(),table[hpos].handler,pktQueue.size());
            }
        }

                delete packet;
        known=false;
        }

    _DoTimedActions();

    if(_world)
        _world->Update();
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
        {SMSG_LEARNED_SPELL, &WorldSession::_HandleLearnedSpellOpcode},
        {SMSG_REMOVED_SPELL, &WorldSession::_HandleLearnedSpellOpcode},
        {SMSG_CHANNEL_LIST, &WorldSession::_HandleChannelListOpcode},
        {SMSG_EMOTE, &WorldSession::_HandleEmoteOpcode},
        {SMSG_TEXT_EMOTE, &WorldSession::_HandleTextEmoteOpcode},
        {SMSG_NEW_WORLD, &WorldSession::_HandleNewWorldOpcode},
        {SMSG_LOGIN_VERIFY_WORLD, &WorldSession::_HandleLoginVerifyWorldOpcode},
        {SMSG_MOTD, &WorldSession::_HandleMotdOpcode},
        {SMSG_NOTIFICATION, &WorldSession::_HandleNotificationOpcode},

        // table termination
        { 0,                         NULL }
    };
    return table;
}

void WorldSession::SetTarget(uint64 guid)
{
    SendSetSelection(guid);
}

void WorldSession::_OnEnterWorld(void)
{
    if(!InWorld())
    {
        _logged=true;
        GetInstance()->GetScripts()->variables.Set("@inworld","true");
        GetInstance()->GetScripts()->RunScript("_enterworld",NULL);

    }
}

void WorldSession::_OnLeaveWorld(void)
{
    if(InWorld())
    {
        _logged=false;
        GetInstance()->GetScripts()->RunScript("_leaveworld",NULL);
        GetInstance()->GetScripts()->variables.Set("@inworld","false");
    }
}

void WorldSession::_DoTimedActions(void)
{
    static clock_t pingtime=0;
    if(InWorld())
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
    
        logdetail("Chars in list: %u\n",num);
        _LoadCache(); // we are about to login, so we need cache data
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

        for(unsigned int i=0;i<num;i++){
        logcustom(0,LGREEN,"## %s (%u) [%s/%s] Map: %s; Area: %s",
            plr[i]._name.c_str(),
            plr[i]._level,
            GetDBMgr().GetRaceName(plr[i]._race).c_str(),
            GetDBMgr().GetClassName_(plr[i]._class).c_str(),
            GetDBMgr().GetMapName(plr[i]._mapId).c_str(),
            GetDBMgr().GetAreaName(plr[i]._zoneId).c_str());
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
                }

        }
        if(!char_found){
                logerror("Character \"%s\" was not found on char list!",GetInstance()->GetConf()->charname.c_str());
                GetInstance()->SetError();
                return;
        } else {
                log("Entering World with Character \"%s\"...",GetInstance()->GetConf()->charname.c_str());

                // create the character and add it to the objmgr.
                // note: this is the only object that has to stay in memory unless its explicitly deleted by the server!
                // that means even if the server sends create object with that guid, do NOT recreate it!!
                MyCharacter *my = new MyCharacter();
                my->Create(_myGUID);
                objmgr.Add(my);

        // TODO: initialize the world here, and load required maps.
        // must remove appropriate code from _HandleLoginVerifyWorldOpcode() then!!

                WorldPacket pkt(CMSG_PLAYER_LOGIN,8);
                pkt << _myGUID;
                SendWorldPacket(pkt);
        }
}


void WorldSession::_HandleSetProficiencyOpcode(WorldPacket& recvPacket)
{
    if(recvPacket.size())
    {
        DEBUG(
            logdebug("SetProficiency: Hexdump:");
            logdebug(toHexDump((uint8*)recvPacket.contents(),recvPacket.size(),true).c_str());
            );
    }
}

void WorldSession::_HandleAccountDataMD5Opcode(WorldPacket& recvPacket)
{
    // packet structure not yet known
}

void WorldSession::_HandleMessageChatOpcode(WorldPacket& recvPacket)
{
    uint8 type=0;
    uint32 lang=0;
    uint64 source_guid=0;
    uint64 target_guid=0;
    uint32 msglen=0;
    uint32 unk=0;
    std::string msg,channel="";
    bool isCmd=false;

    recvPacket >> type >> lang;
    recvPacket >> source_guid >> unk; // added in 2.1.0
    if (type == CHAT_MSG_CHANNEL)
    {	
        recvPacket >> channel; // extract channel name
    }
    recvPacket >> target_guid >> msglen >> msg;

    std::string langname = GetDBMgr().GetLangName(lang);
    const char* ln = langname.c_str();

    std::string plrname;
    if(source_guid)
    {
        plrname=plrNameCache.GetName(source_guid);
        if(plrname.empty())
        {
            SendQueryPlayerName(source_guid);
            plrname="Unknown Entity";
        }
    }
    GetInstance()->GetScripts()->variables.Set("@thismsg_name",plrname);
    GetInstance()->GetScripts()->variables.Set("@thismsg",toString(target_guid));


    DEBUG(logdebug("Chat packet recieved, type=%u lang=%u src="I64FMT" dst="I64FMT" chn='%s' len=%u",
        type,lang,source_guid,target_guid,channel.c_str(),msglen));

    if (type == CHAT_MSG_SYSTEM)
    {
        logcustom(0,WHITE,"SYSMSG: \"%s\"",msg.c_str());
    }
    else if (type==CHAT_MSG_WHISPER )
    {
        logcustom(0,WHITE,"WHISP: %s [%s]: %s",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_CHANNEL )
    {
        logcustom(0,WHITE,"CHANNEL: [%s]: %s [%s]: %s",channel.c_str(),plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_SAY )
    {
        logcustom(0,WHITE,"CHAT: %s [%s]: %s",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_YELL )
    {
        logcustom(0,WHITE,"CHAT: %s yells [%s]: %s ",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_WHISPER_INFORM )
    {
        logcustom(0,WHITE,"TO %s [%s]: %s",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_GUILD )
    {
        logcustom(0,WHITE,"GUILD: %s [%s]: %s",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_PARTY )
    {
        logcustom(0,WHITE,"PARTY: %s [%s]: %s",plrname.c_str(),ln,msg.c_str());
    }
    else if (type==CHAT_MSG_EMOTE )
    {
        logcustom(0,WHITE,"EMOTE [%s]: %s %s",ln,plrname.c_str(),msg.c_str());
    }
    else
    {
        logcustom(0,WHITE,"UNK CHAT TYPE (%u): %s [%s]: %s",type,plrname.c_str(),ln,msg.c_str());
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
                SendChatMessage(CHAT_MSG_SAY,lang,"http://my.opera.com/PseuWoW","");
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
                if(id)
                {
                    logdebug("Found Item in chat message: %u",id);
                    if(objmgr.GetItemProto(id)==NULL)
                        SendQueryItem(id,0);
                }
            }
        }
    }
}

void WorldSession::_HandleMotdOpcode(WorldPacket& recvPacket)
{
    uint32 lines;
    std::string line;
    recvPacket >> lines;
    for(uint32 i = 0; i < lines; i++)
    {
        recvPacket >> line;
        logcustom(0,YELLOW,"MOTD: %s",line.c_str());
    }
}

void WorldSession::_HandleNotificationOpcode(WorldPacket& recvPacket)
{
    std::string text;
    recvPacket >> text;
    logcustom(0,YELLOW,"NOTIFY: %s",text.c_str());
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
        uint32 unk32,time;
        uint64 guid;

        float x, y, z, o;

    guid = recvPacket.GetPackedGuid();
        recvPacket >> unk32 >> unk32 >> time >> x >> y >> z >> o >> unk32;

        logdetail("Got teleported, data: x: %f, y: %f, z: %f, o: %f, guid: "I64FMT, x, y, z, o, guid);

    // TODO: put this into a capsule class later, that autodetects movement flags etc.
    WorldPacket response;
    response.SetOpcode(MSG_MOVE_FALL_LAND);
    response << uint32(0) << (uint32)getMSTime(); // no flags; time correct?
    response << x << y << z << o << uint32(0);
    SendWorldPacket(response);
    if(_world)
        _world->UpdatePos(x,y);
}

void WorldSession::_HandleNewWorldOpcode(WorldPacket& recvPacket)
{
    DEBUG(logdebug("DEBUG: _HandleNewWorldOpcode() objs:%u mychar: ptr=0x%X, guid="I64FMT,objmgr.GetObjectCount(),GetMyChar(),GetMyChar()->GetGUID()));
    uint32 mapid;
    float x,y,z,o;
    // we assume we are NOT on a transport!
    // else we had to do the following before:
    // recvPacket >> tmapid >> tx >> ty >> tz >> to;
    recvPacket >> mapid >> x >> y >> z >> o;
    if(GetMyChar())
        GetMyChar()->ClearSpells(); // will be resent by server
    // TODO: clear action buttons
    if(_world)
        delete _world;
    _world = new World(this);
    _world->UpdatePos(x,y,mapid);
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
        uint8 unk;
        uint16 spellid,spellslot,count;
        recvPacket >> unk >> count;
        logdebug("Got initial spells list, %u spells.",count);
        for(uint16 i = 0; i < count; i++)
        {
                recvPacket >> spellid >> spellslot;
                logdebug("Initial Spell: id=%u slot=%u",spellid,spellslot);

                GetMyChar()->AddSpell(spellid, spellslot);
        }
}

void WorldSession::_HandleLearnedSpellOpcode(WorldPacket& recvPacket)
{
        uint32 spellid;
        recvPacket >> spellid;
        GetMyChar()->AddSpell(spellid, 0); // other spells must be moved by +1 in slot?

        logdebug("Learned spell: id=%u",spellid);
}

void WorldSession::_HandleRemovedSpellOpcode(WorldPacket& recvPacket)
{
    uint32 spellid;
    recvPacket >> spellid;
    GetMyChar()->RemoveSpell(spellid);
    logdebug("Unlearned spell: id=%u",spellid);
}

void WorldSession::_HandleChannelListOpcode(WorldPacket& recvPacket)
{
        _channels->HandleListRequest(recvPacket);
}

void WorldSession::_HandleEmoteOpcode(WorldPacket& recvPacket)
{
    std::string plrname;
    uint32 anim; // animation id?
    uint64 guid; // guid of the unit performing the emote
    recvPacket >> anim >> guid;

    // TODO: check if the emote came from a player or a mob, and query mob name if it was a mob
    if(guid)
    {
        plrname=plrNameCache.GetName(guid);
        if(plrname.empty())
        {
            SendQueryPlayerName(guid);
            plrname="Unknown Entity";
        }
    }

    // TODO: check for mobs
    logdebug(I64FMT " / %s performing emote; anim=%u",guid,plrname.c_str(),anim);

    // TODO: show emote in GUI :P
}

void WorldSession::_HandleTextEmoteOpcode(WorldPacket& recvPacket)
{
    std::string name; // name of emote target
    std::string name_from; // name of the unit performing the emote
    uint32 emotetext; // emote id
    int32 emotev; // variation (like different texts on /flirt and so on)
    uint32 namelen; // length of name of emote target (without '\0' )
    uint64 guid; // guid of the unit performing the emote
    uint8 c; // temp

    recvPacket >> guid >> emotetext >> *((uint32*)&emotev) >> namelen;

    // get the target name, which is NOT null-terminated
    for(uint32 i = 0; i < namelen; i++)
    {
        recvPacket >> c;
        if(c)
            name += c;
    }

    logdebug(I64FMT " Emote: name=%s text=%u variation=%i len=%u",guid,name.c_str(),emotetext,emotev,namelen);
    SCPDatabaseMgr& dbmgr = GetInstance()->dbmgr;
    if(dbmgr.HasDB("emote"))
    {
        SCPDatabase& db = dbmgr.GetDB("emote");
        std::string target,target2;
        bool targeted=false; // if the emote is directed to anyone around or a specific target
        bool targeted_me=false; // if the emote was targeted to us if it was targeted
        bool from_me=false; // if we did the emote
        bool female=false; // if emote causer is female

        if(GetMyChar()->GetGUID() == guid) // we caused the emote
            from_me=true;

        if(name.length()) // is it directed to someone?
        {
            targeted=true; // if yes, we have a target
            if(GetMyChar()->GetName() == name) // if the name is ours, its directed to us
                targeted_me=true;
        }

        Unit *u = (Unit*)objmgr.GetObj(guid);
        if(u)
        {
            if(u->GetGender() != 0) // female
                female=true;
            name_from = u->GetName();
        }

        // if we targeted ourself, the general emote is used!
        if(targeted && from_me && targeted_me)
            targeted_me=false;

        // now build the string that is used to lookup the text in the database
        if(from_me)
            target += "me";
        else
            target += "one";

        if(targeted)
        {
            target += "to";
            if(targeted_me)
                target += "me";
            else
                target += "one";
        }
        else
            target += "general";

        // not all emotes have a female version, so check if there is one in the database
        if(female && db.GetField(emotetext).HasEntry(target + "female"))
                target += "female";

        logdebug("Looking up 'emote' SCP field %u entry '%s'",emotetext,target.c_str());

        std::string etext;
        etext = db.GetField(emotetext).GetString(target);

        char out[300]; // should be enough

        if(from_me)
            sprintf(out,etext.c_str(),name.c_str());
        else
            sprintf(out,etext.c_str(),name_from.c_str(),name.c_str());

        logcustom(0,WHITE,"EMOTE: %s",out);

    }
    else
    {
        logerror("Can't display emote text %u, SCP database \"emote\" not loaded.",emotetext);
    }

}

void WorldSession::_HandleLoginVerifyWorldOpcode(WorldPacket& recvPacket)
{
    float x,y,z,o;
    uint32 m;
    recvPacket >> m >> x >> y >> z >> o;
    // for now, init the world as soon as the server confirmed that we are where we are.
    logdebug("LoginVerifyWorld: map=%u x=%f y=%f z=%f o=%f",m,x,y,z,o);
    _OnEnterWorld();
    if(_world)
        delete _world;
    _world = new World(this);
    _world->UpdatePos(x,y,m);
}


// TODO: delete world on LogoutComplete once implemented


