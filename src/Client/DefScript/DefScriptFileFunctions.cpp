#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include "DefScript.h"
#include "DefScriptTools.h"
#include "SysDefs.h"

using namespace DefScriptTools;

#define FH_MACRO_READ_I(_f,_sty,_ty) if(_sty==(#_ty)) { _ty _in; (_f)->read((char*)&_in,sizeof(_ty)); return toString((uint64)_in); }
#define FH_MACRO_READ_F(_f,_sty,_ty) if(_sty==(#_ty)) { _ty _in; (_f)->read((char*)&_in,sizeof(_ty)); return toString((ldbl)_in); }

DefReturnResult DefScriptPackage::func_fopen(CmdSet& Set)
{
    std::fstream *fh = files.Get(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh->is_open())
        return "";
    std::string mode = stringToLower(Set.arg[1]);
    std::ios::openmode m = (std::ios::openmode)(0);
    if(mode.find('b') != std::string::npos)
        m |= std::ios_base::binary;
    if(mode.find('a') != std::string::npos)
        m |= std::ios_base::app;
    if(mode.find('r') != std::string::npos)
        m |= std::ios_base::in;
    if(mode.find('w') != std::string::npos)
        m |= std::ios_base::out;

    // sort out some possible errors with bad open modes
    if(m & std::ios_base::app)
        m |= std::ios_base::out; // always open for writing when appending
    if(m == 0 || m == std::ios_base::binary)
        m |= std::ios_base::out | std::ios_base::in; // if no openmode or binary only specified, use both r+w additionally

    fh->open(Set.defaultarg.c_str(), m);

    // if the file didnt open properly, like if a not existing file opened in read+write mode,
    // try fallback mode with write mode only
    if(!fh->is_open() && (m & std::ios_base::in))
    {
        m &= (~std::ios_base::in);
        fh->open(Set.defaultarg.c_str(), m);
    }
    _DEFSC_DEBUG( if(!fh->is_open()) printf("DefScript: Can't open file '%s'\n",Set.defaultarg.c_str()));
    return fh->is_open();
}

DefReturnResult DefScriptPackage::func_fclose(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(fh)
    {
        fh->flush();
        fh->close();
        files.DeleteByPtr(fh);
        return true;
    }
    return false;
}

DefReturnResult DefScriptPackage::func_fisopen(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    return fh && fh->good() && fh->is_open();
}

DefReturnResult DefScriptPackage::func_feof(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    return (!fh) || fh->eof();
}

DefReturnResult DefScriptPackage::func_frpos(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(!fh)
        return "";
    return toString((uint64)fh->tellg());
}

DefReturnResult DefScriptPackage::func_fwpos(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(!fh)
        return "";
    return toString((uint64)fh->tellp());
}

DefReturnResult DefScriptPackage::func_fdel(CmdSet& Set)
{
    return (remove(Set.defaultarg.c_str()) == 0);
}

DefReturnResult DefScriptPackage::func_fflush(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(fh && fh->is_open())
        fh->flush();
    return "";
}

DefReturnResult DefScriptPackage::func_fwrite(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        *fh << Set.defaultarg;
        return true;
    }
    return false;
}

DefReturnResult DefScriptPackage::func_fread(CmdSet& Set)
{
    std::string fn = _NormalizeVarName(Set.arg[0],Set.myname);
    std::fstream *fh = files.GetNoCreate(fn);
    if(fh && fh->is_open())
    {
        uint64 bytes;
        uint64 read = 0;
        if(stringToLower(Set.defaultarg) == "all")
        {
            std::fstream::pos_type begin_pos, end_pos, old_pos = fh->tellg();
            fh->seekg(0, std::ios_base::beg);
            begin_pos=fh->tellg();
            fh->seekg(0, std::ios_base::end);
            end_pos = fh->tellg();
            fh->seekg(old_pos, std::ios_base::beg);
            bytes = (end_pos - begin_pos);
        }
        else
            bytes = toUint64(Set.defaultarg);
        std::string ret;
        ret.reserve(bytes);
        for(uint64 i = 0; i < bytes && !fh->eof(); i++)
        {
            ret += fh->get();
            read++;
        }
        return toString(read);
    }
    return "";
}

DefReturnResult DefScriptPackage::func_freadb(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        std::string t = stringToLower(Set.defaultarg);
        if(t == "string")
        {
            std::string ret;
            while(char c = fh->get() && !fh->eof())
                ret += c;
            return ret;
        }
        else if(t == "strnz")
        {
            uint32 bytes = (uint32)toUint64(Set.arg[1]);
            if(bytes)
            {
                std::string g;
                for(uint32 i = 0; i < bytes; i++)
                {
                    g += fh->get();
                }
                return g;
            }
            return "";
        }
        FH_MACRO_READ_I(fh,t,uint8);
        FH_MACRO_READ_I(fh,t,uint16);
        FH_MACRO_READ_I(fh,t,uint32);
        FH_MACRO_READ_I(fh,t,uint64);
        FH_MACRO_READ_F(fh,t,float);
        FH_MACRO_READ_F(fh,t,double);
    }
    return "";
}

