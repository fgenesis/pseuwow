#include "common.h"
#include "log.h"
#include "MapTile.h"
#include "MapMgr.h"

MapMgr::MapMgr()
{
    _tiles = new MapTileStorage();
    _gridx = _gridy = _mapid = (-1);
}

MapMgr::~MapMgr()
{
    Flush();
    delete _tiles;
}

void MapMgr::Update(float x, float y, uint32 m)
{
    if(m != _mapid)
    {
        Flush(); // we teleported to a new map, drop all loaded maps
        WDTFile *wdt = new WDTFile();
        char buf[100];
        sprintf(buf,"data/maps/%u.wdt",m);
        if(!wdt->Load(buf))
        {
            logerror("MAPMGR: Could not load WDT file '%s'",buf);
        }
        _tiles->ImportTileMap(wdt);
        delete wdt;
        _mapid = m;
        _gridx = _gridy = (-1); // must load tiles now
    }
    uint32 xg,yg; // MapTile IDs. Range 0..64
    xg = (uint32)( (ZEROPOINT - x) / TILESIZE);
    yg = (uint32)( (ZEROPOINT - y) / TILESIZE);
    if(xg != _gridx || yg != _gridy)
    {
        _LoadNearTiles(xg,yg,m);
        _gridx = xg;
        _gridy = yg;
        _UnloadOldTiles();
    }
    _mapid = m;
}

void MapMgr::Flush(void)
{
    for(uint32 i = 0; i < 4096; i++)
        _tiles->UnloadMapTile(i);
    logdebug("MAPMGR: Flushed all maps");
}

void MapMgr::_LoadNearTiles(uint32 gx, uint32 gy, uint32 m)
{
    logdebug("MAPMGR: Loading near tiles for (%u, %u) map %u",gx,gy,m);
    for(uint32 v = gy-1; v <= gy+1; v++)
    {
        for(uint32 h = gx-1; h <= gx+1; h++)
        {
            _LoadTile(h,v,m);
        }
    }
}

void MapMgr::_LoadTile(uint32 gx, uint32 gy, uint32 m)
{
    if(!_tiles->TileExists(gx,gy))
    {
        logerror("MAPMGR: Not loading MapTile (%u, %u) map %u, no entry in WDT tile map",gx,gy,m);
        return;
    }

    if( !_tiles->GetTile(gx,gy) )
    {
        ADTFile *adt = new ADTFile();
        char buf[300];
        sprintf(buf,"data/maps/%u_%u_%u.adt",m,gx,gy);
        if(adt->Load(buf))
        {
            logdebug("MAPMGR: Loaded ADT '%s'",buf);
            MapTile *tile = new MapTile();
            tile->ImportFromADT(adt);
            _tiles->SetTile(tile,gx,gy);
            logdebug("MAPMGR: Imported MapTile (%u, %u) for map %u",gx,gy,m);
        }
        else
        {
            logerror("MAPMGR: Loading ADT '%s' failed!",buf);
        }
        delete adt;
    }
    else
    {
        logdebug("MAPMGR: No need to load MapTile (%u, %u) map %u",gx,gy,m);
    }
}

void MapMgr::_UnloadOldTiles(void)
{
    for(uint32 gx=0; gx<64; gx++)
    {
        for(uint32 gy=0; gy<64; gy++)
        {
            if( (_gridx < gx-1 || _gridx > gx+1) && (_gridy < gy-1 || _gridy > gy+1) )
            {
                if(_tiles->GetTile(gx,gy))
                {
                    logdebug("MAPMGR: Unloading old MapTile (%u, %u) map %u",gx,gy,_mapid);
                    _tiles->UnloadMapTile(gx,gy);
                }
            }
        }
    }
}

float MapMgr::GetZ(float x, float y)
{
    uint32 xg,yg; // MapTile IDs. Range 0..64
    xg = (uint32)( (ZEROPOINT - x) / TILESIZE);
    yg = (uint32)( (ZEROPOINT - y) / TILESIZE);
    MapTile *tile = _tiles->GetTile(xg,yg);
    if(tile)
    {
#ifndef _DEBUG
        tile->DebugDumpToFile();
        logdebug("DEBUG: tile dumped");
#endif
        return tile->GetZ(x,y);
    }

    logerror("MapMgr::GetZ() called for not loaded MapTile (%u, %u) for (%f, %f)",xg,yg,x,y);
    return 0;
}



 
