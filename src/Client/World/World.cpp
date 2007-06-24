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
    