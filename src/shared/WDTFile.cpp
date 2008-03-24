#include <fstream>
#include "common.h"
#include "ByteBuffer.h"
#include "WDTFile.h"

inline void flipcc(uint8 *fcc)
{
    char t;
    t=fcc[0];
    fcc[0]=fcc[3];
    fcc[3]=t;
    t=fcc[1];
    fcc[1]=fcc[2];
    fcc[2]=t;
}

bool WDTFile::Load(std::string fn)
{
    std::fstream fh;
    fh.open(fn.c_str(),std::ios_base::in | std::ios_base::binary);
    if(!fh.is_open())
        return false;
    uint32 fs = GetFileSize(fn.c_str());
    ByteBuffer bb(fs);
    bb.resize(fs);
    fh.read((char*)bb.contents(),fs);
    fh.close();
    bb.rpos(0);
    return LoadMem(bb);
}

bool WDTFile::LoadMem(ByteBuffer& buf)
{
    uint8 *fourcc = new uint8[5];
    fourcc[4] = 0;
    uint32 size;

    while(buf.rpos() < buf.size())
    {
        buf.read(fourcc,4); flipcc(fourcc); 
        buf >> size;

        if(!strcmp((char*)fourcc,"MVER"))
        {
            _mver = buf.read<WDT_MVER_Chunk>();
        }
        else if(!strcmp((char*)fourcc,"MPHD"))
        {
            _mphd = buf.read<WDT_MPHD_Chunk>();
        }
        else if(!strcmp((char*)fourcc,"MAIN"))
        {
            _main = buf.read<WDT_MAIN_Chunk>();
        }
        else if(!strcmp((char*)fourcc,"MWMO"))
        {
            if(size)
            {
                DEBUG(printf("WDTFile::LoadMem() abort load, MWMO block isnt empty\n"));
                break;
            }  
        }
        else if(!strcmp((char*)fourcc,"MODF"))
        {
            DEBUG(printf("WDTFile::LoadMem() abort load, MODF block exists\n"));
            break;
        }
    }
    delete [] fourcc;
    return true;
}

void WDTFile::_DebugDump(void)
{
    std::string out;
    for(uint32 i=0; i<64; i++)
    {
        for(uint32 j=0; j<64; j++)
        {
            out += (_main.tiles[i*64+j] ? "1" : "0");
        }
        out += "\n";
    }
    printf("WDT DEBUG DUMP, 64x64 TILES:\n");
    printf(out.c_str());
}

