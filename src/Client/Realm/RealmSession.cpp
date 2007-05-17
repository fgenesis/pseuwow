#include "common.h"
#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "PseuWoW.h"
#include "RealmSocket.h"
#include "RealmSession.h"

enum AuthCmd
{
    AUTH_LOGON_CHALLENGE        = 0x00,
    AUTH_LOGON_PROOF            = 0x01,
    REALM_LIST                  = 0x10,
};

struct SRealmHeader
{
    uint8	cmd;			// OP code = CMD_REALM_LIST
    uint16	size;			// size of the rest of packet, without this part
    uint32	unknown;		// 0x00 00 00 00
    uint8	count;			// quantity of realms
};

struct SRealmInfo
{
    uint8	icon;			// icon near realm
    uint8   locked;         // added in 2.0.x
    uint8	color;			// color of record
    std::string	name;			// Text zero terminated name of Realm
    std::string	addr_port;		// Text zero terminated address of Realm ("ip:port")
    float	population;		// 1.6 -> population value. lower == lower population and vice versa
    uint8	chars_here;		// number of characters on this server
    uint8	timezone;		// timezone
    uint8	unknown;		//
};

struct AuthHandler
{
    uint32 cmd;
    void (RealmSession::*handler)(ByteBuffer&);
};

struct sAuthLogonChallenge_S
{
    uint8   cmd;
    uint8   error;
    uint8   unk2;
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
    uint32  unk2;
};

RealmSession::RealmSession(PseuInstance* instance)
{
    _instance = instance;
    _socket = NULL;
    _mustdie = false;
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
        pkt = pktQueue.next();
        cmd = (*pkt)[0];
        for(uint8 i=0;table[i].handler!=NULL;i++)
        {
            if(table[i].cmd==cmd)
            {
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
                delete pkt;
            }
        }

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

    // alloc space for as many realms as needed
    SRealmInfo *realms=new SRealmInfo[count];

    // readout realms
    for(uint8 i=0;i<count;i++)
    {
        pkt >> realms[i].icon;
        pkt >> realms[i].locked;
        pkt >> realms[i].color;
        pkt >> realms[i].name;
        pkt >> realms[i].addr_port;
        pkt >> realms[i].population;
        pkt >> realms[i].chars_here;
        pkt >> realms[i].timezone;
        pkt >> realms[i].unknown;
    }

    // the rest of the packet is not interesting

    for(uint8 i=0;i<count;i++)
    {
        if(realms[i].name==GetInstance()->GetConf()->realmname)
        {
            realmAddr=realms[i].addr_port;
        }
        logcustom(0,LGREEN,"Realm: %s (%s)",realms[i].name.c_str(),realms[i].addr_port.c_str());
        logdetail(" [chars:%d][population:%f][timezone:%d]",realms[i].chars_here,realms[i].population,realms[i].timezone);
    }
    delete [] realms;

    // now setup where the worldserver is and how to login there
    if(realmAddr.empty()){
        log("Realm \"%s\" was not found on the realmlist!",GetInstance()->GetConf()->realmname.c_str());
        return;
    }

    // transform "hostname:port" into something useful
    // -> convert the worldserver port from string to int
    // -> write it into the config & set appropriate vars

    uint16 colonpos=realmAddr.find(":");
    GetInstance()->GetConf()->worldhost=realmAddr.substr(0,colonpos);
    GetInstance()->GetConf()->worldport=atoi(realmAddr.substr(colonpos+1,realmAddr.length()-colonpos-1).c_str());
    // set vars
    GetInstance()->GetScripts()->variables.Set("WORLDHOST",GetInstance()->GetConf()->worldhost);
    GetInstance()->GetScripts()->variables.Set("WORLDPORT",toString((uint64)(GetInstance()->GetConf()->worldport)));

    // now we have the correct addr/port, time to create the WorldSession
    GetInstance()->CreateWorldSession(); // will be done at next PseuInstance::Update()
}

