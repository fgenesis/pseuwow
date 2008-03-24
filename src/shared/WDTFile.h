#ifndef WDTFILE_H
#define WDTFILE_H

struct WDT_MVER_Chunk
{
    uint32 ver;
};

struct WDT_MPHD_Chunk
{
    uint32 noTerrain;
    uint32 unk2;
    uint32 unk3;
    uint32 unk4;
    uint32 unk5;
    uint32 unk6;
    uint32 unk7;
    uint32 unk8;
};

struct WDT_MAIN_Chunk
{
    uint64 tiles[4096]; //64x64
};


class WDTFile
{
public:
    bool Load(std::string);
    bool LoadMem(ByteBuffer&);
    void _DebugDump(void);
    WDT_MVER_Chunk _mver;
    WDT_MPHD_Chunk _mphd;
    WDT_MAIN_Chunk _main;
    // TODO: implement support for MWMO & MODF chunks (see ADT)
};





#endif
