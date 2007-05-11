#ifndef REALMSESSION_H
#define REALMSESSION_H

#include "common.h"

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
    void SendLogonChallenge(void);
    bool MustDie(void);
    void SetMustDie(void);



private:
    void _HandleRealmList(ByteBuffer&);
    void _HandleLogonProof(ByteBuffer&);
    void _HandleLogonChallenge(ByteBuffer&);
    AuthHandler *_GetAuthHandlerTable(void) const;
    void SendRealmPacket(ByteBuffer&);
    void DumpInvalidPacket(ByteBuffer&);

    SocketHandler _sh;
    PseuInstance *_instance;
    ZThread::LockedQueue<ByteBuffer*,ZThread::FastMutex> pktQueue;
    RealmSocket *_socket;
    uint8 _m2[20];
    RealmSession *_session;
    BigNumber _key;
    bool _mustdie;

};


#endif