void RealmSession::SendLogonChallenge(void)
{
    if( GetInstance()->GetConf()->accname.empty() || GetInstance()->GetConf()->clientversion_string.empty()
        || GetInstance()->GetConf()->clientbuild==0 || GetInstance()->GetConf()->clientlang.empty() )
    {
        logcritical("Missing data, can't send Login to Realm Server!");
        GetInstance()->SetError();
        return;
    }
    std::string acc = stringToUpper(GetInstance()->GetConf()->accname);
    ByteBuffer packet;
    packet << (uint8)AUTH_LOGON_CHALLENGE;
    packet << (uint8)2;
    packet << (uint8)(acc.length()+30); // length of the rest of the packet
    packet << (uint8)0;
    packet.append("WOW",3);
    packet.append(GetInstance()->GetConf()->clientversion,3); // 1.12.2 etc
    packet << (uint8)0;
    packet << (uint16)(GetInstance()->GetConf()->clientbuild); // (uint16) 5875
    packet << "68x" << "niW"; // "x86" - platform; "Win" - Operating system; both reversed and zero terminated
    for(uint8 i=0;i<4;i++)
        packet << (uint8)(GetInstance()->GetConf()->clientlang[3-i]); // "enUS" -> "SUne" : reversed and NOT zero terminated
    packet << (uint32)0x3c; // timezone
    packet << (uint32)_socket->GetClientRemoteAddr(); // my IP address
    packet << (uint8)acc.length(); // length of acc name without \0
    packet.append(acc.c_str(),acc.length()); // append accname, skip \0

    SendRealmPacket(packet);

}

void RealmSession::_HandleLogonChallenge(ByteBuffer& pkt)
{
    logdebug("RealmSocket: Got AUTH_LOGON_CHALLENGE [%u of %u bytes]",pkt.size(),sizeof(sAuthLogonChallenge_S));
    if(pkt.size() < sizeof(sAuthLogonChallenge_S))
    {
        logerror("AUTH_LOGON_CHALLENGE: Recieved incorrect/unknown packet. Hexdump:");
        DumpInvalidPacket(pkt);
        return;
    }

    sAuthLogonChallenge_S lc;
    pkt.read((uint8*)&lc, sizeof(sAuthLogonChallenge_S));

    switch (lc.error)
    {
    case 4:
        log("Realm Server did not find account \"%s\"!",GetInstance()->GetConf()->accname.c_str());
        break;
    case 6:
        log("Account \"%s\" is already logged in!",GetInstance()->GetConf()->accname.c_str());
        // TODO: wait a certain amount of time before reconnecting? conf option?
        break;
    case 9:
        log("Realm Server doesn't accept this version!");
        break;
    case 0:
        {
            logdetail("Login successful, now calculating proof packet...");

            // now lets start calculating
            BigNumber N,A,B,a,u,x,v,S,salt,unk1,g,k(3); // init BNs, default k to 3
            std::string user=stringToUpper( GetInstance()->GetConf()->accname );
            std::string _authstr=stringToUpper( user +":"+GetInstance()->GetConf()->accpass );

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
    logdebug("RealmSocket: Got AUTH_LOGON_PROOF [%u of %u bytes]\n",pkt.size(),26);
    if(pkt.size() < 26)
    {
        logerror("AUTH_LOGON_PROOF: Recieved incorrect/unknown packet. Hexdump:");
        DumpInvalidPacket(pkt);
        if(GetInstance()->GetConf()->reconnect)
            SetMustDie();
        return;
    }
    sAuthLogonProof_S lp;
    pkt.read((uint8*)&lp, 26); // the compiler didnt like 'sizeof(sAuthLogonProof_S)', said it was 28
    //printchex((char*)&lp, sizeof(sAuthLogonProof_S),true);
    if(!memcmp(lp.M2,this->_m2,20))
    {
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

        if(GetInstance()->GetConf()->reconnect)
            SetMustDie();
        else
            GetInstance()->SetError();
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

       



