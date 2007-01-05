#include "common.h"
#include "PseuWoW.h"
#include "Auth/ByteBuffer.h"
#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include "RealmSocket.h"



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

// is here any other socket code neded?
//    _socket.Init()
//    _socket.SetHost(_host);
//    _socket.SetPort(_port);

    bool result=Open(_rhost,_rport);
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
    SRealmHeader hd;
    std::string realmAddr;
    ibuf.Read((char*)&hd, sizeof(SRealmHeader));
	
	
	////DEBUG1(printf("Realms in List: %d\n",count););
    // no realm?!
    if(hd.count==0)
        return;

    // alloc space for as many realms as needed
	SRealmInfo *realms=new SRealmInfo[hd.count];

    // readout realms
	for(uint8 i=0;i<hd.count;i++)
		ibuf.Read((char*)&realms[i], sizeof(SRealmInfo));	
	// the rest of the packet is not interesting

    for(uint8 i=0;i<hd.count;i++)
    {
        if(realms[i].name==GetInstance()->GetConf()->realmname)
        {
            realmAddr=realms[i].addr_port;
        }
		printf("Realm: %s (%s)",realms[i].name.c_str(),realms[i].addr_port.c_str());
        printf("[chars:%d][population:%f][timezone:%d]",realms[i].chars_here,realms[i].population,realms[i].timezone);
        printf("\n");
    }

	// now setup where the woldserver is and how to login there
    if(realmAddr.empty()){
		printf("Realm \"%s\" was not found on the realmlist!\n",GetInstance()->GetConf()->realmname.c_str());
		//something_went_wrong=true;
		//realmCon.Close();
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
}


void RealmSocket::OnRead(void)
{
    TcpSocket::OnRead();
    printf("RealmSocket::OnRead() %u bytes\n",ibuf.GetLength());
    if(!ibuf.GetLength())
        return;
    uint8 cmd, i=0;
    ibuf.SoftRead((char*)&cmd, 1);
    while(table[i].handler!=NULL)
    {
        if(table[i].cmd==cmd)
        {
            (*this.*table[i].handler)();
            break;
        }
    }
    // unk packet
}

/*
	switch(rs_state){
		unsigned int i;
		case 1:{		
			if(pkt[2]==4){
					printf("Realm Server did not find account \"%s\"!\n",accname);
					something_went_wrong=true;
					realmCon.Close();
			} else
			if(pkt[2]==6){
					printf("Account \"%s\" is already logged in!\n",accname);
					realmCon.Close();
					something_went_wrong=true;
			} else
			if(pkt[2]!=0){
					printf("Unknown realm server response! opcode=0x%x\n",(unsigned char)pkt[2]);
					something_went_wrong=true;
					realmCon.Close();
			} else
			if(pkt[2]==0){
					//DEBUG1(printf("Login successful, now calculating proof packet...\n"););
					ProcessClientLogonProof(pkt);
					rs_state=2; // 2=waiting for server proof			
			}
		}break;			

		case 2:{
			if(pkt[1]==4){
			printf("Wrong password!\n");
			something_went_wrong=true;
			realmCon.Close();
			} else
			if(pkt[0]==1 && pkt[1]==0 && memcmp(&pkt[2],Auth_M2,20)!=0){
				printf("Something with Authenticating went wrong, although the password seems correct!\n");
				//DEBUG1(printf("-> expected M2=");printchex(Auth_M2,20,true);)
				//DEBUG1(printf("->      got M2=");printchex(&pkt[2],20,true);)
				something_went_wrong=true;
				realmCon.Close();
			} else 
			if(pkt[0]==1 && pkt[1]==0 && memcmp(&pkt[2],Auth_M2,20)==0){
				printf("Password is correct!! Requesting Realmlist.\n");
				rs_state=3; // requesting realmlist
				// Request Realmlist
				char realmrequest[]={0x10,0,0,0,0}; // 0x10 is opcode, rest is an uint32, nulled 
				realmCon.Send(realmrequest,5);
			}
			else {
				printf("Unknown ErrorID recieved, check the packet hexdump.\n");
				printf("-> IDs=");printchex(pkt,2,true);
				something_went_wrong=true;
				realmCon.Close();
			}		
		}break;

		case 3:{
			if(pkt[0]!=0x10){
				printf("Expected a realmlist packet, got something different. opcode=0x%x\n",(unsigned char)pkt[0]);
				something_went_wrong=true;
				realmCon.Close();
			}
            ByteBuffer bbuf;
            bbuf.append(pkt,size);
			if(HandleRealmList(bbuf)==true){
				printf("Connecting to realm \"%s\" at \"%s\", port %d\n",realmname,worldhost.c_str(),ws_port);
				while(!worldCon.IsConnected()){
					worldCon.ConnectTo((char*)worldhost.c_str(),ws_port); // init world server connection, we have all info we need to enter
				}
				realmCon.Close(); // close the realm server connection, its no longer needed now
			}
		}break;
			// more?
		default:{
			//...
				}
	}
}
*/



