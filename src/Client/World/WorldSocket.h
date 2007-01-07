#ifndef _WORLDSOCKET_H
#define _WORLDSOCKET_H

#include "Network/ResolvSocket.h"
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
    
    void OnRead();
    void OnConnect();
    void OnConnectFailed();

    void SendWorldPacket(WorldPacket &pkt);
    void InitCrypt(uint8*,uint32);

private:
    WorldSession *_session;
    AuthCrypt _crypt;
    bool _gothdr; // true if only the header was recieved yet
    ByteBuffer _hdr;
    uint16 _opcode;
    uint16 _remaining;

};

#endif