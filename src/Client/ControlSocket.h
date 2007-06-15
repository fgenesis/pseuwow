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

private:
    PseuInstance *_instance;
    bool _ok;
};


#endif
