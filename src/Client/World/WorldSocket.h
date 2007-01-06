#ifndef _WORLDSOCKET_H
#define _WORLDSOCKET_H

#include "Network/ResolvSocket.h"
#include "SysDefs.h"

class WorldSession;

class WorldSocket : public TcpSocket
{
public:
    WorldSocket(SocketHandler &h, WorldSession *s);
    WorldSession *GetSession(void) { return _session; }
    
    void OnRead();
    void OnConnect();
    void OnConnectFailed();

private:
    WorldSession *_session;

};

#endif