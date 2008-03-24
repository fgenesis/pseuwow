#include "common.h"
#include "PseuWoW.h"
#include "ByteBuffer.h"
#include "RealmSession.h"
#include "RealmSocket.h"
#include "Network/Utility.h"




RealmSocket::RealmSocket(SocketHandler& h) : TcpSocket(h)
{
    _session = NULL;
    _ok = false;
}

RealmSocket::~RealmSocket()
{
    this->Close();
}

bool RealmSocket::IsOk(void)
{
    return _ok;
}

void RealmSocket::SetSession(RealmSession *rs)
{
    _session = rs;
}

int RealmSocket::Close(void)
{
    _ok = false;
    return TcpSocket::Close();
}


void RealmSocket::OnRead(void)
{
    TcpSocket::OnRead();
    uint32 len = ibuf.GetLength();
    logdev("RealmSocket::OnRead() %u bytes",len);
    if(!len)
        return;
    ByteBuffer *pkt = new ByteBuffer(len);
    uint8* data = new uint8[len];
    ibuf.Read((char*)data,len);
    pkt->append(data,len);
    delete [] data;
    _session->AddToPktQueue(pkt);
}


RealmSession *RealmSocket::GetSession(void)
{
    return _session;
}

void RealmSocket::OnAccept(void)
{
    logdev("RealmSocket accepted.");
}

void RealmSocket::OnConnect(void)
{
    logdetail("RealmSocket connected!");
    _ok = true;
}

void RealmSocket::OnConnectFailed(void)
{
    logerror("Connecting to Realm failed!");
    _ok = false;
}

void RealmSocket::OnException(void)
{
    if(_ok)
    {
        logerror("RealmSocket: Exception!");
        _ok = false;
    }
}

void RealmSocket::OnDelete(void)
{
    _ok = false;
    _session->SetMustDie();
}

uint32 RealmSocket::GetMyIP(void)
{
    return GetRemoteIP4();
}
    
