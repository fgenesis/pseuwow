
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldSocket.h"

void WorldSocket::OnConnect()
{
    printf("Connected to world server.\r\n");
}

void WorldSocket::OnRead()
{
    TcpSocket::OnRead();
    if(!ibuf.GetLength())
        return;
    uint8 *buf=new uint8[ibuf.GetLength()];
    ibuf.Read((char*)buf,ibuf.GetLength());
    GetSession()->AddToDataQueue(buf,ibuf.GetLength());
    delete buf; // UNSURE: delete or delete[]
}
