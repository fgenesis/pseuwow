#ifndef MAPMGR_H
#define MAPMGR_H

class MapTileStorage;

class MapMgr
{
public:
    MapMgr();
    ~MapMgr();
    void Update(float,float,uint32);
    void Flush(void);
    float GetZ(float,float);

private:
    MapTileStorage *_tiles;
    void _LoadTile(uint32,uint32,uint32);
    void _LoadNearTiles(uint32,uint32,uint32);
    void _UnloadOldTiles(void);
    uint32 _mapid;
    uint32 _gridx,_gridy;
};

#endif