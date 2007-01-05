#include "common.h"
#include "PseuWoW.h"
#include <time.h>
#include <openssl/rand.h>

#include "Auth/ByteBuffer.h"
#include "DefScript/DefScript.h"
#include "DefScriptInterface.h"
#include "Auth/BigNumber.h"
#include "Auth/ByteBuffer.h"
#include "DefScript/DefScript.h"
#include "Realm/RealmSocket.h"
#include "World/WorldSession.h"

/* TODO: 
defscriptpackage fix, xchgresult einbauen, CmdSet.caller einbauen, permissionmap einbauen
WorldSesssion::Update() rein
opcode handler table vervollstädigen + funktionen dafür umschreiben

*/

//###### Start of program code #######

void PseuInstanceRunnable::run(void)
{
    PseuInstance *_i = new PseuInstance();
	_i->SetConfDir("./conf/");
    _i->SetScpDir("./scripts/");
	_i->Init();
	// more
	_i->Run();
    delete _i;
}

PseuInstance::PseuInstance()
{
    _ver="PseuWoW Alpha Build 12 dev 2" DEBUG_APPENDIX;
    _ver_short="A12-dev1" DEBUG_APPENDIX;
    _wsession=NULL;
    _rsession=NULL;
    _stop=false;


}

PseuInstance::~PseuInstance()
{
}

bool PseuInstance::Init(void) {

    if(_confdir.empty())
        _confdir="./conf/";
    if(_scpdir.empty())
        _scpdir="./scp/";

	srand((unsigned)time(NULL));
	RAND_set_rand_method(RAND_SSLeay()); // init openssl randomizer

	if(SDL_Init(0)==-1) {
			printf("SDL_Init: %s\n", SDL_GetError());
			return false;
		}
	
	_scp=new DefScriptPackage();
    _scp->SetParentMethod((void*)this);
	_conf=new PseuInstanceConf();	
	_rsession=new RealmSocket(_sh);
	_wsession=new WorldSession(this);
	
	if(!_scp->variables.ReadVarsFromFile(_confdir + "PseuWoW.conf"))
	{
		printf("Error reading conf file [%s]",_confdir.append("PseuWoW.conf").c_str()); 
		return false;
	}
	_conf->ApplyFromVarSet(_scp->variables);
	
    if(_scp->variables.ReadVarsFromFile(_confdir + "users.conf"))
	{
		printf("-> Done reading users.\n");
	}
	else
	{
		printf("Error reading conf file [%s] - NO PERMISSIONS SET!\n",_confdir.append("users.conf").c_str()); 
	}


//    //DEBUG1(printf("Main_Init: Setting up DefScripts path '%s'\n",defScpPath.c_str()););
    _scp->SetPath(_scpdir);

  //  //DEBUG1(printf("Main_Init: Setting up custom DefScript function interface...\n"););
    //_scp->SetFunctionTable(_GetSCTable());

  //  //DEBUG1(printf("Main_Init: Setting up predefined DefScript macros...\n"););
    _scp->variables.Set("@version_short",_ver_short);
    _scp->variables.Set("@version",_ver);


  //  //DEBUG1(printf("Main_Init: Loading DefScripts from folder '%s'\n",defScpPath.c_str()););
    //if(!_scp->RunScriptByName("_startup",NULL,255)){
    //    printf("Main_Init: Error executing '_startup.def'\n");
    //}

    if(_stop){
			printf("Errors while initializing, proc halted!!\n");
            if(GetConf()->exitonerror)
                exit(0);
			while(true)SDL_Delay(1000);
    }

//	if(DEBUG)printf("Main_Init: Init complete.\n");
	_initialized=true; 
	return true;
}

void PseuInstance::Run(void)
{
    if(!_initialized)
        return;


    _rsession->SetHost(GetConf()->realmlist);
    _rsession->SetPort(GetConf()->realmport);
    _rsession->SetInstance(this);
    _rsession->Start();
    
    if(_rsession->IsValid())  // TODO: need better check for IsValid(), it must check also if it could connect
    {
        _sh.Add(_rsession);
        _rsession->SendLogonChallenge();
        _sh.Select(1,0);
    }

    while(!_stop)
    {
        Update();
    }
    // save all data here

}

void PseuInstance::Update()
{
    if(_sh.GetCount())
        _sh.Select(1,0); // update the realmsocket
    //else
        // socket invalid?

    _wsession->Update();

}

