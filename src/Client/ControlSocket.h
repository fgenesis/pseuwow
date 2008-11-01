#ifndef CONTROLSOCKET_H
#define CONTROLSOCKET_H

#include "Network/TcpSocket.h"
#include "RemoteController.h"

class ControlSocket : public TcpSocket
{
public:
    ControlSocket(SocketHandler& h);

    void SetInstance(PseuInstance *in) { _instance = in; }

    void OnAccept();
    void OnRead();

    void HandleString(std::string);
    void SendTelnetText(std::string);

private:
    void _Execute(std::string);
    PseuInstance *_instance;
    bool _ok;
    std::string _str;
    bool _authed;
};


#endif
