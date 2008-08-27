#include "common.h"
#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "PseuWoW.h"
#include "RealmSocket.h"
#include "RealmSession.h"

enum AuthCmd
{
    //AUTH_NO_CMD                 = 0xFF,
    AUTH_LOGON_CHALLENGE        = 0x00,
    AUTH_LOGON_PROOF            = 0x01,
    //AUTH_RECONNECT_CHALLENGE    = 0x02,
    //AUTH_RECONNECT_PROOF        = 0x03,
    //update srv =4
    REALM_LIST                  = 0x10,
    XFER_INITIATE               = 0x30,
    XFER_DATA                   = 0x31,
    XFER_ACCEPT                 = 0x32,
    XFER_RESUME                 = 0x33,
    XFER_CANCEL                 = 0x34
};

enum eAuthResults
{
    REALM_AUTH_SUCCESS = 0,
    REALM_AUTH_FAILURE=0x01,                                ///< Unable to connect
    REALM_AUTH_UNKNOWN1=0x02,                               ///< Unable to connect
    REALM_AUTH_ACCOUNT_BANNED=0x03,                         ///< This <game> account has been closed and is no longer available for use. Please go to <site>/banned.html for further information.
    REALM_AUTH_NO_MATCH=0x04,                               ///< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    REALM_AUTH_UNKNOWN2=0x05,                               ///< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    REALM_AUTH_ACCOUNT_IN_USE=0x06,                         ///< This account is already logged into <game>. Please check the spelling and try again.
    REALM_AUTH_PREPAID_TIME_LIMIT=0x07,                     ///< You have used up your prepaid time for this account. Please purchase more to continue playing
    REALM_AUTH_SERVER_FULL=0x08,                            ///< Could not log in to <game> at this time. Please try again later.
    REALM_AUTH_WRONG_BUILD_NUMBER=0x09,                     ///< Unable to validate game version. This may be caused by file corruption or interference of another program. Please visit <site> for more information and possible solutions to this issue.
    REALM_AUTH_UPDATE_CLIENT=0x0a,                          ///< Downloading
    REALM_AUTH_UNKNOWN3=0x0b,                               ///< Unable to connect
    REALM_AUTH_ACCOUNT_FREEZED=0x0c,                        ///< This <game> account has been temporarily suspended. Please go to <site>/banned.html for further information
    REALM_AUTH_UNKNOWN4=0x0d,                               ///< Unable to connect
    REALM_AUTH_UNKNOWN5=0x0e,                               ///< Connected.
    REALM_AUTH_PARENTAL_CONTROL=0x0f                        ///< Access to this account has been blocked by parental controls. Your settings may be changed in your account preferences at <site>
};

struct SRealmHeader
{
    uint8	cmd;			// OP code = CMD_REALM_LIST
    uint16	size;			// size of the rest of packet, without this part
    uint32	unknown;		// 0x00 00 00 00
    uint8	count;			// quantity of realms
};