DefReturnResult DefScriptPackage::func_fwriteb(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        std::string t = stringToLower(Set.arg[1]);
        if(t == "string")
        {
            fh->write(Set.defaultarg.c_str(), Set.defaultarg.size() + 1); // also put \0
            return true;
        }
        else if(t == "strnz")
        {
            fh->write(Set.defaultarg.c_str(), Set.defaultarg.size()); // skip \0
            return true;
        }
        else if(t == "uint8")
        {
            uint8 o = (uint8)toUint64(Set.defaultarg);
            fh->write((char*)&o,sizeof(uint8));
            return true;
        }
        else if(t == "uint16")
        {
            uint16 o = (uint16)toUint64(Set.defaultarg);
            fh->write((char*)&o,sizeof(uint16));
            return true;
        }
        else if(t == "uint32")
        {
            uint32 o = (uint32)toUint64(Set.defaultarg);
            fh->write((char*)&o,sizeof(uint32));
            return true;
        }
        else if(t == "uint64")
        {
            uint64 o = toUint64(Set.defaultarg);
            fh->write((char*)&o,sizeof(uint64));
            return true;
        }
        else if(t == "float")
        {
            float o = (float)toNumber(Set.defaultarg);
            fh->write((char*)&o,sizeof(float));
            return true;
        }
        else if(t == "double")
        {
            double o = (double)toNumber(Set.defaultarg);
            fh->write((char*)&o,sizeof(double));
            return true;
        }

    }
    return false;
}

DefReturnResult DefScriptPackage::func_fwritebb(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
        if(bb)
        {
            uint32 bytes = (uint32)toUint64(Set.arg[1]);
            if(!bytes)
            {
                bytes = bb->size();
            }
            if(bytes)
            {
                fh->write((char*)bb->contents(), bytes);
            }
            return toString((uint64)bytes);
        }
    }
    return "";
}

DefReturnResult DefScriptPackage::func_freadbb(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        ByteBuffer *bb = bytebuffers.Get(_NormalizeVarName(Set.defaultarg,Set.myname));
        uint32 bytes = (uint32)toUint64(Set.arg[1]);
        if(!bytes)
        {
            // get amount of remaining bytes
            std::ios::pos_type oldpos = fh->tellg();
            fh->seekg(0,std::ios_base::end);
            bytes = uint32(fh->tellg() - oldpos);
            fh->seekg(oldpos,std::ios_base::beg);
        }
        if(bytes)
        {
            bb->resize(bb->size() + bytes);
            fh->read((char*)bb->contents(), bytes);
        }
        return toString((uint64)bytes);
    }
    return "";
}

DefReturnResult DefScriptPackage::func_fsize(CmdSet& Set)
{
    std::ifstream f;
    f.open(Set.defaultarg.c_str(), std::ios_base::binary | std::ios_base::in);
    if (!f.good() || f.eof() || !f.is_open()) { return ""; }
    f.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = f.tellg(), end_pos;
    f.seekg(0, std::ios_base::end);
    end_pos = f.tellg();
    f.close();
    return toString((uint64)(end_pos - begin_pos));
}

DefReturnResult DefScriptPackage::func_freadline(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(fh && fh->is_open())
    {
        char c;
        std::string line;
        line.reserve(30); // rough guess to speedup appending
        while(!fh->eof())
        {
            c = fh->get();
            if(c == '\n' || fh->eof())
                return line;
            else
                line += c;
        }
    }
    return "";
}

DefReturnResult DefScriptPackage::func_fseekw(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        std::string w = stringToLower(Set.arg[1]);
        uint32 pos = (uint32)toUint64(Set.defaultarg);
        if(w.empty() || w == "begin")
        {
            fh->seekp(pos, std::ios_base::beg);
            return true;
        }
        else if(w == "end")
        {
            fh->seekp(pos, std::ios_base::end);
            return true;
        }
    }
    return false;
}

DefReturnResult DefScriptPackage::func_fseekr(CmdSet& Set)
{
    std::fstream *fh = files.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(fh && fh->is_open())
    {
        std::string w = stringToLower(Set.arg[1]);
        uint32 pos = (uint32)toUint64(Set.defaultarg);
        if(w.empty() || w == "begin")
        {
            fh->seekg(pos, std::ios_base::beg);
            return true;
        }
        else if(w == "end")
        {
            fh->seekg(pos, std::ios_base::end);
            return true;
        }
    }
    return false;
}
