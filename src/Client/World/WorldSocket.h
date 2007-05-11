#ifndef _WORLDSOCKET_H
#define _WORLDSOCKET_H

#include "Network/TcpSocket.h"
#include "SysDefs.h"

class WorldSession;

struct ClientPktHeader
{
    uint16 size;
    uint16 cmd;
	uint16 nil;
};

struct ServerPktHeader
{
    uint16 size;
    uint16 cmd;
};

class WorldSocket : public TcpSocket
{
public:
    WorldSocket(SocketHandler &h, WorldSession *s);
    WorldSession *GetSession(void) { return _session; }
    bool IsOk();
    
    void OnRead();
    void OnConnect();
    void OnConnectFailed();
    void OnDelete();
    void OnException();

    void SendWorldPacket(WorldPacket &pkt);
    void InitCrypt(uint8*,uint32);

private:
    WorldSession *_session;
    AuthCrypt _crypt;
    bool _gothdr; // true if only the header was recieved yet
    uint16 _opcode; // stores the last recieved opcode
    uint16 _remaining; // bytes amount of the next data packet
    bool _ok;

};

#endif