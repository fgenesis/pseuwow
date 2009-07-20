#include "common.h"
#include "MapTile.h"
#include "log.h"

MapTile::MapTile()
{
}

MapTile::~MapTile()
{
}

void MapTile::ImportFromADT(ADTFile *adt)
{
    // strip the path name from the dependency files, just store the plain filename
    for(std::vector<std::string>::iterator it = adt->_textures.begin(); it != adt->_textures.end(); it++)
        this->_textures.push_back(_PathToFileName(*it));
    for(std::vector<std::string>::iterator it = adt->_models.begin(); it != adt->_models.end(); it++)
        this->_models.push_back(_PathToFileName(*it));
    for(std::vector<std::string>::iterator it = adt->_wmos.begin(); it != adt->_wmos.end(); it++)
        this->_wmos.push_back(_PathToFileName(*it));

    // import the height map
    for(uint32 ch=0; ch<CHUNKS_PER_TILE; ch++)
    {
        _chunks[ch].baseheight = adt->_chunks[ch].hdr.zbase; // ADT files store (x/z) as ground coords and (y) as the height!
        _chunks[ch].basex = adt->_chunks[ch].hdr.xbase; // here converting it to (x/y) on ground and basehight as actual height.
        _chunks[ch].basey = adt->_chunks[ch].hdr.ybase; // strange coords they use... :S
        _chunks[ch].lqheight = adt->_chunks[ch].waterlevel;
        // extract heightmap
        uint32 fcnt=0, rcnt=0;
        while(true) //9*9 + 8*8
        {
            for(uint32 h=0; h<9; h++)
            {
                _chunks[ch].hmap_rough[rcnt] = adt->_chunks[ch].vertices[fcnt+rcnt];
                rcnt++;
            }
            if(rcnt+fcnt >= 145)
                break;
            for(uint32 h=0; h<8; h++)
            {
                _chunks[ch].hmap_fine[fcnt] = adt->_chunks[ch].vertices[fcnt+rcnt];
                fcnt++;
            }
        }
        // extract water heightmap
        for(uint32 i = 0; i < 81; i++)
        {
            _chunks[ch].hmap_lq[i] = adt->_chunks[ch].lqvertex[i].h;
        }
        // extract map layers with texture filenames
        for(uint32 ly = 0; ly < adt->_chunks[ch].hdr.nLayers; ly++)
        {
            uint32 texoffs = adt->_chunks[ch].layer[ly].textureId;
            _chunks[ch].texlayer.push_back(std::string("data/texture/") + NormalizeFilename(std::string(adt->_textures[texoffs])).c_str());
        }

        memcpy(_chunks[ch].alphamap, adt->_chunks[ch].alphamap, adt->_chunks[ch].hdr.sizeAlpha - 8);

        /*
        // extract alpha maps. in adt they are stored in 4-bit encoding, which makes 4096 entries in 64x32 values
        for(uint32 al = 0; al < (adt->_chunks[ch].hdr.sizeAlpha - 8) / 2048; al++) // see comment in ADTFile.cpp when loading MCAL chunk for explanation
        {
            for(uint32 aly = 0; aly < 64; aly++)
            {
                for(uint32 alx = 0; alx < 32; alx++)
                {
                    _chunks[ch].alphamap[al][aly*64 + (alx*2)]   = adt->_chunks[ch].alphamap[al][aly*64 + alx] & 0xF0; // first 4 bits
                    _chunks[ch].alphamap[al][aly*64 + (alx*2)+1] = adt->_chunks[ch].alphamap[al][aly*64 + alx] & 0x0F; // second
                }
            }
        }
        */
    }

    // copy over doodads and do some transformations
    DEBUG(logdebug("%u doodads", adt->_doodadsp.size()));
    for(uint32 i = 0; i < adt->_doodadsp.size(); i++)
    {
        Doodad d;
        MDDF_chunk& mddf = adt->_doodadsp[i];
        // and yet another coordinate system...
        d.y = -(mddf.x - ZEROPOINT);
        d.z = mddf.y;
        d.x = -(mddf.z - ZEROPOINT);
        d.ox = mddf.c;
        d.oy = mddf.b;
        d.oz = mddf.a;
        d.flags = mddf.flags;
        d.uniqueid = mddf.uniqueid;
        d.model = std::string("./data/model/") + NormalizeFilename(_PathToFileName(adt->_models[mddf.id]));
        // this .mdx -> .m2 transformation is annoying >.< - replace "mdx" and end of string with "m2"
        // d.model = d.model.substr(0, d.model.length() - 3) + "m2";
        // 3.1.3 - no more .mdx in ADT 
        d.scale = mddf.scale / 1024.0f;
        if(d.scale < 0.00001f)
            d.scale = 1;
        _doodads.push_back(d);
    }

    // copy over wmos and do some transformations
    DEBUG(logdebug("%u wmos", adt->_wmosp.size()));
    for(uint32 i = 0; i < adt->_wmosp.size(); i++)
    {
        WorldMapObject wmo;
        MODF_chunk& modf = adt->_wmosp[i];
        wmo.y = -(modf.x - ZEROPOINT);
        wmo.z = modf.y;
        wmo.x = -(modf.z - ZEROPOINT);
        wmo.ox = modf.ox;
        wmo.oy = modf.oy;
        wmo.oz = modf.oz;
        wmo.flags = modf.flags;
        wmo.uniqueid = modf.uniqueid;
        wmo.model = std::string("./data/wmo/") + NormalizeFilename(_PathToFileName(adt->_wmos[modf.id]));
        _wmo_data.push_back(wmo);
    }

    // copy sound emitters
    _soundemm = adt->_soundemm;

    _xbase = _chunks[0].basex;
    _ybase = _chunks[0].basey;
    _hbase = _chunks[0].baseheight;

    DEBUG(logdebug("MapTile first chunk base: h=%f x=%f y=%f",_hbase,_xbase,_ybase));
}

