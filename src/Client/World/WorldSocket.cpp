
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

void WorldSocket::OnRead()
{
    TcpSocket::OnRead();
    uint32 len = ibuf.GetLength();
    printf("WorldSocket::OnRead() %u bytes\n",len);
    if(!len)
        return;

    uint8 *buf=new uint8[len];
    ibuf.Read((char*)buf,len);

    ByteBuffer bb;
    bb.append(buf,len);
    bb.hexlike();

    uint32 offset=0;

    // this is a bit tricky, but needed, because sometimes packets come over as [[hdr][data][hdr]]
    // and the 2nd header needs to be extracted too
    while(true)
    {

        if(_gothdr) // already got header, this packet has to be the data part
        {
            _gothdr=false;
            if( len != _remaining )
            {
                printf("WP: Recieved packet is not correct (%u of %u)\n",len,_remaining);
                break;
            }

            printf("WP: Building complete WorldPacket with opcode %u\n",_opcode);
            WorldPacket *wp = new WorldPacket;
            wp->append(buf+offset,len);
            wp->SetOpcode(_opcode);
            GetSession()->AddToPktQueue(wp);
            break;
        }
        else // no pending header stored, so this packet must be a header
        {
            printf("WP: Got header (%u bytes)\n",len);
            ServerPktHeader hdr;
            memcpy(&hdr,buf+offset,sizeof(ServerPktHeader));
            _crypt.DecryptRecv((uint8*)&hdr,sizeof(ServerPktHeader));
            _remaining = ntohs(hdr.size)-2;
            _opcode = hdr.cmd;
            printf("WP: Opcode=%u, remains=%u\n",_opcode,_remaining);

            // the header is fine, now check if there are more data
            if(_remaining == 0)
            {
                printf("WP: Got header-only Packet, building WorldPacket\n");
                WorldPacket *wp = new WorldPacket;
                wp->SetOpcode(_opcode);
                GetSession()->AddToPktQueue(wp);
                break;
            }
            else if( _remaining && len-4==0 )
            {
                printf("WP: Got ONLY a header, waiting for data...\n");
                _gothdr=true; // only got the header, next packet wil contain the data
                break;
            }
            else // if the packet is bigger then 4 bytes, header & packet are merged
            {
                if( (len - sizeof(ServerPktHeader)) == _remaining )
                {
                    printf("WP: Got merged Packet, building WorldPacket\n");
                    WorldPacket *wp = new WorldPacket;
                    wp->append(buf+4+offset,_remaining); // skip first 4 bytes (header)
                    wp->SetOpcode(_opcode);
                    GetSession()->AddToPktQueue(wp);
                    break;
                }
                else if ( (len - sizeof(ServerPktHeader)) < _remaining )
                {
                    printf("WP: Got too small merged packet (total=%u, data=%u)\n",len,_remaining);
                    break;
                }
                else if( (len - sizeof(ServerPktHeader)) > _remaining)
                {
                    WorldPacket *wp = new WorldPacket;
                    wp->append(buf+4+offset,_remaining); // skip first 4 bytes (header)
                    wp->SetOpcode(_opcode);
                    GetSession()->AddToPktQueue(wp);

                    offset += (4 + _remaining); // skip the header & the data already used
                    len -= (4 + _remaining);
                    printf("WP: Got too big merged packet, new offset=%u, new len=%u, reading more data..\n",offset,len);

                    if(len <= 0)
                        break; // all data done
                    else
                        continue; // read next part
                }
            }
        }
    }
        
    delete buf; // UNSURE: delete or delete[]
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
