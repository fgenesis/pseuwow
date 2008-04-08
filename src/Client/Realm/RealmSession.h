#ifndef REALMSESSION_H
#define REALMSESSION_H

#include "common.h"
#include "Auth/MD5Hash.h"

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
};


#endif
