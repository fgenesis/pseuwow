#ifndef WORLD_H
#define WORLD_H

class WorldSession;
class MapMgr;

// used as interface for movement, map data,
class World
{
public:
    World(WorldSession*);
    ~World();

    uint32 GetMapId(void) { return _mapId; }
    WorldSession *GetSession(void) { return _session; }
    void Update(void);
    void UpdatePos(float,float,uint32);
    void UpdatePos(float,float);
    //GetPosZ(float x, float y);

private:
    WorldSession *_session;
    MapMgr *_mapmgr;
    uint32 _mapId;
    float _x,_y;

};

#endif