void PseuInstance::SaveAllCache(void)
{
    GetWSession()->plrNameCache.SaveToFile();
    //...
}



    /*
    printf("%s\n",ver);
    for(unsigned int temp=0;temp<=strlen(ver);temp++)printf("~");printf("\n");
	printf("[Compiled on: %s , %s]\n\n",__DATE__,__TIME__);
    if (!initproc()) quitproc_error();

	bool _auth_sent=false;
	bool _first_wpacket=true;
    clock_t ping_time=clock()-25000;

	while (true) {

		if(something_went_wrong){
			if(inworld){
				SendChatMessage(CHAT_MSG_YELL,0,"ERROR! Something went wrong, exiting...","");
			}
			printf("!!! Something went wrong, proc halted.!!!\n");
			ctrlCon.Close();
			worldCon.Close();
			realmCon.Close();
			if(exitonerror)quitproc();
			while(true)SDL_Delay(1000);
		}

		if( !realmCon.IsConnected() && !worldCon.IsConnected() && !something_went_wrong){
			printf("Opening realm TCP connection <%s:%d>\n",realmlist,rs_port);
			while(!realmCon.ConnectTo(realmlist,rs_port));
			_auth_sent=false;
		}

		if(realmCon.IsConnected() && !worldCon.IsConnected() && !something_went_wrong){
			if(!_auth_sent){
				CLIENT_LOGON_CHALLENGE(accname, accpass);
				_auth_sent=true;
			}
		}
		if(realmCon.HasData()){
            Buf buf;
            buf=realmCon.GetData();
            rs_parser(buf.str,buf.len); // TODO: change type to Buf (also the func arg!)
		}

		
		
		while(worldCon.GetBufferLevel()>1){
			////DEBUG3(
			//	printf("WS_IN, %d bytes. Buffer Level=%u\n",worldCon.GetKeepData().len,worldCon.GetBufferLevel());
			//	printchex(worldCon.GetKeepData().str,worldCon.GetKeepData().len,true);
			//)
			//_first_wpacket=false;
			ByteBuffer pk2;
			pk2.append(worldCon.GetDataString());
			pk2.resize(pk2.size()-1);
			HandleWorldPacket(pk2);
		}

		// this covers the first serverseed packet, which get sometimes merged into 1 packet instead of 2
		if(worldCon.HasData() && _first_wpacket && worldCon.GetKeepData().len==8){
			_first_wpacket=false;
			ByteBuffer pk2;
			pk2.append(worldCon.GetDataString());
			pk2.resize(pk2.size()-1);
			HandleWorldPacket(pk2);
		}


		if(!worldCon.IsConnected() && inworld){
			printf("Disconnected from World server!\n");
			inworld=false;
			_first_wpacket=true;
			_auth_sent=false;
		}

        if(clock()-ping_time>30000)
        { 
            ping_time=clock();
            SendPing(ping_time);
        }


		SDL_Delay(idleSleepTime);
		if (error)return error;




void _SaveAllCache(void){
    bool result;
    result=plrNameCache.SaveToFile();    
    // +++
}
*/


PseuInstanceConf::PseuInstanceConf()
{
}

void PseuInstanceConf::ApplyFromVarSet(VarSet &v)
{
    debug=atoi(v.Get("DEBUG").c_str());
    realmlist=v.Get("REALMLIST");
	accname=v.Get("ACCNAME");
	accpass=v.Get("ACCPASS");
	exitonerror=(bool)atoi(v.Get("EXITONERROR").c_str());
	realmport=atoi(v.Get("REALMPORT").c_str());
    clientversion_string=v.Get("CLIENTVERSION");
	clientbuild=atoi(v.Get("CLIENTBUILD").c_str());
	clientlang=v.Get("CLIENTLANG");
	realmname=v.Get("REALMNAME");
	charname=v.Get("CHARNAME");
	networksleeptime=atoi(v.Get("NETWORKSLEEPTIME").c_str());

    // clientversion is a bit more complicated to add
    {
        std::string opt=clientversion_string + ".";
        std::string num;
        uint8 p=0;
        for(uint8 i=0;i<opt.length();i++)
        {
            if(!isdigit(opt.at(i)))
            {
                clientversion[p]=(unsigned char)atoi(num.c_str());
                num.clear();
                p++;
                if(p>2)
                    break;
                continue;
            }
            num+=opt.at(i);
        }
    }
}




PseuInstanceConf::~PseuInstanceConf()
{
	//...
}


	

