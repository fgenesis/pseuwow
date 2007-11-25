#ifndef MAPMGR_H
#define MAPMGR_H

class MapTileStorage;
class MapTile;

class MapMgr
{
public:
    MapMgr();
    ~MapMgr();
    void Update(float,float,uint32);
    void Flush(void);
    float GetZ(float,float);
    uint32 GetGridCoord(float f);
    MapTile *GetTile(uint32 xg, uint32 yg, bool forceLoad = false);
    MapTile *GetCurrentTile(void);
    MapTile *GetNearTile(int32, int32);
    inline bool Loaded(void) { return _mapsLoaded; }

private:
    MapTileStorage *_tiles;
    void _LoadTile(uint32,uint32,uint32);
    void _LoadNearTiles(uint32,uint32,uint32);
    void _UnloadOldTiles(void);
    uint32 _mapid;
    uint32 _gridx,_gridy;
    bool _mapsLoaded;
};

#endif