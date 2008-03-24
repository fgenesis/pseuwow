#ifndef REMOTECONTROLLER_H
#define REMOTECONTROLLER_H

#include "Network/SocketHandler.h"

class PseuInstance;
class ControlSocket;

class ControlSocketHandler : public SocketHandler
{
public:
    void SetInstance(PseuInstance *in) { _instance = in; }
    PseuInstance *GetInstance(void) { return _instance; }
private:
    PseuInstance *_instance;

};



class RemoteController
{
public:
    RemoteController(PseuInstance*,uint32 port);
    ~RemoteController();
    void SetPermission(uint8 p) { _perm = p; }
    void Update(void);
    bool MustDie(void) { return _mustdie; }

private:
    ControlSocketHandler h;
    bool _mustdie;
    PseuInstance *_instance;
    uint8 _perm;
};

#endif
