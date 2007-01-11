
#ifndef _PSEUWOW_H
#define _PSEUWOW_H

#include "common.h"
#include "Auth/BigNumber.h"
#include "DefScript/DefScript.h"
#include "Network/SocketHandler.h"

class RealmSocket;
class WorldSession;
class Sockethandler;
class PseuInstanceRunnable;
class CliRunnable;

class PseuInstanceConf
{
	public:
		
	PseuInstanceConf();
	~PseuInstanceConf();
	void ApplyFromVarSet(VarSet &v);
	
	
	uint8 debug;
	std::string realmlist;
	std::string accname;
	std::string accpass;
	bool exitonerror;
    bool reconnect;
	uint16 realmport;
    uint16 worldport;
	uint8 clientversion[3];
    std::string clientversion_string;
	uint16 clientbuild;
	std::string clientlang;
	std::string realmname;
	std::string charname;
    std::string worldhost;
	uint16 networksleeptime;
    uint8 showopcodes;
    bool allowgamecmd;
    bool enablecli;
	
	
};
	

class PseuInstance
{
	public:

	PseuInstance(PseuInstanceRunnable *run);
	~PseuInstance();	
	

    WorldSession *GetWSession(void) { return _wsession; }
    RealmSocket *GetRSession(void) { return _rsession; }
    PseuInstanceConf *GetConf(void) { return _conf; }
    DefScriptPackage *GetScripts(void) { return _scp; }
    PseuInstanceRunnable *GetRunnable(void) { return _runnable; }
    void SetConfDir(std::string dir) { _confdir = dir; }
    void SetScpDir(std::string dir) { _scpdir = dir; }
    void SetSessionKey(BigNumber key) { _sessionkey = key; }
    BigNumber GetSessionKey(void) { return _sessionkey; }

	
	
	
	bool Init();
	void SaveAllCache(void);
    void Stop(void) { _stop = true; }
    void SetFastQuit(bool q=true) { _fastquit=true; }
	void Quit(void);
    void Run(void);
    void Update(void);	
    void Sleep(uint32 msecs);
	
	
	bool createWorldSession;

	private:

    PseuInstanceRunnable *_runnable;
	RealmSocket *_rsession;
	WorldSession *_wsession;
	PseuInstanceConf *_conf;
	DefScriptPackage *_scp;
	std::string _confdir,_scpdir;
	bool _initialized;	
	bool _stop,_fastquit;
    bool _startrealm;
	BigNumber _sessionkey;
    char *_ver,*_ver_short;
    SocketHandler _sh;
    CliRunnable *_cli;
    ZThread::Thread _clithread;


};

class PseuInstanceRunnable : public ZThread::Runnable
{
public:
    PseuInstanceRunnable::PseuInstanceRunnable();
    void run(void);
    void sleep(uint32);
    PseuInstance *GetInstance(void) { return _i; }

private:
    PseuInstance *_i;
};


#endif