void MapTileStorage::_DebugDump(void)
{
    std::string out;
    for(uint32 i=0; i<64; i++)
    {
        for(uint32 j=0; j<64; j++)
        {

            out += (_hasTile[i*64 + j] ? "1" : "0");
        }
        out += "\n";
    }
    printf("MAP TILE MAP DEBUG DUMP, 64x64 TILES:\n");
    printf(out.c_str());
}

// get approx Z position for world position (x,y).
float MapTile::GetZ(float x, float y)
{
    float bx,by;
    float real_z;
    bx = _chunks[0].basex; // world base coords of tile
    by = _chunks[0].basey;
    uint32 chx = (uint32)fabs((bx - x) / CHUNKSIZE); // get chunk id for given coords
    uint32 chy = (uint32)fabs((by - y) / CHUNKSIZE);
    if( chx > 15 || chy > 15)
    {
        logerror("MapTile::GetZ() wrong chunk indexes (%d, %d) for (%f, %f)",chx,chy,x,y);
        logerror(" - These coords are NOT on this tile!");
        return INVALID_HEIGHT;
    }
    MapChunk& ch = _chunks[chx*16 + chy];
    uint32 vx,vy; // get vertex position (0,0) ... (8,8);
    vy = (uint32)floor((fabs(ch.basey - y) / (CHUNKSIZE/16.0f)) + 0.5f);
    if (vy % 2 == 0)
    {
        vx = (uint32)floor((fabs(ch.basex - x) / (CHUNKSIZE/8.0f)) + 0.5f);
        real_z = ch.hmap_rough[vx*9 + (vy/2)] + ch.baseheight;
    }
    else if (vy % 2 != 0)
    {
        vx = (uint32)floor((fabs(ch.basex - x) / (CHUNKSIZE/7.0f)) ); //edit: removed + 0.5f
        real_z = ch.hmap_fine[vx*8 + ((vy-1)/2)] + ch.baseheight;
    }
    if(vx > 8 || vy > 17)
    {
        logerror("MapTile::GetZ() wrong vertex indexes (%d, %d) for chunk (%d, %d) for (%f, %f) chunkpos (%f, %f) tile (%f, %f)",vx,vy,chx,chy,x,y,ch.basex,ch.basey,bx,by);
        return INVALID_HEIGHT;
    }

    return real_z;
}

void MapTile::DebugDumpToFile(void)
{
    const char *f = "0123456789abcdefghijklmnopqrstuvwxyz";
    float z;
    uint32 p;
    std::string out;
    for(uint32 cy=0;cy<16;cy++)
    {
        for(uint32 vy=0;vy<9;vy++)
        {
            for(uint32 cx=0;cx<16;cx++)
            {
                for(uint32 vx=0;vx<9;vx++)
                {
                    z = _chunks[cy*16 + cx].hmap_rough[vy*9 + vx] + _chunks[cy*16 + cx].baseheight;
                    p = (uint32)z;
                    uint32 pos = 17 + (p/10);
                    if(pos > strlen(f)-1)
                        pos=strlen(f)-1;
                    char c = f[pos];
                    out += c;
                }
            }
            out += "\n";
        }
    }
    FILE *fh;
    fh = fopen("map_dump.txt","w");
    fprintf(fh, out.c_str());
    fclose(fh);
}

