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
        _chunks[ch].basey = adt->_chunks[ch].hdr.xbase; // here converting it to (x/y) on ground and basehight as actual height.
        _chunks[ch].basex = adt->_chunks[ch].hdr.ybase; // strange coords they use... :S
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
    }
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
// TODO: use inner vertices also
// TODO: interpolate values instead of choosing closest vertex
// formula taken from BoogieBot, thx!
float MapTile::GetZ(float x, float y)
{
    float bx,by;
    bx = _chunks[0].basex; // world base coords of tile
    by = _chunks[0].basey;
    uint32 chx = (uint32)fabs((bx - x) / CHUNKSIZE); // get chunk id for given coords
    uint32 chy = (uint32)fabs((by - y) / CHUNKSIZE);
    if( chx > 15 || chy > 15)
    {
        logerror("MapTile::GetZ() wrong chunk indexes (%d, %d) for (%f, %f)",chx,chy,x,y);
        logerror(" - These coords are NOT on this tile!");
        return 0;
    }
    MapChunk& ch = _chunks[chy*16 + chx];
    uint32 vx,vy; // get vertex position (0,0) ... (8,8);
    vx = (uint32)floor((fabs(ch.basex - x) / UNITSIZE) + 0.5f);
    vy = (uint32)floor((fabs(ch.basey - y) / UNITSIZE) + 0.5f);
    if(vx > 8 || vy > 8)
    {
        logerror("MapTile::GetZ() wrong vertex indexes (%d, %d) for chunk (%d, %d) for (%f, %f)",vx,vy,chx,chy,x,y);
        return 0;
    }

    float real_z = ch.hmap_rough[vy*9 + vx] + ch.baseheight;

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