struct AuthHandler
{
    uint32 cmd;
    void (RealmSession::*handler)(ByteBuffer&);
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct sAuthLogonChallenge_S
{
    uint8   cmd;
    uint8   unk2;
    uint8   error;
    uint8   B[32];
    uint8   g_len;
    uint8   g[1];
    uint8   N_len;
    uint8   N[32];
    uint8   salt[32];
    uint8   unk3[16];
};

struct sAuthLogonProof_S
{
    uint8   cmd;
    uint8   error;
    uint8   M2[20];
    uint32  unk1;
    uint32  unk2;
    uint16  unk3;
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

RealmSession::RealmSession(PseuInstance* instance)
{
    _instance = instance;
    _socket = NULL;
    _mustdie = false;
    _filetransfer = false;
    _file_size = 0;
    _sh.SetAutoCloseSockets(false);
}

RealmSession::~RealmSession()
{
    // drop the socket
    ClearSocket();

    // clear the queue
    ByteBuffer *packet;
    while(!pktQueue.empty())
    {
        packet = pktQueue.next();
        delete packet;
    }
    memset(_m2,0,20);
    _key=0;
}

void RealmSession::Connect(void)
{
    ClearSocket();
    _socket = new RealmSocket(_sh);
    _socket->SetSession(this);
    _socket->Open(GetInstance()->GetConf()->realmlist,GetInstance()->GetConf()->realmport);
    _sh.Add(_socket);
    _sh.Select(3,0);
}

void RealmSession::ClearSocket(void)
{
    if(_socket)
    {
        delete _socket;
        _socket = NULL;
    }
}

void RealmSession::SetMustDie(void)
{
    _mustdie = true;
    logdebug("RealmSession: Must die now.");
}

bool RealmSession::MustDie(void)
{
    return _mustdie;
}

AuthHandler *RealmSession::_GetAuthHandlerTable(void) const
{
    static AuthHandler table[] =
    {
        {AUTH_LOGON_CHALLENGE,&RealmSession::_HandleLogonChallenge},
        {AUTH_LOGON_PROOF,&RealmSession::_HandleLogonProof},
        {REALM_LIST,&RealmSession::_HandleRealmList},
        {XFER_INITIATE,&RealmSession::_HandleTransferInit},
        {XFER_DATA,&RealmSession::_HandleTransferData},
        {0,NULL}
    };
    return table;
}

void RealmSession::AddToPktQueue(ByteBuffer *pkt)
{
    pktQueue.add(pkt);
}

void RealmSession::Update(void)
{
    AuthHandler *table = _GetAuthHandlerTable();
    ByteBuffer *pkt;
    uint8 cmd;
    bool valid = true;

    if( _sh.GetCount() ) // the socket will remove itself from the handler if it got closed
        _sh.Select(0,0);
    else // so we just need to check if the socket doesnt exist or if it exists but isnt valid anymore.
    {    // if thats the case, we dont need the session anymore either
        if(!_socket || (_socket && !_socket->IsOk()))
        {
            SetMustDie();
        }
    }

    while(pktQueue.size())
    {
        valid = false;
        pkt = pktQueue.next();
        cmd = (*pkt)[0];

        // this is a dirty hack for oversize/splitted up packets that are buffered wrongly by realmd
        if(_filetransfer)
        {
            _HandleTransferData(*pkt);
        }
        // if we dont expect a file transfer select packets as usual
        else
        {
            for(uint8 i=0;table[i].handler!=NULL;i++)
            {
                if(table[i].cmd==cmd)
                {
                    valid = true;
                    (this->*table[i].handler)(*pkt);
                    if(pkt->rpos() < pkt->size())
                    {
                        uint32 len = pkt->size() - pkt->rpos();
                        uint8 *data = new uint8[len];
                        pkt->read(data,len); // if we have data crap left on the buf, delete it
                        logdebug("Data left on RealmSocket, Hexdump:");
                        logdebug(toHexDump(data,len).c_str());
                        delete [] data;
                    }
                    break;
                }
            }
            if(!valid)
            {
                logerror("Invalid realm packet, unknown opcode 0x%X",cmd);
                //logerror(toHexDump((uint8*)pkt->contents(),pkt->size()).c_str());
            }
        }
        delete pkt;
    }
}

PseuInstance *RealmSession::GetInstance(void)
{
    return _instance;
}

void RealmSession::_HandleRealmList(ByteBuffer& pkt)
{
    std::string realmAddr;

    uint32 unk;
    uint16 len,count;
    uint8 cmd;
    pkt >> cmd >> len >> unk >> count;

    // no realm?
    if(count==0)
        return;

    _realms.clear();
    _realms.resize(count);

    // readout realms
    for(uint8 i=0;i<count;i++)
    {
        pkt >> _realms[i].icon;
        pkt >> _realms[i].locked;
        pkt >> _realms[i].color;
        pkt >> _realms[i].name;
        pkt >> _realms[i].addr_port;
        pkt >> _realms[i].population;
        pkt >> _realms[i].chars_here;
        pkt >> _realms[i].timezone;
        pkt >> _realms[i].unknown;
    }

    // the rest of the packet is not interesting

    for(uint8 i = 0; i < count; i++)
    {
        if(!stricmp(_realms[i].name.c_str(), GetInstance()->GetConf()->realmname.c_str()))
        {
            realmAddr = _realms[i].addr_port;
        }
        logcustom(0,LGREEN,"Realm: %s (%s)",_realms[i].name.c_str(),_realms[i].addr_port.c_str());
        logdetail(" [chars:%d][population:%f][timezone:%d]",_realms[i].chars_here,_realms[i].population,_realms[i].timezone);
    }

    // now setup where the worldserver is and how to login there
    if(realmAddr.empty())
    {
        if(PseuGUI *gui = GetInstance()->GetGUI())
        {
            logdebug("RealmSession: GUI exists, switching to realm selection screen");
            gui->SetSceneState(SCENESTATE_REALMSELECT); // realm select is a sub-window of character selection
        }
        else
        {
            logerror("Realm \"%s\" was not found on the realmlist!",GetInstance()->GetConf()->realmname.c_str());
        }
        return;
    }

    // transform "hostname:port" into something useful
    // -> convert the worldserver port from string to int
    // -> write it into the config & set appropriate vars

    SetRealmAddr(realmAddr);

    // now we have the correct addr/port, time to create the WorldSession
    GetInstance()->CreateWorldSession(); // will be done at next PseuInstance::Update()
}

void RealmSession::SetRealmAddr(std::string host)
{
    logdebug("SetRealmAddr [%s]", host.c_str());
    uint16 colonpos=host.find(":");
    ASSERT(colonpos != std::string::npos);
    GetInstance()->GetConf()->worldhost=host.substr(0,colonpos);
    GetInstance()->GetConf()->worldport=atoi(host.substr(colonpos+1,host.length()-colonpos-1).c_str());
    // set vars
    GetInstance()->GetScripts()->variables.Set("WORLDHOST",GetInstance()->GetConf()->worldhost);
    GetInstance()->GetScripts()->variables.Set("WORLDPORT",DefScriptTools::toString((uint64)(GetInstance()->GetConf()->worldport)));
}

void RealmSession::SetLogonData(void)
{
    _accname=GetInstance()->GetConf()->accname;
    _accpass=GetInstance()->GetConf()->accpass;
}

void RealmSession::SendLogonChallenge(void)
{
    if(!_socket)
    {
        logerror("Can't send logon challenge, socket doesn't exist");
        return;
    }
    if( _accname.empty() || GetInstance()->GetConf()->clientversion_string.empty()
        || GetInstance()->GetConf()->clientbuild==0 || GetInstance()->GetConf()->clientlang.empty() )
    {
        logcritical("Missing data, can't send Login challenge to Realm Server! (check your conf files)");
        GetInstance()->SetError();
        return;
    }
    if(PseuGUI *gui = GetInstance()->GetGUI())
        gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS, DSCENE_LOGIN_LOGGING_IN);
    std::string acc = stringToUpper(_accname);
    ByteBuffer packet;
    packet << (uint8)AUTH_LOGON_CHALLENGE;
    packet << (uint8)6;
    packet << (uint8)(acc.length()+30); // length of the rest of the packet
    packet << (uint8)0;
    packet << "WoW";
    packet.append(GetInstance()->GetConf()->clientversion,3); // 1.12.2 etc
    packet << (uint16)(GetInstance()->GetConf()->clientbuild); // (uint16) 5875
    packet << "68x" << "niW"; // "x86" - platform; "Win" - Operating system; both reversed and zero terminated
    for(uint8 i=0;i<4;i++)
        packet << (uint8)(GetInstance()->GetConf()->clientlang[3-i]); // "enUS" -> "SUne" : reversed and NOT zero terminated
    packet << (uint32)0x3c; // timezone
    packet << (uint32)_socket->GetMyIP(); // my IP address
    packet << (uint8)acc.length(); // length of acc name without \0
    packet.append(acc.c_str(),acc.length()); // append accname, skip \0

