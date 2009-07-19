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
#include "GUI/SceneData.h"
#include "MemoryDataHolder.h"


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
        getchar(); // if init failed, wait for keypress before exit
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
    _ver="PseuWoW Alpha Build 13.51" DEBUG_APPENDIX;
    _ver_short="A13.51" DEBUG_APPENDIX;
    _wsession=NULL;
    _rsession=NULL;
    _scp=NULL;
    _conf=NULL;
    _cli=NULL;
    _rmcontrol=NULL;
    _gui=NULL;
    _guithread=NULL;
    _stop=false;
    _fastquit=false;
    _startrealm=true;
    _createws=false;
    _creaters=false;
    _error=false;
    _initialized=false;
    for(uint32 i = 0; i < COND_MAX; i++)
    {
        _condition[i] = new ZThread::Condition(_mutex);
    }

}

PseuInstance::~PseuInstance()
{
    if(_cli)
    {
        _cli->stop();
        // delete _cli; // ok this is a little mem leak... can be fixed sometime in future
    }

    if(_gui)
        _gui->Shutdown();
    logdebug("Waiting for GUI to quit...");
    while(_gui)
        Sleep(1);

    if(_guithread)
        _guithread->wait();

    if(_rmcontrol)
        delete _rmcontrol;
    if(_rsession)
        delete _rsession;
    if(_wsession)
        delete _wsession;

    delete _scp;
    delete _conf;

    for(uint32 i = 0; i < COND_MAX; i++)
    {
        delete _condition[i];
    }

    log("--- Instance shut down ---");
}

bool PseuInstance::Init(void)
{
    log_setloglevel(0);
    log("");
    log("--- Initializing Instance ---");

    if(_confdir.empty())
        _confdir="./conf/";
    if(_scpdir.empty())
        _scpdir="./scripts/";

    srand((unsigned)time(NULL));
    RAND_set_rand_method(RAND_SSLeay()); // init openssl randomizer

    _scp=new DefScriptPackage();
    _scp->SetParentMethod((void*)this);
    _conf=new PseuInstanceConf();

    _scp->SetPath(_scpdir);

    CreateDir("cache");

    dbmgr.AddSearchPath("./cache");
    dbmgr.AddSearchPath("./data/scp");
    dbmgr.SetCompression(6);

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
        if(InitGUI())
            logdebug("GUI: Init successful.");
        else
            logerror("GUI: Init failed!");
    }

    if(GetConf()->rmcontrolport)
    {
        _rmcontrol = new RemoteController(this,GetConf()->rmcontrolport);
    }

#if !(PLATFORM == PLATFORM_WIN32 && !defined(_CONSOLE))
    if(GetConf()->enablecli)
    {
        log("Starting CLI...");
        _cli = new CliRunnable(this);
        ZThread::Thread t(_cli);
    }
#endif

    if(_error)
    {
        logcritical("Errors while initializing!");
        return false;
    }

    log("Init complete.");
    _initialized=true;
    return true;
}

bool PseuInstance::InitGUI(void)
{
    if(GetGUI())
    {
        logerror("GUI: Aborting init, GUI already exists!");
        return false;
    }

    /*if (!GetConf()->enablegui)
    {
        logdebug("GUI: Can't start, gui disabled in config");
        return false;
    }*/

    uint16 x,y,depth;
    uint8 driver;
    bool shadows,vsync,win,usesound;

    driver=(uint8)atoi(GetScripts()->variables.Get("GUI::DRIVER").c_str());
    vsync=(bool)atoi(GetScripts()->variables.Get("GUI::VSYNC").c_str());
    depth=(uint8)atoi(GetScripts()->variables.Get("GUI::DEPTH").c_str());
    x=(uint16)atoi(GetScripts()->variables.Get("GUI::RESX").c_str());
    y=(uint16)atoi(GetScripts()->variables.Get("GUI::RESY").c_str());
    win=(bool)atoi(GetScripts()->variables.Get("GUI::WINDOWED").c_str());
    shadows=(bool)atoi(GetScripts()->variables.Get("GUI::SHADOWS").c_str());
    usesound=(bool)atoi(GetScripts()->variables.Get("GUI::USESOUND").c_str());
    log("GUI settings: driver=%u, depth=%u, res=%ux%u, windowed=%u, shadows=%u sound=%u",driver,depth,x,y,win,shadows,usesound);
    if(x>0 && y>0 && (depth==16 || depth==32) && driver>0 && driver<=5)
    {
        PseuGUIRunnable *rgui = new PseuGUIRunnable();
        _gui = rgui->GetGUI();
        _gui->SetInstance(this);
        _gui->SetDriver(driver);
        _gui->SetResolution(x,y,depth);
        _gui->SetVSync(vsync);
        _gui->UseShadows(shadows);
        _gui->SetWindowed(win);
        _gui->SetUseSound(usesound);
        _guithread = new ZThread::Thread(rgui);
        return true;
    }
    else
        logerror("GUI: incorrect settings!");
    return false;
}

