#include "common.h"
#include "MapTile.h"


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
        _chunks[ch].xbase = adt->_chunks[ch].hdr.xbase;
        _chunks[ch].ybase = adt->_chunks[ch].hdr.ybase;
        _chunks[ch].zbase = adt->_chunks[ch].hdr.zbase;
        uint32 fcnt=0, rcnt=0;
        while(fcnt+rcnt < 145) //9*9 + 8*8
        {
            for(uint32 h=0; h<9; h++)
            {
                rcnt++;
                _chunks[ch].hmap_rough[rcnt] = adt->_chunks[ch].vertices[fcnt+rcnt];
            }
            for(uint32 h=0; h<8; h++)
            {
                fcnt++;
                _chunks[ch].hmap_fine[rcnt] = adt->_chunks[ch].vertices[fcnt+rcnt];
            }
        }
    }
}