    SendRealmPacket(packet);
    logdebug("Packet Sent");
}

void RealmSession::_HandleLogonChallenge(ByteBuffer& pkt)
{
    PseuGUI *gui = GetInstance()->GetGUI();
    logdebug("RealmSocket: Got AUTH_LOGON_CHALLENGE [%u of %u bytes]",pkt.size(),sizeof(sAuthLogonChallenge_S));
    if(pkt.size() < 3)
    {
        logerror("AUTH_LOGON_CHALLENGE: Recieved incorrect/unknown packet. Hexdump:");
        DumpInvalidPacket(pkt);
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_UNK_ERROR);
        return;
    }

    sAuthLogonChallenge_S lc;
    lc.error = pkt[2]; // pre-set error (before copying whole challenge)

    switch (lc.error)
    {
    case 4:
        logerror("Realm Server did not find account \"%s\"!",_accname.c_str());
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_ACC_NOT_FOUND);
        break;
    case 6:
        logerror("Account \"%s\" is already logged in!",_accname.c_str());
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_ALREADY_CONNECTED);
        break;
    case 9:
        logerror("Realm Server doesn't accept this version!");
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_WRONG_VERSION);
        break;
    case 0:
        {
            pkt.read((uint8*)&lc, sizeof(sAuthLogonChallenge_S));
            logdetail("Login successful, now calculating proof packet...");
            if(PseuGUI *gui = GetInstance()->GetGUI())
                gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS, DSCENE_LOGIN_AUTHENTICATING);

            // now lets start calculating
            BigNumber N,A,B,a,u,x,v,S,salt,unk1,g,k(3); // init BNs, default k to 3
            std::string user=stringToUpper( _accname );
            std::string _authstr=stringToUpper( user +":"+_accpass );

            B.SetBinary(lc.B,32);
            g.SetBinary(lc.g,lc.g_len);
            N.SetBinary(lc.N,lc.N_len);
            salt.SetBinary(lc.salt,32);
            unk1.SetBinary(lc.unk3,16);

            logdebug("== Server Bignums ==");
            logdebug("--> B=%s",B.AsHexStr());
            logdebug("--> g=%s",g.AsHexStr());
            logdebug("--> N=%s",N.AsHexStr());
            logdebug("--> salt=%s",salt.AsHexStr());
            logdebug("--> unk=%s",unk1.AsHexStr());

            logdebug("== My Bignums ==");
            a.SetRand(19*8);
            ASSERT(a.AsDword() > 0);
            logdebug("--> a=%s",a.AsHexStr());
            Sha1Hash userhash,xhash,uhash;
            userhash.UpdateData(_authstr);
            userhash.Finalize();
            xhash.UpdateData(salt.AsByteArray(),salt.GetNumBytes());
            xhash.UpdateData(userhash.GetDigest(),userhash.GetLength());
            xhash.Finalize();
            x.SetBinary(xhash.GetDigest(),xhash.GetLength());
            logdebug("--> x=%s",x.AsHexStr());
            v=g.ModExp(x,N);
            logdebug("--> v=%s",v.AsHexStr());
            A=g.ModExp(a,N);
            logdebug("--> A=%s",A.AsHexStr());
            uhash.UpdateBigNumbers(&A, &B, NULL);
            uhash.Finalize();
            u.SetBinary(uhash.GetDigest(), 20);
            logdebug("--> u=%s",u.AsHexStr());
            S=(B - k*g.ModExp(x,N) ).ModExp((a + u * x),N);
            logdebug("--> S=%s",S.AsHexStr());
            ASSERT(S.AsDword() > 0);


            // calc M1 & M2
            unsigned int i=0;
            char S1[16+1],S2[16+1]; // 32/2=16 :) +1 for \0
            // split it into 2 seperate strings, interleaved
            for(i=0;i<16;i++){
                S1[i]=S.AsByteArray()[i*2];
                S2[i]=S.AsByteArray()[i*2+1];
            }

            // hash each one:
            Sha1Hash S1hash,S2hash;
            S1hash.UpdateData((const uint8*)S1,16);
            S1hash.Finalize();
            S2hash.UpdateData((const uint8*)S2,16);
            S2hash.Finalize();
            // Re-combine them
            char S_hash[40];
            for(i=0;i<20;i++){
                S_hash[i*2]=S1hash.GetDigest()[i];
                S_hash[i*2+1]=S2hash.GetDigest()[i];
            }
            _key.SetBinary((uint8*)S_hash,40); // used later when authing to world
            logdebug("--> SessionKey=%s",_key.AsHexStr());

            char Ng_hash[20];
            Sha1Hash userhash2,Nhash,ghash;
            userhash2.UpdateData((const uint8*)user.c_str(),user.length());
            userhash2.Finalize();
            //printchex((char*)userhash2.GetDigest(),userhash2.GetLength(),true);
            Nhash.UpdateBigNumbers(&N,NULL);
            Nhash.Finalize();
            ghash.UpdateBigNumbers(&g,NULL);
            ghash.Finalize();
            for(i=0;i<20;i++)Ng_hash[i] = Nhash.GetDigest()[i]^ghash.GetDigest()[i];
            //printchex(Ng_hash,20,true);

            BigNumber t_acc,t_Ng_hash;
            t_acc.SetBinary((const uint8*)userhash2.GetDigest(),userhash2.GetLength());
            t_Ng_hash.SetBinary((const uint8*)Ng_hash,20);


            Sha1Hash M1hash,M2hash;

            M1hash.UpdateBigNumbers(&t_Ng_hash,&t_acc,&salt,&A,&B,NULL);
            M1hash.UpdateData((const uint8*)S_hash,40);
            M1hash.Finalize();

            M2hash.UpdateBigNumbers(&A,NULL);
            M2hash.UpdateData((const uint8*)M1hash.GetDigest(),M1hash.GetLength());
            M2hash.UpdateData((const uint8*)S_hash,40);
            M2hash.Finalize();

            logdebug("== Common Hashes ==");
            logdebug("--> M1=%s",toHexDump(M1hash.GetDigest(),M1hash.GetLength(),false).c_str());
            logdebug("--> M2=%s",toHexDump(M2hash.GetDigest(),M2hash.GetLength(),false).c_str());

            // Calc CRC & CRC_hash
            // i don't know yet how to calc it, so set it to zero
            char crc_hash[20];
            memset(crc_hash,0,20);

            logdebug("--> CRC=%s",toHexDump((uint8*)crc_hash,20,false).c_str());


            // now lets prepare the packet
            ByteBuffer packet;
            packet << (uint8)AUTH_LOGON_PROOF;
            packet.append(A.AsByteArray(),A.GetNumBytes());
            packet.append(M1hash.GetDigest(),M1hash.GetLength());
            packet.append(crc_hash,20);
            packet << (uint8)0; // number of keys = 0

            if(GetInstance()->GetConf()->clientbuild > 5302)
                packet << (uint8)0; // 1.11.x compatibility (needs one more 0)

            GetInstance()->SetSessionKey(_key);
            memcpy(this->_m2,M2hash.GetDigest(),M2hash.GetLength()); // save M2 to an extern var to check it later

            SendRealmPacket(packet);
        }
        break;

    default:
        logerror("Unknown realm server response! opcode=0x%x\n",(unsigned char)lc.error);
        DumpInvalidPacket(pkt);
        break;
    }
}


