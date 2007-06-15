#include "common.h"
#include "PseuWoW.h"
#include "RemoteController.h"
#include "ControlSocket.h"

ControlSocket::ControlSocket(SocketHandler& h) : TcpSocket(h)
{
    _ok = false;
    DEBUG(logdebug("ControlSocket created"));
}

void ControlSocket::OnAccept(void)
{
    logdetail("ControlSocket: Incoming connection from %s:%u [host:%s]",GetRemoteAddress().c_str(),GetRemotePort(),GetRemoteHostname().c_str());
   
    // must perform some crappy ptr conversion here, doesnt want to typecast SocketHandler -> ControlSocketHandler directly
    SocketHandler& hnd = Handler();
    ControlSocketHandler *chnd = static_cast<ControlSocketHandler*>(&hnd);
    _instance = chnd->GetInstance();
    DEBUG(logdebug("ControlSocket: setting instance = %X",_instance));

    // accept only connections from one host for now, if set
    if(_instance->GetConf()->rmcontrolhost.length()
        && !(GetRemoteAddress() == _instance->GetConf()->rmcontrolhost || GetRemoteHostname() == _instance->GetConf()->rmcontrolhost))
    {
        logdetail("ControlSocket: connection rejected. closing.");
        SetCloseAndDelete(true);
        return;
    }

    _ok = true;
}

void ControlSocket::OnRead(void)
{
    if(!_ok)
        return;
    TcpSocket::OnRead();
    uint32 len = ibuf.GetLength();
    if(!len)
    {
        logdetail("ControlSocket: connection to %s:%u closed.",GetRemoteAddress().c_str(),GetRemotePort());
        return;
    }

    char *buf = new char[len];
    ibuf.Read(buf,len);
    if(buf[0]==0)
    {
        // reserved for future applications
    }
    else
    {
        if(_instance && _instance->GetScripts())
            _instance->GetScripts()->RunSingleLine(&buf[0]);
    }

    delete [] buf;
}