void RealmSocket::SendLogonChallenge(void)
{
    if(!this->Ready())
    {
        printf("Error sending AUTH_LOGON_CHALLENGE, port is not ready!\n");
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

    packet.hexlike();

    SendBuf((char*)packet.contents(),packet.size());

}

PseuInstance *RealmSocket::GetInstance(void)
{
    return _instance;
}

void RealmSocket::_HandleLogonChallenge(void)
{
    sAuthLogonChallenge_S lc;
    ibuf.Read((char*)&lc, sizeof(sAuthLogonChallenge_S));

    switch (lc.error)
    {
    case 4:
        printf("Realm Server did not find account \"%s\"!\n",GetInstance()->GetConf()->accname.c_str());
        break;
    case 6:
        printf("Account \"%s\" is already logged in!\n",GetInstance()->GetConf()->accname.c_str());
        break;
    case 0:
        printf("Login successful, now calculating proof packet...\n");

        // now lets start calculating
        BigNumber N,A,B,a,u,x,v,S,salt,unk1,g,k(3); // init BNs, default k to 3
        std::string user=stringToUpper( GetInstance()->GetConf()->accname );
        std::string _authstr=stringToUpper( user +":"+GetInstance()->GetConf()->accpass );
        
        B.SetBinary(lc.B,32);
        g.SetBinary(lc.g,lc.g_len);
        N.SetBinary(lc.N,lc.N_len);
        salt.SetBinary(lc.salt,32);
        unk1.SetBinary(lc.unk3,16);

	    /*
	    // debug output	
	    //DEBUG3(printchex(B_str,BNLEN,true);)
	    //DEBUG3(printchex(g_str,1,true);)
	    //DEBUG3(printchex(N_str,BNLEN,true);)
	    //DEBUG3(printchex(salt_str,BNLEN,true);)
	    //DEBUG3(printchex(unk1_str,16,true);)
        */

	    // client-side BN calculations:
	    ////DEBUG3(printf("--> k=%s\n",k.AsHexStr());)
	    a.SetRand(19*8);
	    Sha1Hash userhash,xhash,uhash;
	    userhash.UpdateData(_authstr);
	    userhash.Finalize();
	    xhash.UpdateData(salt.AsByteArray(),salt.GetNumBytes());
	    xhash.UpdateData(userhash.GetDigest(),userhash.GetLength());
	    xhash.Finalize();
	    x.SetBinary(xhash.GetDigest(),xhash.GetLength());
	    ////DEBUG3(printf("--> x=%s\n",x.AsHexStr());)
	    v=g.ModExp(x,N);
	    ////DEBUG3(printf("--> v=%s\n",v.AsHexStr());)
	    A=g.ModExp(a,N);
	    ////DEBUG3(printf("--> A=%s\n",A.AsHexStr());)
        uhash.UpdateBigNumbers(&A, &B, NULL);
        uhash.Finalize();
        u.SetBinary(uhash.GetDigest(), 20);
	    ////DEBUG3(printf("--> u=%s\n",u.AsHexStr());)
	    S=(B - k*g.ModExp(x,N) ).ModExp((a + u * x),N);
	    ////DEBUG3(printf("--> S=%s\n",S.AsHexStr());)

	    // calc M1 & M2
	    unsigned int i=0;
	    char S1[16+1],S2[16+1]; // 32/2=16 :) +1 for \0
	    // split it into 2 seperate strings, interleaved
	    for(i=0;i<=15;i++){
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


		////DEBUG3(
		//	printf("--> M1=");printchex((char*)M1hash.GetDigest(),20,true);\
		//	printf("--> M2=");printchex((char*)M2hash.GetDigest(),20,true);\
		//)

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
        break;

    //default:
    //    printf("Unknown realm server response! opcode=0x%x\n",(unsigned char)lc.error);
    //    break;
    }
}


void RealmSocket::_HandleLogonProof(void)
{
    sAuthLogonProof_S lp;
    ibuf.Read((char*)&lp, sizeof(sAuthLogonProof_S));
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
        // auth failed, M2 differs
        //...
    }
}

void RealmSocket::OnConnect()
{
    printf("DEBUG: RealmSocket connected!\n");
    SendLogonChallenge();
}

void RealmSocket::OnConnectFailed(void)
{
    printf("RealmSocket::OnConnectFailed()\n");
}
    
