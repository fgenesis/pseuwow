#ifndef MAPTILE_H
#define MAPTILE_H

#include <bitset>

#include "WDTFile.h"
#include "ADTFile.h"

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)
#define ZEROPOINT (32.0f * (TILESIZE))

// individual chunks of a map
class MapChunk
{
public:
    float hmap_rough[9*9];
    float hmap_fine[8*8];
    float hmap[17*17]; // combined rough and fine hmap
    float basex,basey,baseheight,lqheight;
    float hmap_lq[9*9]; // liquid (water, lava) height map
    //... TODO: implement the rest of this
};

// generic map tile class. stores the information previously stored in an ADT file
// in an easier to use form.
class MapTile
{
public:
    MapTile();
    ~MapTile();
    void ImportFromADT(ADTFile*);
    float GetZ(float,float);
    void DebugDumpToFile(void);
    inline MapChunk *GetChunk(uint32 x, uint32 y) { return &_chunks[y * 16 + x]; }
    inline float GetBaseX(void) { return _xbase; }
    inline float GetBaseY(void) { return _ybase; }
    inline float GetBaseHeight(void) { return _hbase; }

private:
    MapChunk _chunks[256]; // 16x16
    std::vector<std::string> _textures;
    std::vector<std::string> _wmos;
    std::vector<std::string> _models;

    float _xbase,_ybase,_hbase;

};

// store which map tiles are present in the world
class MapTileStorage
{
public:
    inline MapTileStorage()
    {
        memset(_tiles,0,sizeof(MapTile*)*4096);
    }
    inline ~MapTileStorage()
    {
        for(uint32 i=0; i<4096; i++)
            UnloadMapTile(i);
    }
    inline void ImportTileMap(WDTFile* w)
    {
        _hasTile.reset();
        for(uint32 i=0; i<64; i++)
        {
            for(uint32 j=0; j<64; j++)
            {
                if(w->_main.tiles[i*64 + j])
                    _hasTile[i*64 + j] = true;
            }

        }
    }
    inline void SetTile(MapTile* tile, uint32 x, uint32 y) { SetTile(tile, y*64 + x); }
    inline void SetTile(MapTile* tile, uint32 pos)
    {
        if(pos < 4096)
            _tiles[pos] = tile;
    }
    inline void UnloadMapTile(uint32 x, uint32 y) { UnloadMapTile(y*64 + x); }
    inline void UnloadMapTile(uint32 pos)
    {
        if(pos < 4096 && _tiles[pos])
        {
            delete _tiles[pos];
            _tiles[pos] = NULL;
        }
    }
    inline bool TileExists(uint32 x, uint32 y) { return TileExists(y*64 + x); }
    inline bool TileExists(uint32 pos)
    {
        return pos < 4096 ? _hasTile[pos] : false;
    }
    inline MapTile *GetTile(uint32 x, uint32 y) { return GetTile(y*64 + x); }
    inline MapTile *GetTile(uint32 pos)
    {
        return pos < 4096 ? _tiles[pos] : NULL;
    }
    void _DebugDump(void);

private:
    MapTile *_tiles[4096]; //64x64
    std::bitset<4096> _hasTile;
};



#endif
