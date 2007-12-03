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

    inline uint32 GetMapId(void) { return _mapId; }
    inline WorldSession *GetSession(void) { return _session; }
    void Clear(void);
    void Update(void);
    void UpdatePos(float,float,uint32);
    void UpdatePos(float,float);
    float GetPosZ(float x, float y);
    inline MapMgr *GetMapMgr(void) { return _mapmgr; }

private:
    WorldSession *_session;
    MapMgr *_mapmgr;
    uint32 _mapId;
    float _x,_y;
    float _lastx,_lasty;

};

#endif