void RealmSession::_HandleLogonProof(ByteBuffer& pkt)
{
    PseuGUI *gui = GetInstance()->GetGUI();
    logdebug("RealmSocket: Got AUTH_LOGON_PROOF [%u of %u bytes]",pkt.size(),sizeof(sAuthLogonProof_S));
    if(pkt.size() < 2)
    {
        logerror("AUTH_LOGON_PROOF: Recieved incorrect/unknown packet. Hexdump:");
        DumpInvalidPacket(pkt);
        DieOrReconnect(true);
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_UNK_ERROR);
        return;
    }
    uint8 error = pkt[1];

    // handle error codes
    switch(error)
    {
        case REALM_AUTH_UPDATE_CLIENT:
            log("The realm server requested client update.");
            if(gui)
                gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_WRONG_VERSION);
            DieOrReconnect(true);
            return;

        case REALM_AUTH_NO_MATCH:
        case REALM_AUTH_UNKNOWN2:
            if(gui)
                gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS, DSCENE_LOGIN_AUTH_FAILED);
            logerror("Wrong password or invalid account information or authentication error");
            DieOrReconnect(false);
            return;

        // cover all other cases. continue only if success.
        default:
            if(error != REALM_AUTH_SUCCESS)
            {
                logerror("AUTH_LOGON_PROOF: unk error = 0x%X",error);
                if(gui)
                    gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_UNK_ERROR);
                pkt.rpos(2);
                DieOrReconnect(true);
                return;
            }
    }


    sAuthLogonProof_S lp;
    pkt.read((uint8*)&lp, sizeof(sAuthLogonProof_S));
    //printchex((char*)&lp, sizeof(sAuthLogonProof_S),true);
    if(!memcmp(lp.M2,this->_m2,20))
    {
        if(PseuGUI *gui = GetInstance()->GetGUI())
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS, DSCENE_LOGIN_REQ_REALM);
        // auth successful
        ByteBuffer packet;
        packet << (uint8)REALM_LIST;
        packet << (uint32)0;
        SendRealmPacket(packet);
    }
    else
    {
        logcritical("Auth failed, M2 differs!");
        printf("My M2 :"); printchex((char*)_m2,20,true);
        printf("Srv M2:"); printchex((char*)lp.M2,20,true);
        if(gui)
            gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_AUTH_FAILED);
        DieOrReconnect(true);
    }
}

