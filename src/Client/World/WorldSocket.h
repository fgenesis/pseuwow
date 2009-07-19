#ifndef _WORLDSOCKET_H
#define _WORLDSOCKET_H

#include "Network/TcpSocket.h"
#include "SysDefs.h"

class WorldSession;
class BigNumber;

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct ClientPktHeader
{
    uint16 size;
    uint16 cmd;
	uint16 nil;
};

struct ServerPktHeader
{
    ServerPktHeader() : size(0), cmd(0) {}
    uint16 size;
    uint16 cmd;
};

struct ServerPktHeaderBig
{
    ServerPktHeaderBig() : cmd(0) { memset(size, 0, 3); }
    uint8 size[3];
    uint16 cmd;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif


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
    void InitCrypt(BigNumber *);

private:
    WorldSession *_session;
    AuthCrypt _crypt;
    bool _gothdr; // true if only the header was recieved yet
    uint16 _opcode; // stores the last recieved opcode
    uint32 _remaining; // bytes amount of the next data packet
    bool _ok;

};

#endif
