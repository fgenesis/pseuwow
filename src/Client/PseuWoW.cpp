#include "common.h"
#include "PseuWoW.h"
#include <time.h>
#include <openssl/rand.h>

#include "ByteBuffer.h"
#include "DefScript/DefScript.h"
#include "DefScriptInterface.h"
#include "Auth/BigNumber.h"
#include "DefScript/DefScript.h"
#include "RealmSession.h"
#include "WorldSession.h"
#include "CacheHandler.h"
#include "GUI/PseuGUI.h"
#include "RemoteController.h"
#include "Cli.h"


//###### Start of program code #######

PseuInstanceRunnable::PseuInstanceRunnable()
{
}

void PseuInstanceRunnable::run(void)
{
    _i = new PseuInstance(this);
	_i->SetConfDir("./conf/");
    _i->SetScpDir("./scripts/");
	if(_i->Init())
    {
        _i->Run();  
    }
    else
    {
        getchar();
    }
    delete _i;
}

void PseuInstanceRunnable::sleep(uint32 msecs)
{
    ZThread::Thread::sleep(msecs);
}

PseuInstance::PseuInstance(PseuInstanceRunnable *run)
{
    _runnable=run;
    _ver="PseuWoW Alpha Build 13.2" DEBUG_APPENDIX;
    _ver_short="A13.2" DEBUG_APPENDIX;
    _wsession=NULL;
    _rsession=NULL;
    _scp=NULL;
    _conf=NULL;
    _cli=NULL;
    _rmcontrol=NULL;
    _stop=false;
    _fastquit=false;
    _startrealm=true;
    _createws=false;
    _error=false;
    _initialized=false;


}

PseuInstance::~PseuInstance()
{
    if(_cli)
    {
        _cli->stop();
        // delete _cli; // ok this is a little mem leak... can be fixed sometime in future
    }

    if(_rmcontrol)
        delete _rmcontrol;
    if(_rsession)
        delete _rsession;
    if(_wsession)
        delete _wsession;

    delete _scp;
    delete _conf;

    log("--- Instance shut down ---");
    log_close();
}

bool PseuInstance::Init(void) {
    log_prepare("logfile.txt",this);
    log("");
    log("--- Initializing Instance ---");

    if(_confdir.empty())
        _confdir="./conf/";
    if(_scpdir.empty())
        _scpdir="./scp/";

	srand((unsigned)time(NULL));
	RAND_set_rand_method(RAND_SSLeay()); // init openssl randomizer
	
	_scp=new DefScriptPackage();
    _scp->SetParentMethod((void*)this);
	_conf=new PseuInstanceConf();	

    _scp->SetPath(_scpdir);

    _scp->variables.Set("@version_short",_ver_short);
    _scp->variables.Set("@version",_ver);
    _scp->variables.Set("@inworld","false");

    if(!_scp->LoadScriptFromFile("./_startup.def"))
    {
        logerror("Error loading '_startup.def'");
        SetError();
    }
    else if(!_scp->BoolRunScript("_startup",NULL))
    {
        logerror("Error executing '_startup.def'");
        SetError();
    }

    // TODO: find a better loaction where to place this block!
    if(GetConf()->enablegui)
    {
        uint16 x,y,depth;
        uint8 driver;
        bool shadows,vsync,win;

        driver=(uint8)atoi(GetScripts()->variables.Get("GUI::DRIVER").c_str());
        vsync=(bool)atoi(GetScripts()->variables.Get("GUI::VSYNC").c_str());
        depth=(uint8)atoi(GetScripts()->variables.Get("GUI::DEPTH").c_str());
        x=(uint16)atoi(GetScripts()->variables.Get("GUI::RESX").c_str());
        y=(uint16)atoi(GetScripts()->variables.Get("GUI::RESY").c_str());
        win=(bool)atoi(GetScripts()->variables.Get("GUI::WINDOWED").c_str());
        shadows=(bool)atoi(GetScripts()->variables.Get("GUI::SHADOWS").c_str());
        log("GUI settings: driver=%u, depth=%u, res=%ux%u, windowed=%u, shadows=%u",driver,depth,x,y,win,shadows);
        if(x>0 && y>0 && (depth==16 || depth==32) && driver>0 && driver<=5)
        {
            PseuGUIRunnable *rgui = new PseuGUIRunnable();
            PseuGUI *gui = rgui->GetGUI();
            gui->SetInstance(this);
            gui->SetDriver(driver);
            gui->SetResolution(x,y,depth);
            gui->SetVSync(vsync);
            gui->UseShadows(shadows);
            ZThread::Thread *t = new ZThread::Thread(rgui);
        }
        else
            logerror("GUI: incorrect settings!");
    }

    if(GetConf()->rmcontrolport)
    {
        _rmcontrol = new RemoteController(this,GetConf()->rmcontrolport);
    }

    if(GetConf()->enablecli)
    {
        log("Starting CLI...");
        _cli = new CliRunnable(this);
        ZThread::Thread t(_cli);
    }

    if(_error)
    {
		logcritical("Errors while initializing!");
        return false;
    }

    log("Init complete.\n");
	_initialized=true; 
	return true;
}

