// all credit for this file goes to the guys who write the wiki article about ADT files @ wowdev.org

#ifndef ADTFILESTRUCTS_H
#define ADTFILESTRUCTS_H

#define OFFSET_TEXTURES 7
#define OFFSET_MODELS 8
#define OFFSET_WMOS 10

#define ADT_MAXLAYERS 4

struct MHDR_chunk
{
    uint32 pad;
    uint32 offsInfo;		
    uint32 offsTex;		
    uint32 offsModels;		
    uint32 offsModelsIds;		
    uint32 offsMapObjects;		
    uint32 offsMapObjectsIds;		
    uint32 offsDoodsDef;		
    uint32 offsObjectsDef;	
    uint32 pad1;	
    uint32 pad2;		
    uint32 pad3;	
    uint32 pad4;		
    uint32 pad5;		
    uint32 pad6;		
    uint32 pad7;
};

struct MCIN_chunk
{
    uint32 offset;
    uint32 size;
    uint32 flags;
    uint32 async;    
};

// MTEX: texture filename list. strings only!

// MMDX: M2 models filename list. strings only!

// MMID: string start offset list of the MMDX block

// MWMO: filename list for WMOs

// MWID: string start offset list of the MWMO block

struct MDDF_chunk
{
    uint32 id; // position in the MMDX list
    uint32 uniqueid; // unique instance id
    float x; // position (quaternion)
    float y;
    float z;
    float a; // rotation
    float b;
    float c;
    uint16 scale;
    uint16 flags;
};

struct MODF_chunk
{
    uint32 id;
    uint32 uniqueid;
    float x;
    float y;
    float z;
    float ox;
    float oy;
    float oz;
    // unk floats (orientation?)
    float ou11;
    float ou12;
    float ou13;
    float ou21;
    float ou22;
    float ou23;
    uint32 flags;
    uint16 doodadSet;
    uint16 nameSet;
};

enum ADTMapChunkHeaderFlags
{
    FLAG_SHADOW,
    FLAG_IMPASS,
    FLAG_LQ_RIVER,
    FLAG_LQ_OCEAN,
    FLAG_LQ_MAGMA,
};

struct ADTMapChunkHeader
{
    uint32 flags;
    uint32 IndexX;
    uint32 IndexY;
    uint32 nLayers;
    uint32 nDoodadRefs;
    uint32 offsHeight;
    uint32 offsNormal;
    uint32 offsLayer;
    uint32 offsRefs;
    uint32 offsAlpha;
    uint32 sizeAlpha;
    uint32 offsShadow;
    uint32 sizeShadow;
    uint32 areaid;
    uint32 nMapObjRefs;
    uint32 holes;
    uint16 unk1;
    uint16 unk2;
    uint32 unk3;
    uint32 unk4;
    uint32 unk5;
    uint32 predTex;
    uint32 noEffectDoodad;
    uint32 offsSndEmitters;
    uint32 nSndEmitters;
    uint32 offsLiquid;
    uint32 sizeLiquid; // includes the 8 initinal bytes ("MCLQ" and size)
    float xbase;
    float ybase;
    float zbase; 
    uint32 textureId;
    uint32 props;
    uint32 effectId;  
};

// MCNK sub-chunk
// 9x9+8x8 vertices. 145 floats, format: 9 outer, 8 inner, 9 outer, 8 inner, ... , 9 outer.

// MCNR chunk: Normal vectors for each vertex, encoded as 3 signed bytes per normal, in the same order as specified above.

struct MCLY_chunk
{
    uint32 textureId; // offset in MTEX list
    uint32 flags; // 0x100 means using alpha map
    uint32 offAlpha;
    uint32 effectId; //detail texture id (?)
};

// MCRF chunk: 
// A list of indices into the parent file's MDDF chunk,
// saying which MCNK subchunk those particular MDDF doodads are drawn within.
// This MCRF list contains duplicates for map doodads that overlap areas.
// But I guess that's what the MDDF's UniqueID field is for.

// MCSH chunk: 8x8 bytes (64x64 bits) shadow map





struct MCSE_chunk
{
    uint32 soundPointID;		
    float x;	
    float y;   		
    float z; 	

    // TODO: Find real structure, fields left for references in code
    uint32 soundNameID;		
    float minDistance;	 		
    float maxDistance;	
    /*float cutoffDistance;		
    uint16 startTime;	
    uint16 endTime;
    uint16 groupSilenceMin;		
    uint16 groupSilenceMax; 	
    uint16 playInstancesMin;
    uint16 playInstancesMax;	
    uint16 loopCountMin;
    uint16 loopCountMax;
    uint16 interSoundGapMin;
    uint16 interSoundGapMax;*/
};

struct LiquidVertex
{
    uint16 unk1;
    uint16 unk2;
    float h;
};

struct NormalVector
{
    uint8 x;
    uint8 y;
    uint8 z;
};

// also known as MCNK block
// 256 per adt file
struct ADTMapChunk
{
    ADTMapChunkHeader hdr;
    float vertices[145];
    NormalVector normalvecs[145];
    MCLY_chunk layer[ADT_MAXLAYERS]; // can be less
    uint32 nTextures;
    uint8 shadowmap[512]; // 1 bit 64x64
    uint8 alphamap[ADT_MAXLAYERS][4096]; // 8 bits, 64x64. max 4, 1 per layer
    bool haswater;
    float waterlevel;
    LiquidVertex lqvertex[81];
    uint8 lqflags[64];




};

#endif
