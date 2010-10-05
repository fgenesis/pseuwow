#ifndef MAPMGR_H
#define MAPMGR_H

#include "PseuWoW.h"
#include "SCPDatabase.h"

class MapTileStorage;
class MapTile;

struct GridCoordPair
{
    GridCoordPair() {}
    GridCoordPair(uint32 xu, uint32 yu) { x = xu; y = yu; }
    uint32 x;
    uint32 y;
};

class MapMgr
{
public:
    MapMgr(PseuInstance*);
    ~MapMgr();
    void Update(float,float,uint32);
    void Flush(void);
    float GetZ(float,float);
    static uint32 GetGridCoord(float f);
    static GridCoordPair GetTransformGridCoordPair(float x, float y);
    MapTile *GetTile(uint32 xg, uint32 yg, bool forceLoad = false);
    MapTile *GetCurrentTile(void);
    MapTile *GetNearTile(int32, int32);
    char* MapID2Name(uint32);
    inline bool Loaded(void) { return _mapsLoaded; }
    uint32 GetLoadedMapsCount(void);
    std::string GetLoadedTilesString(void);
    inline uint32 GetGridX(void) { return _gridx; }
    inline uint32 GetGridY(void) { return _gridy; }

private:
    PseuInstance *_instance;
    SCPDatabase* mapdb;
    MapTileStorage *_tiles;
    void _LoadTile(uint32,uint32,uint32);
    void _LoadNearTiles(uint32,uint32,uint32);
    void _UnloadOldTiles(void);
    uint32 _mapid;
    uint32 _gridx,_gridy;
    bool _mapsLoaded;
};

#endif
