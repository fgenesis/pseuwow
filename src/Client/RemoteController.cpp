#include "common.h"
#include "log.h"
#include "ControlSocket.h"
#include "Network/ListenSocket.h"
#include "RemoteController.h"

RemoteController::RemoteController(PseuInstance *in,uint32 port)
{
    DEBUG(logdebug("RemoteController: setting instance = %X",in));
    h.SetInstance(in);
    _mustdie = false;
    _instance = in;
    ListenSocket<ControlSocket> *ls = new ListenSocket<ControlSocket>(h);
    if(ls->Bind(port))
    {
        logerror("RemoteController: Can't bind to port %u",port);
        _mustdie = true;
        return;
    }
    h.Add(ls);
    log("RemoteController: listening on port %u",port);
}

RemoteController::~RemoteController()
{
    DEBUG(logdebug("~RemoteController()"));
}

void RemoteController::Update(void)
{
    if(_mustdie)
        return;
    h.Select(0,0);
}