void PseuInstance::Run(void)
{
    if(!_initialized)
        return;

    logdetail("PseuInstance: Initialized and running!");

    if(GetGUI())
    {
        while(!GetGUI()->IsInitialized())
            Sleep(1); // wait until the gui is ready. it will crash otherwise
        logdebug("GUI: switching to startup display...");
        GetGUI()->SetSceneState(SCENESTATE_GUISTART);
    }
    // TODO: as soon as username and password can be inputted into the gui, wait until it was set by user.

    if(GetConf()->realmlist.empty() || GetConf()->realmport==0)
    {
        logcritical("Realmlist address not set, can't connect.");
        SetError();
    }
    else
    {

        if(!GetConf()->enablegui || !(GetConf()->accname.empty() || GetConf()->accpass.empty()) )
        {
            logdebug("GUI not active or Login data pre-entered, skipping Login GUI");
            CreateRealmSession();
        }
        else
        {
            GetGUI()->SetSceneState(SCENESTATE_LOGINSCREEN);
        }

        // this is the mainloop
        while(!_stop)
        {
            Update();
            if(_error)
                _stop=true;
        }


    }

    // fastquit is defined if we clicked [X] (on windows)
    // If softquit is set, do not terminate forcefully, but shut it down instead
    if(_fastquit && !_conf->softquit)
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

    if(GetScripts()->ScriptExists("_onexit"))
    {
        CmdSet Set;
        Set.arg[0] = DefScriptTools::toString(_error);
        GetScripts()->RunScript("_onexit",&Set);
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
    // if the user typed anything into the console, process it before anything else.
    // note that it can also be used for simulated cli commands sent by other threads, so it needs to be checked even if cli is disabled
    ProcessCliQueue();

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

    if(_creaters)
    {
        _creaters = false;
        if(_rsession)
            delete _rsession;
        ConnectToRealm();
    }

    // if we have no active sessions, we may reconnect, if no GUI is active for login
    if((!_rsession) && (!_wsession) && GetConf()->reconnect && !_gui)
    {
        if(GetConf()->accname.empty() || GetConf()->accpass.empty())
        {
            logdev("Skipping reconnect, acc name or password not set");
        }
        else
        {   // everything fine, we have all data
            logdetail("Waiting %u ms before reconnecting.",GetConf()->reconnect);
            for(uint32 t = 0; t < GetConf()->reconnect && !this->Stopped(); t+=100) Sleep(100);
            this->Sleep(1000); // wait 1 sec before reconnecting
            CreateRealmSession();
        }
    }
    if((!_rsession) && (!_wsession) && _gui)
    {
        if(_gui->GetSceneState() != SCENESTATE_LOGINSCREEN)
        {
            logdetail("Disconnected, switching GUI back to Loginscreen.");
            _gui->SetSceneState(SCENESTATE_LOGINSCREEN);
            while(_gui && _gui->GetSceneState() != SCENESTATE_LOGINSCREEN) // .. and wait until scenestate is set
                Sleep(1);
        }
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

void PseuInstance::ProcessCliQueue(void)
{
    std::string cmd;
    while(_cliQueue.size())
    {
        cmd = _cliQueue.next();
        try
        {
            GetScripts()->RunSingleLine(cmd);
        }
        catch(...)
        {
            logerror("Exception while executing CLI command: \"%s\"",cmd.c_str());
        }
    }
}

void PseuInstance::AddCliCommand(std::string cmd)
{
    _cliQueue.add(cmd);
}

void PseuInstance::SaveAllCache(void)
{
    //...
    if(GetWSession())
    {
        GetWSession()->plrNameCache.SaveToFile();
        ItemProtoCache_WriteDataToCache(GetWSession());
        CreatureTemplateCache_WriteDataToCache(GetWSession());
        GOTemplateCache_WriteDataToCache(GetWSession());
        //...
    }
}

void PseuInstance::Sleep(uint32 msecs)
{
    GetRunnable()->sleep(msecs);
}

void PseuInstance::DeleteGUI(void)
{
    _gui = NULL;
    delete _guithread; // since it was allocated with new
    _guithread = NULL;
    if(GetScripts()->ScriptExists("_onguiclose"))
        AddCliCommand("_onguiclose"); // since this func is called from another thread, use threadsafe variant via CLI

    // if console mode is disabled in windows, closing the gui needs to close the app
#if PLATFORM == PLATFORM_WIN32 && !defined(_CONSOLE)
    this->Stop();
#endif
}

bool PseuInstance::ConnectToRealm(void)
{
    _rsession = new RealmSession(this);
    _rsession->SetLogonData(); // get accname & accpass from PseuInstanceConfig and set it in the realm session
    _rsession->Connect();
    if(_rsession->MustDie() || !_rsession->SocketGood()) // something failed. it will be deleted in next Update() call
    {
        logerror("PseuInstance: Connecting to Realm failed!");
        if(_gui)
            _gui->SetSceneData(ISCENE_LOGIN_CONN_STATUS, DSCENE_LOGIN_CONN_FAILED);
        return false;
    }

    _rsession->SendLogonChallenge();
    return true;
}

void PseuInstance::WaitForCondition(InstanceConditions c, uint32 timeout /* = 0 */)
{
    _mutex.acquire();
    if(timeout)
        _condition[c]->wait(timeout);
    else
        _condition[c]->wait();
    _mutex.release();
}

PseuInstanceConf::PseuInstanceConf()
{
    enablecli=false;
    enablegui=false;
    exitonerror=false;
    debug=0;
    rmcontrolport=0;
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
    hideDisabledOpcodes=(bool)atoi(v.Get("HIDEDISABLEDOPCODES").c_str());
    enablecli=(bool)atoi(v.Get("ENABLECLI").c_str());
    allowgamecmd=(bool)atoi(v.Get("ALLOWGAMECMD").c_str());
    notifyping=(bool)atoi(v.Get("NOTIFYPING").c_str());
    showmyopcodes=(bool)atoi(v.Get("SHOWMYOPCODES").c_str());
    disablespellcheck=(bool)atoi(v.Get("DISABLESPELLCHECK").c_str());
    enablegui=(bool)atoi(v.Get("ENABLEGUI").c_str());
    rmcontrolport=atoi(v.Get("RMCONTROLPORT").c_str());
    rmcontrolhost=v.Get("RMCONTROLHOST");
    rmcontrolpass=v.Get("RMCONTROLPASS");
    useMaps=(bool)atoi(v.Get("USEMAPS").c_str());
    skipaddonchat=(bool)atoi(v.Get("SKIPADDONCHAT").c_str());
    dumpPackets=(uint8)atoi(v.Get("DUMPPACKETS").c_str());
    softquit=(bool)atoi(v.Get("SOFTQUIT").c_str());
    dataLoaderThreads=atoi(v.Get("DATALOADERTHREADS").c_str());

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

    // GUI related
    terrainsectors = atoi(v.Get("GUI::TERRAINSECTORS").c_str());
    terrainrendersize = atoi(v.Get("GUI::TERRAINRENDERSIZE").c_str());
    terrainupdatestep = atoi(v.Get("GUI::TERRAINUPDATESTEP").c_str());
    farclip = atof(v.Get("GUI::FARCLIP").c_str());
    fogfar = atof(v.Get("GUI::FOGFAR").c_str());
    fognear = atof(v.Get("GUI::FOGNEAR").c_str());
    fov = atof(v.Get("GUI::FOV").c_str());
    masterSoundVolume = atof(v.Get("GUI::MASTERSOUNDVOLUME").c_str());

    // cleanups, internal settings, etc.
    log_setloglevel(debug);
    log_setlogtime((bool)atoi(v.Get("LOGTIME").c_str()));
    MemoryDataHolder::SetThreadCount(dataLoaderThreads);
}




PseuInstanceConf::~PseuInstanceConf()
{
        //...
}