void PseuInstance::Run(void)
{

    if(!_initialized)
        return;

    if(GetConf()->realmlist.empty() || GetConf()->realmport==0)
    {
        logcritical("Realmlist address not set, can't connect.");
        SetError();
    }
    else
    {
        // for now: create the realmsession only on startup.
        // may be extended to a script command later on.
        // then try to connect
        _rsession = new RealmSession(this);
        _rsession->Connect();
        _rsession->SendLogonChallenge();

        // this is the mainloop
        while(!_stop)
        {
            Update();
            if(_error)
                _stop=true;
        }


    }

    // fastquit is defined if we clicked [X] (on windows)
    if(_fastquit)
    {
        log("Aborting Instance...");
        return;
    }

    log("Shutting down instance...");

    // if there was an error, better dont save, as the data might be damaged
    if(!_error)
    {
        SaveAllCache();
        //...
    }

    if(GetConf()->exitonerror == false && _error)
    {
        log("Exiting on error is disabled, PseuWoW is now IDLE");
        log("-- Press enter to exit --");
        char crap[100];
        fgets(crap,sizeof(crap),stdin); // workaround, need to press enter 2x for now
    }

}

void PseuInstance::Update()
{
    // delete sessions if they are no longer needed
    if(_rsession && _rsession->MustDie())
    {
        delete _rsession;
        _rsession = NULL;
    }

    if(_wsession && _wsession->MustDie())
    {
        delete _wsession;
        _wsession = NULL;
    }

    if(_createws)
    {
        _createws = false;
        if(_wsession)
            delete _wsession;
        _wsession = new WorldSession(this);
        _wsession->Start();
    }

    // if we have no active sessions, we may reconnect
    if((!_rsession) && (!_wsession) && GetConf()->reconnect)
    {
        logdetail("Waiting %u ms before reconnecting.",GetConf()->reconnect);
        for(uint32 t = 0; t < GetConf()->reconnect && !this->Stopped(); t+=100) Sleep(100);
        this->Sleep(1000); // wait 1 sec before reconnecting
        _rsession = new RealmSession(this);
        _rsession->Connect();
        _rsession->SendLogonChallenge(); // and login again
    }
    
    // update currently existing/active sessions
    if(_rsession)
        _rsession->Update();
    if(_wsession)
        try { _wsession->Update(); } catch (...)
        {
            logerror("Unhandled exception in WorldSession::Update()");
        }

 
    if(_rmcontrol)
    {
        _rmcontrol->Update();
        if(_rmcontrol->MustDie())
        {
            delete _rmcontrol;
            _rmcontrol = NULL;
        }
    }

    GetScripts()->GetEventMgr()->Update();

    this->Sleep(GetConf()->networksleeptime);
}

void PseuInstance::SaveAllCache(void)
{
    //...
    if(GetWSession())
    {
        GetWSession()->plrNameCache.SaveToFile();
        ItemProtoCache_WriteDataToCache(GetWSession());
        //...
    }
}

void PseuInstance::Sleep(uint32 msecs)
{
    GetRunnable()->sleep(msecs);
}

PseuInstanceConf::PseuInstanceConf()
{
    enablecli=false;
    exitonerror=false;
    debug=0;
}

void PseuInstanceConf::ApplyFromVarSet(VarSet &v)
{
    debug=atoi(v.Get("DEBUG").c_str());
    realmlist=v.Get("REALMLIST");
	accname=v.Get("ACCNAME");
	accpass=v.Get("ACCPASS");
	exitonerror=(bool)atoi(v.Get("EXITONERROR").c_str());
    reconnect=atoi(v.Get("RECONNECT").c_str());
	realmport=atoi(v.Get("REALMPORT").c_str());
    clientversion_string=v.Get("CLIENTVERSION");
	clientbuild=atoi(v.Get("CLIENTBUILD").c_str());
	clientlang=v.Get("CLIENTLANGUAGE");
	realmname=v.Get("REALMNAME");
	charname=v.Get("CHARNAME");
	networksleeptime=atoi(v.Get("NETWORKSLEEPTIME").c_str());
    showopcodes=atoi(v.Get("SHOWOPCODES").c_str());
	hidefreqopcodes=(bool)atoi(v.Get("HIDEFREQOPCODES").c_str());
    enablecli=(bool)atoi(v.Get("ENABLECLI").c_str());
    allowgamecmd=(bool)atoi(v.Get("ALLOWGAMECMD").c_str());
	enablechatai=(bool)atoi(v.Get("ENABLECHATAI").c_str());
    notifyping=(bool)atoi(v.Get("NOTIFYPING").c_str());
    showmyopcodes=(bool)atoi(v.Get("SHOWMYOPCODES").c_str());
    disablespellcheck=(bool)atoi(v.Get("DISABLESPELLCHECK").c_str());
    enablegui=(bool)atoi(v.Get("ENABLEGUI").c_str());
    rmcontrolport=atoi(v.Get("RMCONTROLPORT").c_str());
    rmcontrolhost=v.Get("RMCONTROLHOST");
    useMaps=(bool)atoi(v.Get("USEMAPS").c_str());

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


	

