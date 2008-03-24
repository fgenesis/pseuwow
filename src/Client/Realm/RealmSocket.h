#ifndef _REALMSOCKET_H
#define _REALMSOCKET_H

#include "Network/TcpSocket.h"

class RealmSession;

class RealmSocket : public TcpSocket
{
public:
    RealmSocket(SocketHandler &h);
	~RealmSocket();

	RealmSession *GetSession(void);
    bool IsOk(void);
    void SetSession(RealmSession*);
    uint32 GetMyIP(void);

    void Update(void);
    void SendLogonChallenge(void);
	
    void OnRead(void);
    void OnConnect(void);
    void OnConnectFailed(void);
    void OnException(void);
    void OnAccept(void);
    void OnDelete(void);
    int Close(void);



private:
    bool _ok;
    RealmSession *_session;


};
	
	
	
	




#endif
