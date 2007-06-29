#include "common.h"
#include "MapMgr.h"
#include "WorldSession.h"
#include "World.h"

World::World(WorldSession *s)
{
    _session = s;
    _mapId = -1;
    _mapmgr = NULL;
    if(_session->GetInstance()->GetConf()->useMaps)
        _mapmgr = new MapMgr();
}

World::~World()
{
    if(_mapmgr)
        delete _mapmgr;
}

void World::Update(void)
{
    if(_mapmgr)
    {
        _mapmgr->Update(_x,_y,_mapId);
    }

    // some debug code for testing...
    if(_mapmgr && _x != _lastx || _y != _lasty)
    {
        logdetail("WORLD: relocation, to x=%f y=%f, calculated z=%f",_x,_y,this->GetPosZ(_x,_y));
        _lastx = _x;
        _lasty = _y;
    }
        
}

void World::UpdatePos(float x, float y, uint32 m)
{
    UpdatePos(x,y);
    _mapId = m;
}

void World::UpdatePos(float x, float y)
{
    _x = x;
    _y = y;
}

float World::GetPosZ(float x, float y)
{
    if(_mapmgr)
        return _mapmgr->GetZ(x,y);

    logdebug("WORLD: GetPosZ() called, but no MapMgr exists (do you really use maps?)");
    return 0;
}
    