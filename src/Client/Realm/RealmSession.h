#ifndef REALMSESSION_H
#define REALMSESSION_H

#include "common.h"
#include "Auth/MD5Hash.h"

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

struct AuthHandler;
class RealmSocket;

class RealmSession
{
public:
    RealmSession(PseuInstance*);
    ~RealmSession();
    void AddToPktQueue(ByteBuffer*);
    void Connect(void);
    void Update(void);
    PseuInstance *GetInstance(void);
    void ClearSocket(void);
    void SetLogonData(void);
    void SendLogonChallenge(void);
    bool MustDie(void);
    void SetMustDie(void);
    bool SocketGood(void);
    void SetRealmAddr(std::string);
    inline uint32 GetRealmCount(void) { return _realms.size(); }
    inline SRealmInfo& GetRealm(uint32 i) { return _realms[i]; }


private:
    void _HandleRealmList(ByteBuffer&);
    void _HandleLogonProof(ByteBuffer&);
    void _HandleLogonChallenge(ByteBuffer&);
    void _HandleTransferInit(ByteBuffer&);
    void _HandleTransferData(ByteBuffer&);
    AuthHandler *_GetAuthHandlerTable(void) const;
    void SendRealmPacket(ByteBuffer&);
    void DumpInvalidPacket(ByteBuffer&);
    void DieOrReconnect(bool err = false);
    std::string _accname,_accpass;
    SocketHandler _sh;
    PseuInstance *_instance;
    ZThread::LockedQueue<ByteBuffer*,ZThread::FastMutex> pktQueue;
    RealmSocket *_socket;
    uint8 _m2[20];
    RealmSession *_session;
    BigNumber _key;
    bool _mustdie;
    bool _filetransfer;
    uint8 _file_md5[MD5_DIGEST_LENGTH];
    uint64 _file_done, _file_size;
    ByteBuffer _filebuf;
    ByteBuffer _transbuf; // stores parts of unfinished packets
    std::vector<SRealmInfo> _realms;
};


#endif
