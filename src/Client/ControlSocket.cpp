#include "common.h"
#include "PseuWoW.h"
#include "RemoteController.h"
#include "ControlSocket.h"

ControlSocket::ControlSocket(SocketHandler& h) : TcpSocket(h)
{
    _ok = false;
    _authed = false;
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

    SendTelnetText(_instance->GetScripts()->variables.Get("@version"));
    if(_instance->GetConf()->rmcontrolpass.size())
    {
        SendTelnetText("Authentication?");
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

    if(buf[0]==0 && len > 1)
    {
        // reserved for future applications
    }
    else
    {
        uint32 i = 0;
        while(i < len)
        {
            if(buf[i] == 127) // ignore del key for now
                continue;
            if(buf[i] == 27) // escape sequence, drop whole buf (sent on direction key press for example)
                break;
            if(buf[i] == 8) // backspace
            {
                if(_str.length())
                    _str.erase(_str.length() - 1);
            }
            else if(buf[i] == 10 || buf[i] == 13 || buf[i] == 0) // newline or \0 char
            {
                if(_str.length() && _instance && _instance->GetScripts())
                {
                    HandleString(_str);
                }
                _str.clear();
            }
            else
            {
                _str += buf[i];
            }
            ++i;
        }
    }

    delete [] buf;
}

void ControlSocket::SendTelnetText(std::string s)
{
    s += "\n\r";
    SendBuf(s.c_str(),s.size());
}

void ControlSocket::_Execute(std::string s)
{
    DefReturnResult r = _instance->GetScripts()->RunSingleLine(s);
    if(r.ok)
    {
        std::stringstream ss;
        ss << "+OK";
        if(r.ret.size())
            ss << ". r: [" << r.ret << "]";
        SendTelnetText(ss.str());
    }
    else
        SendTelnetText("+ERR");
}

void ControlSocket::HandleString(std::string s)
{
    if(_instance->GetConf()->rmcontrolpass.size())
    {
        if(_authed)
        {
            _Execute(s);
        }
        else
        {
            if(s.size() > 3 && !memicmp(s.c_str(),"pw ",3)) // string format: "pw secret12345"
            {
                if(_instance->GetConf()->rmcontrolpass == s.c_str() + 3)
                {
                    logdetail("ControlSocket: Authenticated successfully with: \"%s\"",s.c_str());
                    SendTelnetText("+accepted");
                    _authed = true;
                }
                else
                {
                    SendTelnetText("+wrong password");
                    SetCloseAndDelete(true);
                }
            }
        }
    }
    else
    {
        _Execute(s);
    }
}

