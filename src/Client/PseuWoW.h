
#ifndef _PSEUWOW_H
#define _PSEUWOW_H

#include "../shared/common.h"
#include "Auth/BigNumber.h"
#include "DefScript/DefScript.h"
#include "../shared/Network/SocketHandler.h"


class RealmSocket;
class WorldSession;
class Sockethandler;

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
	
	
};
	

class PseuInstance
{
	public:

	PseuInstance();
	~PseuInstance();	
	

    WorldSession *GetWSession(void) { return _wsession; }
    RealmSocket *GetRSession(void) { return _rsession; }
    PseuInstanceConf *GetConf(void) { return _conf; }
    DefScriptPackage *GetScripts(void) { return _scp; }
    void SetConfDir(std::string dir) { _confdir = dir; }
    void SetScpDir(std::string dir) { _scpdir = dir; }
    void SetSessionKey(BigNumber key) { _sessionkey = key; }
    BigNumber GetSessionKey(void) { return _sessionkey; }

	
	
	
	bool Init();
	void SaveAllCache(void);
    void Stop(void) { _stop = true; }
	void Quit(void);
    void Run(void);
    void Update(void);	
	
	
	
	private:

	RealmSocket *_rsession;
	WorldSession *_wsession;
	PseuInstanceConf *_conf;
	DefScriptPackage *_scp;
	std::string _confdir,_scpdir;
	bool _initialized;	
	bool _stop;
	BigNumber _sessionkey;
    char *_ver,*_ver_short;
    SocketHandler _sh;


};

class PseuInstanceRunnable : public ZThread::Runnable
{
public:
    void run();
};
	
	
	

// OBOELETE
/*

class SDLTCPConnection;
class DefScriptPackage;
class PlayerNameCache;
class VarSet;

extern char DEBUG;
extern char *ver;
extern char *realmlist,*accname,*accpass,*realmname;
extern bool quit,quitted,exitonerror;
extern unsigned char error;
extern unsigned char clientversion[3];
extern unsigned short clientbuild;
extern char clientlang[4];
extern bool something_went_wrong;

extern unsigned int c_port;
extern bool allowcontroller;
extern char *c_password;

extern unsigned int rs_port;
extern unsigned short clientbuild;
extern char clientlang[4];

extern unsigned int ws_port;
extern std::string worldhost, charname;
extern SDLTCPConnection worldCon,realmCon,ctrlCon;
extern bool inworld;
extern unsigned short idleSleepTime;

extern DefScriptPackage defScp;
extern std::string defScpPath;

extern PlayerNameCache plrNameCache;

extern VarSet playerPermissions;

extern uint64 _myGUID, _targetGUID, _followGUID;


// --- Some Functions ---

void quitproc_error(void);
void quitproc(void);
void _SaveAllCache(void);

*/

#endif