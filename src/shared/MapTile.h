#ifndef MAPTILE_H
#define MAPTILE_H

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
    float xbase,ybase,zbase;
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

private:
    MapChunk _chunks[256]; // 16x16
    std::vector<std::string> _textures;
    std::vector<std::string> _wmos;
    std::vector<std::string> _models;

    float _xbase,_ybase;

};

// store which map tiles are present in the world
class MapTileStorage
{
public:
    inline void ImportTileMap(WDTFile* w)
    {
        for(uint32 i=0; i<64; i++)
            for(uint32 j=0; j<64; j++)
                if(w->_main.tiles[i*64 + j])
                    _hasTile[i] &= (1 << j);
    }
    inline void SetTile(MapTile* tile, uint32 x, uint32 y) { SetTile(tile, y*64 + x); }
    inline void SetTile(MapTile* tile, uint32 pos)
    {
        _tiles[pos] = tile;
    }
    inline void UnloadMapTile(uint32 x, uint32 y) { UnloadMapTile(y*64 + x); }
    inline void UnloadMapTile(uint32 pos)
    {
        delete _tiles[pos];
        _tiles[pos] = NULL;
    }
    inline void TileExists(uint32 x, uint32 y) { TileExists(y*64 + x); }
    inline bool TileExists(uint32 pos)
    {
        return _hasTile[pos/64] & (1<<(pos%64));
    }
    inline MapTile *GetTile(uint32 x, uint32 y) { GetTile(y*64 + x); }
    inline MapTile *GetTile(uint32 pos)
    {
        return _tiles[pos];
    }
private:
    MapTile *_tiles[4096]; //64x64
    uint64 _hasTile[64]; //64 x 64 bits, this saves some mem compared to bluzz format
};



#endif
