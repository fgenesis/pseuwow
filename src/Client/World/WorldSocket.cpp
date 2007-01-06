
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldSocket.h"

WorldSocket::WorldSocket(SocketHandler &h, WorldSession *s) : TcpSocket(h)
{
    _session = s;
}
    

void WorldSocket::OnConnect()
{
    printf("Connected to world server.\r\n");
}

void WorldSocket::OnConnectFailed()
{
    printf("WorldSocket::OnConnectFailed()\n");
}

void WorldSocket::OnRead()
{
    TcpSocket::OnRead();
    printf("WorldSocket::OnRead() %u bytes\n",ibuf.GetLength());
    if(!ibuf.GetLength())
        return;
    uint8 *buf=new uint8[ibuf.GetLength()];
    ibuf.Read((char*)buf,ibuf.GetLength());
    GetSession()->AddToDataQueue(buf,ibuf.GetLength());
    delete buf; // UNSURE: delete or delete[]
}
