#include "common.h"
#include "PseuWoW.h"
#include "Auth/ByteBuffer.h"
#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "RealmSocket.h"
#include "WorldSession.h"



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
	uint32	icon;			// icon near realm
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
    void (RealmSocket::*handler)(void);
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

const AuthHandler table[]=
{
    {AUTH_LOGON_CHALLENGE,&RealmSocket::_HandleLogonChallenge},
    {AUTH_LOGON_PROOF,&RealmSocket::_HandleLogonProof},
    {REALM_LIST,&RealmSocket::_HandleRealmList},
    {0,NULL}
};

RealmSocket::RealmSocket(SocketHandler& h) : TcpSocket(h)
{
    _instance=NULL;
    _valid=false;
    _rport=3724;
}

RealmSocket::~RealmSocket()
{
    Stop();
}

bool RealmSocket::IsValid(void)
{
    return _valid;
}

void RealmSocket::SetHost(std::string h)
{
    _rhost=h;
}

void RealmSocket::SetPort(uint16 p)
{
    _rport=p;
}

void RealmSocket::SetInstance(PseuInstance *pi)
{
    _instance=pi;
}

void RealmSocket::Start(void)
{
    if(_rhost.empty() || _rport==0 || _instance==NULL)
        return;
    log("Connecting to Realm Server on '%s:%u'",_rhost.c_str(),_rport);
    Open(_rhost,_rport);

    //...
    _valid=true;
}

void RealmSocket::Stop(void)
{
    _valid=false;
    this->Close();
    memset(_m2,0,20);
    _key=0;
}

void RealmSocket::_HandleRealmList(void)
{
    std::string realmAddr;
    ByteBuffer realmbuf;
    realmbuf.reserve(ibuf.GetLength());
    ibuf.Read((char*)realmbuf.contents(), ibuf.GetLength());

    uint32 unk;
    uint16 len;
    uint8 cmd,count;
    realmbuf >> cmd >> len >> unk >> count;
	
    // no realm?
    if(count==0)
        return;

    // alloc space for as many realms as needed
	SRealmInfo *realms=new SRealmInfo[count];

    // readout realms
	for(uint8 i=0;i<count;i++)
    {
        realmbuf >> realms[i].icon;
        realmbuf >> realms[i].color;
        realmbuf >> realms[i].name;
        realmbuf >> realms[i].addr_port;
        realmbuf >> realms[i].population;
        realmbuf >> realms[i].chars_here;
        realmbuf >> realms[i].timezone;
        realmbuf >> realms[i].unknown;
    }
		
	// the rest of the packet is not interesting

    for(uint8 i=0;i<count;i++)
    {
        if(realms[i].name==GetInstance()->GetConf()->realmname)
        {
            realmAddr=realms[i].addr_port;
        }
		log("Realm: %s (%s)",realms[i].name.c_str(),realms[i].addr_port.c_str());
        logdetail(" [chars:%d][population:%f][timezone:%d]",realms[i].chars_here,realms[i].population,realms[i].timezone);
    }

	// now setup where the woldserver is and how to login there
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
    GetInstance()->createWorldSession=true;
}


void RealmSocket::OnRead(void)
{
    TcpSocket::OnRead();
    bool known=false;
    //printf("RealmSocket::OnRead() %u bytes\n",ibuf.GetLength());
    if(!ibuf.GetLength())
        return;
    uint8 cmd, i=0;
    ibuf.SoftRead((char*)&cmd, 1);
    for(uint8 i=0;table[i].handler!=NULL;i++)
    {
        if(table[i].cmd==cmd)
        {
            (*this.*table[i].handler)();
            known=true;
            break;
        }
    }
    if(!known)
    {
        log("RealmSocket: Got unknown packet, cmd=%u",cmd);
    }
    ibuf.Remove(ibuf.GetLength()); // if we have data crap left on the buf, delete it
}


