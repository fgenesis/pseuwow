#ifndef WORLD_H
#define WORLD_H

class WorldSession;
class MapMgr;
class MovementMgr;

struct WorldPosition
{
    WorldPosition() : x(0.0f), y(0.0f), z(0.0f), o(0.0f) {};
    WorldPosition(float px, float py) : x(px), y(py), z(0.0f), o(0.0f) {};
    WorldPosition(float px, float py, float pz) : x(px), y(py), z(pz), o(0.0f) {};
    WorldPosition(float px, float py, float pz, float po) : x(px), y(py), z(pz), o(po) {};
    float x,y,z,o;
};

inline ByteBuffer& operator<<(ByteBuffer& bb, WorldPosition p)
{
    bb << p.x << p.y << p.z << p.o;
    return bb;
}
inline ByteBuffer& operator>>(ByteBuffer& bb, WorldPosition& p)
{
    bb >> p.x >> p.y >> p.z >> p.o;
    return bb;
}

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
    inline MovementMgr *GetMoveMgr(void) { return _movemgr; }
    void CreateMoveMgr(void);

private:
    WorldSession *_session;
    MapMgr *_mapmgr;
    uint32 _mapId;
    float _x,_y;
    float _lastx,_lasty;

    MovementMgr *_movemgr;

};

#endif
