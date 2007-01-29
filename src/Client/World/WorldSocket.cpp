
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldSocket.h"

WorldSocket::WorldSocket(SocketHandler &h, WorldSession *s) : TcpSocket(h)
{
    _session = s;
    _gothdr = false;
}
    

void WorldSocket::OnConnect()
{
    printf("Connected to world server.\r\n");
}

void WorldSocket::OnConnectFailed()
{
    printf("WorldSocket::OnConnectFailed()\n");
}

void WorldSocket::OnDelete()
{
    printf("Connection to world server has been closed.\n");
    if(_session)
        _session->SetSocket(NULL);
}

void WorldSocket::OnException()
{
    this->SetCloseAndDelete();
}

void WorldSocket::OnRead()
{
    TcpSocket::OnRead();
    uint32 len = ibuf.GetLength();
    //printf("WorldSocket::OnRead() %u bytes\n",len);
    if(!len)
    {
        this->CloseAndDelete();
        return;
    }

    uint8 *buf=new uint8[len];
    ibuf.Read((char*)buf,len);

    uint32 offset=0; // to skip data already read

    
    while(len > 0) // when all packets from the current ibuf are transformed into WorldPackets the remaining len will be zero
    {

        if(_gothdr) // already got header, this packet has to be the data part
        {
            _gothdr=false;
            WorldPacket *wp = new WorldPacket;
            wp->append(buf+offset,_remaining);
            wp->SetOpcode(_opcode);
            GetSession()->AddToPktQueue(wp);
            offset += _remaining; // skip the data already used
            len -= _remaining; // and adjust the length
        }
        else // no pending header stored, so this packet must be a header
        {
            ServerPktHeader hdr;
            memcpy(&hdr,buf+offset,sizeof(ServerPktHeader));
            _crypt.DecryptRecv((uint8*)&hdr,sizeof(ServerPktHeader));
            _remaining = ntohs(hdr.size)-2;
            _opcode = hdr.cmd;
            if(_opcode > 800) // no opcode has yet a number over 800
            {
                printf("CRYPT ERROR: opcode=%u, remain=%u\n",_opcode,_remaining);
                GetSession()->GetInstance()->SetError();
                // if the crypt gets messy its hardly possible to recover it, especially if we dont know
                // the lentgh of the following data part
                // TODO: invent some way how to recover the crypt (reconnect?)
                return;
            }

            // the header is fine, now check if there are more data
            if(_remaining == 0) // this is a packet with no data (like CMSG_NULL_ACTION)
            {
                WorldPacket *wp = new WorldPacket;
                wp->SetOpcode(_opcode);
                GetSession()->AddToPktQueue(wp);
                offset += 4 ; // skip the data already used
                len -= 4; // and adjust the lentgh
            }
            else // there is a data part to fetch
            {
                _gothdr=true; // only got the header, next packet wil contain the data
                offset += 4 ; // skip the data already used
                len -= 4; // and adjust the lentgh
            }
        }
    }
        
    delete buf;
}

void WorldSocket::SendWorldPacket(WorldPacket &pkt)
{
    ClientPktHeader hdr;
    memset(&hdr,0,sizeof(ClientPktHeader));
    hdr.size = ntohs(pkt.size()+4);
    hdr.cmd = pkt.GetOpcode();
    _crypt.EncryptSend((uint8*)&hdr, 6);
    ByteBuffer final(pkt.size()+6);
    final.append((uint8*)&hdr,sizeof(ClientPktHeader));
    final.append(pkt.contents(),pkt.size());
    SendBuf((char*)final.contents(),final.size());
}

void WorldSocket::InitCrypt(uint8 *key,uint32 len)
{
    _crypt.SetKey(key,len);
    _crypt.Init();
}