void RealmSocket::SendLogonChallenge(void)
{
    if(!this->Ready())
    {
        logerror("Error sending AUTH_LOGON_CHALLENGE, port is not ready!\n");
        return;
    }
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
    packet.append("WOW",3); // game name = World Of Warcraft
    packet.append(GetInstance()->GetConf()->clientversion,3); // 1.12.2 etc
    packet << (uint8)0;
    packet << (uint16)(GetInstance()->GetConf()->clientbuild); // (uint16) 5875
    packet << "68x" << "niW"; // "x86" - platform; "Win" - Operating system; both reversed and zero terminated
    for(uint8 i=0;i<4;i++)
        packet << (uint8)(GetInstance()->GetConf()->clientlang[3-i]); // "enUS" -> "SUne" : reversed and NOT zero terminated
    packet << (uint32)0x3c; // timezone
    packet << (uint32)GetClientRemoteAddr(); // my IP address
    packet << (uint8)acc.length(); // length of acc name without \0
    packet.append(acc.c_str(),acc.length()); // append accname, skip \0

    SendBuf((char*)packet.contents(),packet.size());

}

PseuInstance *RealmSocket::GetInstance(void)
{
    return _instance;
}

void RealmSocket::_HandleLogonChallenge(void)
{
    logdebug("RealmSocket: Got AUTH_LOGON_CHALLENGE [%u of %u bytes]",ibuf.GetLength(),sizeof(sAuthLogonChallenge_S));
    sAuthLogonChallenge_S lc;
    ibuf.Read((char*)&lc, sizeof(sAuthLogonChallenge_S));

    switch (lc.error)
    {
    case 4:
        log("Realm Server did not find account \"%s\"!",GetInstance()->GetConf()->accname.c_str());
        break;
    case 6:
        log("Account \"%s\" is already logged in!",GetInstance()->GetConf()->accname.c_str());
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

	    a.SetRand(19*8);
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
	    char S_hash[40]; // 2*Sha1Len+1 for \0
	    for(i=0;i<20;i++){
		    S_hash[i*2]=S1hash.GetDigest()[i];
		    S_hash[i*2+1]=S2hash.GetDigest()[i];
		    }
        _key.SetBinary((uint8*)S_hash,40); // used later when authing to world
					
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


	    //logdebug("--> M1=");printchex((char*)M1hash.GetDigest(),20,true);
		//logdebug("--> M2=");printchex((char*)M2hash.GetDigest(),20,true);

		// Calc CRC & CRC_hash
		// i don't know yet how to calc it, so set it to zero
		char crc_hash[20];
		memset(crc_hash,0,20);


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

        SendBuf((char*)packet.contents(),packet.size());
        }
        break;

    default:
        log("Unknown realm server response! opcode=0x%x\n",(unsigned char)lc.error);
        break;
    }
}


void RealmSocket::_HandleLogonProof(void)
{
    logdetail("RealmSocket: Got AUTH_LOGON_PROOF [%u of %u bytes]\n",ibuf.GetLength(),26);
    sAuthLogonProof_S lp;
    ibuf.Read((char*)&lp, 26); // the compiler didnt like 'sizeof(sAuthLogonProof_S)', said it was 28
    //printchex((char*)&lp, sizeof(sAuthLogonProof_S),true);
    if(!memcmp(lp.M2,this->_m2,20))
    {
        // auth successful
        ByteBuffer packet;
        packet << (uint8)REALM_LIST;
        packet << (uint32)0;
        SendBuf((char*)packet.contents(),packet.size());
    }
    else
    {
        logcritical("Auth failed, M2 differs!");
        printf("My M2 :"); printchex((char*)_m2,20,true);
        printf("Srv M2:"); printchex((char*)lp.M2,20,true);
        GetInstance()->SetError();
    }
}

void RealmSocket::OnConnect()
{
    logdetail("RealmSocket connected!");
    SendLogonChallenge();
}

void RealmSocket::OnConnectFailed(void)
{
    log("Connecting to Realm failed!");
}
    
