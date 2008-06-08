#include "common.h"
#include "MapMgr.h"
#include "WorldSession.h"
#include "World.h"
#include "MovementMgr.h"

World::World(WorldSession *s)
{
    _session = s;
    _mapId = -1;
    _mapmgr = NULL;
    _movemgr = NULL;
    if(_session->GetInstance()->GetConf()->useMaps)
    {
        _mapmgr = new MapMgr();
    }

}

World::~World()
{
    Clear();
    if(_mapmgr)
        delete _mapmgr;
}

// called on SMSG_NEW_WORLD
void World::Clear(void)
{
    if(_mapmgr)
    {
        _mapmgr->Flush();
    }
    // TODO: clear WorldStates (-> SMSG_INIT_WORLD_STATES ?) and everything else thats required
}


void World::Update(void)
{
    if(_mapId == uint32(-1)) // to prevent unexpected behaviour
        return;

    if(_mapmgr)
    {
        _mapmgr->Update(_x,_y,_mapId);
    }
    if(_movemgr)
    {
        _movemgr->Update(false);
    }

    // some debug code for testing...
    /*if(_mapmgr && _x != _lastx || _y != _lasty)
    {
        logdetail("WORLD: relocation, to x=%f y=%f, calculated z=%f",_x,_y,this->GetPosZ(_x,_y));
        _lastx = _x;
        _lasty = _y;
    }*/

}

void World::UpdatePos(float x, float y, uint32 m)
{
    _mapId = m;
    UpdatePos(x,y);
}

void World::UpdatePos(float x, float y)
{
    _x = x;
    _y = y;
    Update();
}

float World::GetPosZ(float x, float y)
{
    if(_mapmgr)
        return _mapmgr->GetZ(x,y);

    logdebug("WORLD: GetPosZ() called, but no MapMgr exists (do you really use maps?)");
    return 0;
}

// must be called after MyCharacter is created
void World::CreateMoveMgr(void)
{
    _movemgr = new MovementMgr();
    _movemgr->SetInstance(_session->GetInstance());
}