void RealmSession::_HandleTransferInit(ByteBuffer& pkt)
{
    _filebuf.clear();
    _transbuf.clear();
    _file_done = 0;
    _filetransfer = true;

    uint8 cmd;
    uint8 type_size;
    uint8 *type_str;

    pkt >> cmd >> type_size;
    type_str = new uint8[type_size+1];
    type_str[type_size] = 0;
    pkt.read(type_str,type_size);
    pkt >> _file_size;
    pkt.read(_file_md5,MD5_DIGEST_LENGTH);
    logcustom(0,GREEN,"TransferInit [%s]: File size: "I64FMTD" KB (MD5: %s)", (char*)type_str, _file_size / 1024L, toHexDump(&_file_md5[0],MD5_DIGEST_LENGTH,false).c_str());
    if(PseuGUI *gui = GetInstance()->GetGUI())
        gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS,DSCENE_LOGIN_FILE_TRANSFER);
    delete [] type_str;
    ByteBuffer bb(1);
    bb << uint8(XFER_ACCEPT);
    SendRealmPacket(bb);
    logdebug("XFER_ACCEPT sent");
}

void RealmSession::_HandleTransferData(ByteBuffer& pkt)
{
    if(!_file_size)
    {
        logerror("Realm server attempted to transfer a file, but didn't init!");
        DieOrReconnect(false);
        return;
    }

    uint8 cmd;
    uint16 size;
    uint8 *data;

    _transbuf.append(pkt.contents(),pkt.size()); // append everything to the transfer buffer, which may also store incomplete bytes from the packet before
    pkt.rpos(pkt.size()); // set rpos to the end of the packet to indicate that we used all data

    logdev("transbuf size=%u rpos=%u diff=%u",_transbuf.size(),_transbuf.rpos(),_transbuf.size() - _transbuf.rpos());

    while( _transbuf.size() - _transbuf.rpos() >= 3) // 3 = sizeof(uint32)+sizeof(uint8)
    {
        _transbuf >> cmd >> size;
        if(_transbuf.size()-_transbuf.rpos() < size)
        {
            _transbuf.rpos(_transbuf.rpos()-3); // read the header next time again
            break; // packet parts missing, continue after recieving next packet
        }
        data = new uint8[size];
        _transbuf.read(data,size);
        _filebuf.append(data,size);
        _file_done += size;
        delete [] data;
        float pct = ((float)_file_done / (float)_file_size * 100.0f);

        // use better output formatting in debug level
        if(GetInstance()->GetConf()->debug >= 2)
            logdebug("Got data packet, %u data bytes. [%.2f%% done]  cmd 0x%X",size,pct,cmd);
        else
        {
            _log_setcolor(true,GREEN);
            printf("\r[%.2f%% done]",pct);
            _log_resetcolor(true);
        }

    }

    // finalize file
    if(_file_done >= _file_size)
    {
        log("");
        log("File transfer finished.");
        _filetransfer = false;
        MD5Hash md5h;
        md5h.Update((uint8*)_filebuf.contents(),_filebuf.size());
        md5h.Finalize();
        std::string md5hex = toHexDump(md5h.GetDigest(),md5h.GetLength(),false);
        logdebug("MD5 hash: %s", md5hex.c_str());
        if(!memcmp(_file_md5, md5h.GetDigest(), md5h.GetLength()))
        {
            std::fstream fh;
            char namebuf[100];
            sprintf(namebuf,"%u_%s.mpq",GetInstance()->GetConf()->clientbuild,GetInstance()->GetConf()->clientlang.c_str());
            fh.open(namebuf,std::ios_base::out | std::ios_base::binary);
            if(fh.is_open())
            {
                fh.write((const char*)_filebuf.contents(),_filebuf.size());
                fh.close();
                log("File saved as \"%s\"",namebuf);
            }
            else
            {
                logerror("Could not save \"%s\"",namebuf);
            }
        }
        else
        {
            logerror("File corruption! Transfer failed! (MD5: %s",md5hex.c_str());
        }
        _transbuf.clear();

        // client sends cancel after successful file transfer also
        ByteBuffer bb(1);
        bb << uint8(XFER_CANCEL);
        SendRealmPacket(bb);

        log("Now modify your conf files and restart PseuWoW.");
        for(int8 x = 3; x > -1; x--) // add little delay
        {
            printf("exiting in... [%u]\r",x);
            GetInstance()->Sleep(1000);
        }
        SetMustDie();
        GetInstance()->Stop();
    }
}

void RealmSession::DumpInvalidPacket(ByteBuffer& pkt)
{
    if(pkt.size())
        logerror( toHexDump((uint8*)pkt.contents(),pkt.size()).c_str() );
}

void RealmSession::SendRealmPacket(ByteBuffer& pkt)
{
    if(_socket && _socket->IsOk())
    {
        if(pkt.size()) // dont send packets with no data
            _socket->SendBuf((const char*)pkt.contents(),pkt.size());
    }
    else
    {
        logerror("Can't send realm packet, socket does not exist or is not ready!");
    }
}

// err=true will close PseuWoW if ExitOnError=1
void RealmSession::DieOrReconnect(bool err)
{
    if(GetInstance()->GetConf()->reconnect)
        SetMustDie();
    else if(err)
    {
        SetMustDie();
        GetInstance()->SetError();
    }
}

bool RealmSession::SocketGood(void)
{
    return _socket && _socket->IsOk();
}
