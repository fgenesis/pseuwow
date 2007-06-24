#include <fstream>
#include "common.h"
#include "ADTFile.h"


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

bool ADTFile::Load(std::string fn)
{
    uint32 fs = GetFileSize(fn.c_str());
    if(!fs)
        return false;
    std::fstream fh;
    fh.open(fn.c_str(), std::ios_base::in | std::ios_base::binary);
    if(!fh.is_open())
        return false;

    ByteBuffer buf(fs);
    buf.resize(fs);
    fh.read((char*)buf.contents(),fs);
    fh.close();
    buf.rpos(0);
    return LoadMem(buf);
}

bool ADTFile::LoadMem(ByteBuffer& buf)
{
    uint32 texturecnt=0,modelcnt=0,wmocnt=0;
    uint32 size; // used for every chunk
    uint32 mcnkid=0;
    uint8 *fourcc = new uint8[5]; fourcc[4]=0;

    while(buf.rpos()<buf.size())
    {
        buf.read(fourcc,4); flipcc(fourcc); 
        buf.read((uint8*)&size,4);
        //DEBUG(printf("ADT: reading '%s' size %u\n",fourcc,size));

        if(!strcmp((char*)fourcc,"MVER"))
        {
            buf >> _version;
        }
        else if(!strcmp((char*)fourcc,"MHDR"))
        {
            mhdr = buf.read<MHDR_chunk>();
        }
        else if(!strcmp((char*)fourcc,"MCIN"))
        {
            for(uint32 i = 0; i < CHUNKS_PER_TILE; i++)
            {
                mcin[i] = buf.read<MCIN_chunk>();
                //DEBUG(printf("ADT chunk %u at offset %u, size %u flags %X async %u\n",i,mcin[i].offset,mcin[i].size,mcin[i].flags,mcin[i].async));
                if(!mcin[i].offset)
                {
                    printf("ADT: ERROR: chunk offset is NULL! Not loading.\n");
                    return false;
                }
            }
        }
        else if(!strcmp((char*)fourcc,"MTEX"))
        {
            for(uint32 i=0;;i++)
            {
                std::string tex;
                memcpy(fourcc,buf.contents()+buf.rpos(),4);
                flipcc(fourcc);
                if(!memcmp(fourcc,"MMDX",4))
                    break;
                buf >> tex;
                //DEBUG(printf("MTEX offset %u \"%s\"\n",buf.rpos(),tex.c_str()));
                _textures.push_back(tex);
                texturecnt++;
            }
            //DEBUG(printf("ADT: loaded %u textures\n",texturecnt));
        }
        else if(!strcmp((char*)fourcc,"MMDX"))
        {
            for(uint32 i=0;;i++)
            {
                std::string model;
                memcpy(fourcc,buf.contents()+buf.rpos(),4);
                flipcc(fourcc);
                if(!memcmp(fourcc,"MMID",4))
                    break;
                buf >> model;
                //DEBUG(printf("MMDX offset %u \"%s\"\n",buf.rpos(),model.c_str()));
                _models.push_back(model);
                modelcnt++;
            }
            //DEBUG(printf("ADT: loaded %u models\n",modelcnt));
        }
        /*else if(!strcmp((char*)fourcc,"MMID"))
        {
            for(uint32 i = 0; i <= modelcnt; i++)
            {
                uint32 offs;
                buf >> offs; // we dont really need those offsets
            }
        }*/
        else if(!strcmp((char*)fourcc,"MWMO"))
        {
            for(uint32 i=0;;i++)
            {
                std::string wmo;
                memcpy(fourcc,buf.contents()+buf.rpos(),4);
                flipcc(fourcc);
                if(!memcmp(fourcc,"MWID",4))
                    break;
                buf >> wmo;
                //DEBUG(printf("MWMO offset %u \"%s\"\n",buf.rpos(),wmo.c_str()));
                _wmos.push_back(wmo);
                wmocnt++;
            }
        }
        /*else if(!strcmp((char*)fourcc,"MWID"))
        {
            for(uint32 i = 0; i <= wmocnt; i++)
            {
                uint32 offs;
                buf >> offs; // we dont really need those offsets
            }
        }*/
        else if(!strcmp((char*)fourcc,"MDDF"))
        {
            uint32 ndoodads = size / 36;
            //DEBUG(printf("ADT: Loading %u doodads.\n",ndoodads));
            for(uint32 i = 0; i<ndoodads; i++)
            {
                _doodadsp.push_back(buf.read<MDDF_chunk>());
            }
        }
        else if(!strcmp((char*)fourcc,"MODF"))
        {
            uint32 nwmos = size / 64;
            //DEBUG(printf("ADT: Loading %u wmos.\n",nwmos));
            for(uint32 i = 0; i<nwmos; i++)
            {
                _wmosp.push_back(buf.read<MODF_chunk>());
            }
        }
        else if(!strcmp((char*)fourcc,"MCNK"))
        {
            _chunks[mcnkid].hdr = buf.read<ADTMapChunkHeader>();
            uint8 *mfcc = new uint8[5]; mfcc[4]=0;
            uint32 msize;
            while(buf.rpos()<buf.size())
            {
                buf.read(mfcc,4); flipcc(mfcc); 
                buf.read((uint8*)&msize,4);

                // HACKS to make it work properly
                if(!msize && !strcmp((char*)mfcc,"MCAL"))
                    continue;
                if((!msize) && !strcmp((char*)mfcc,"MCLQ"))
                    msize = _chunks[mcnkid].hdr.sizeLiquid;

                //DEBUG(printf("ADT: MCNK: reading '%s' size %u\n",mfcc,msize));

                if(!strcmp((char*)mfcc,"MCVT"))
                {
                    for(uint32 i = 0; i < 145; i++)
                    {
                        buf >>_chunks[mcnkid].vertices[i];
                    }
                }
                else if(!strcmp((char*)mfcc,"MCNR"))
                {
                    for(uint32 i = 0; i < 145; i++)
                    {
                        _chunks[mcnkid].normalvecs[i] = buf.read<NormalVector>();
                    }
                    // HACK: skip unk junk bytes
                    if(msize==0x1B3)
                        buf.rpos(buf.rpos()+0xD);
                }
                else if(!strcmp((char*)mfcc,"MCLY"))
                {
                    _chunks[mcnkid].nTextures = msize / 16;
                    ASSERT(msize/16 == _chunks[mcnkid].hdr.nLayers);
                    for(uint32 i = 0; i < _chunks[mcnkid].nTextures; i++)
                    {
                        _chunks[mcnkid].layer[i] = buf.read<MCLY_chunk>();
                    }
                }
                else if(!strcmp((char*)mfcc,"MCSH"))
                {
                    buf.read((uint8*)&(_chunks[mcnkid].shadowmap),512);
                }
                else if(!strcmp((char*)mfcc,"MCAL"))
                {
                    for(uint32 i = 0; i < (_chunks[mcnkid].hdr.sizeAlpha - 8) / 2048; i++)
                    {
                        buf.read((uint8*)(_chunks[mcnkid].alphamap[i]),2048);
                    }
                }
                else if(!strcmp((char*)mfcc,"MCLQ"))
                {
                    uint8 *fcc1 = new uint8[5];
                    buf.read(fcc1,4);
                    flipcc(fcc1);
                    fcc1[4]=0;
                    if (!strcmp((char*)fcc1,"MCSE"))
                    {
                        _chunks[mcnkid].haswater = false;
                        //DEBUG(printf("ADT: MCNK: MCLQ not present\n"));
                        buf.rpos(buf.rpos()-4);
                        delete [] fcc1;
                        continue; // next block read will be the MCSE block
                    }
                    else
                    {
                        _chunks[mcnkid].haswater = true;
                        float tmp;
                        buf.rpos(buf.rpos()-4);
                        uint32 bufpos=buf.rpos();
                        uint32 rbytes,diffbytes;
                        buf >> _chunks[mcnkid].waterlevel;
                        buf >> tmp;
                        //DEBUG(printf("ADT: MCNK: MCLQ base floats: %f %f\n",_chunks[mcnkid].waterlevel,tmp));
                        //buf.rpos(buf.rpos()+4); // base height??
                        if(msize > 8) // just to be sure
                        {
                            for(uint32 i = 0; i < 81; i++)
                            {
                                _chunks[mcnkid].lqvertex[i] = buf.read<LiquidVertex>();
                            }
                            for(uint32 i = 0; i < 64; i++)
                            {
                                buf >> _chunks[mcnkid].lqflags[i];
                            }
                            rbytes = buf.rpos() - bufpos;
                            //DEBUG(printf("ADT: MCNK: MCLQ block loaded. %u / %u bytes.\n",rbytes,msize));
                        }
                        else
                        {
                            //DEBUG(printf("ADT: MCNK: MCLQ block has only %u bytes\n",msize));
                        }
                        // HACK: skip some unk junk bytes
                        diffbytes = (msize-8) - rbytes; // dont forget to skip the 8 initial bytes
                        buf.rpos(buf.rpos()+diffbytes);
                        //DEBUG(printf("ADT: MCNK: MCLQ - %u junk bytes skipped\n",diffbytes));
                        delete [] fcc1;
                    }
                }
                else if(!strcmp((char*)mfcc,"MCSE"))
                {
                    uint32 emm = _chunks[mcnkid].hdr.nSndEmitters;
                    for(uint32 i = 0; i < emm; i++)
                    {
                        _soundemm.push_back(buf.read<MCSE_chunk>());
                    }
                    break;
                }
                else
                {
                    //DEBUG(printf("ADT: MCNK: '%s' block unhandled, skipping %u bytes\n",mfcc,msize));
                    if(!(isalnum(mfcc[0]) && isalnum(mfcc[1]) && isalnum(mfcc[2]) && isalnum(mfcc[3])))
                    {
                        printf("Error loading ADT file (chunk %u error).\n",mcnkid);
                        return false; // dont care about those few mem leaks
                    }
                        
                    buf.rpos(buf.rpos()+msize);
                }

            }
            delete [] mfcc;
            mcnkid++;
        }
        else
        {
            //DEBUG(printf("ADT: '%s' block unhandled, skipping %u bytes\n",fourcc,size));
            if(!(isalnum(fourcc[0]) && isalnum(fourcc[1]) && isalnum(fourcc[2]) && isalnum(fourcc[3])))
            {
                printf("Error loading ADT file.\n");
                return false; // dont care about those few mem leaks
            }
            buf.rpos(buf.rpos()+size);
        }

    }
    delete [] fourcc;
    return true;
}






void ADT_ExportStringSetByOffset(const uint8* data, uint32 off, std::set<std::string>& st, char* stop)
{
    data += ((uint32*)data)[off]; // seek to correct absolute offset
    data += 28; // move ptr to real start of data
    uint32 offset=0;
    std::string s;
    char c;
    while(memcmp(data+offset,stop,4))
    {
        c = data[offset];
        if(!c)
        {
            if(s.length())
            {
                DEBUG(printf("DEP: %s\n",s.c_str()));
                st.insert(s);
                s.clear();
            }
        }
        else
            s += c;
        offset++;
    }
}

void ADT_FillTextureData(const uint8* data,std::set<std::string>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_TEXTURES,st,"XDMM");
}

void ADT_FillWMOData(const uint8* data,std::set<std::string>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_WMOS,st,"DIWM");
}

void ADT_FillModelData(const uint8* data,std::set<std::string>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_MODELS,st,"DIMM");
}
