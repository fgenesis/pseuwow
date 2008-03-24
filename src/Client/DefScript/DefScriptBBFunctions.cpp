#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include "DefScript.h"
#include "DefScriptTools.h"
#include "SysDefs.h"
#include "ByteBuffer.h"

using namespace DefScriptTools;

// Helper macros for insert/extract various datatypes that need type casting
#define BB_CAN_READ(bytebuffer, _ty) ((*(bytebuffer)).size() - (*(bytebuffer)).rpos() >= sizeof(_ty))
#define BB_MACRO_INSERT_I(bytebuffer, _sty, _ty) if( (_sty)==(#_ty) ) { *(bytebuffer) << (_ty)toUint64(Set.defaultarg); return true; }
#define BB_MACRO_INSERT_F(bytebuffer, _sty, _ty) if( (_sty)==(#_ty) ) { *(bytebuffer) << (_ty)toNumber(Set.defaultarg); return true; }
#define BB_MACRO_EXTRACT_I(bytebuffer, _sty, _ty) if( (_sty)==(#_ty) && BB_CAN_READ(bytebuffer,_ty) ) {_ty _var; *(bytebuffer) >> _var; return DefScriptTools::toString((uint64)_var); }
#define BB_MACRO_EXTRACT_F(bytebuffer, _sty, _ty) if( (_sty)==(#_ty) && BB_CAN_READ(bytebuffer,_ty) ) {_ty _var; *(bytebuffer) >> _var; return DefScriptTools::toString((ldbl)_var); }


// Initializes bytebuffer with initial capacity (reserved, not resized)
// @def - bytebuffer identifier
// @0 - if set, initial capacity
DefReturnResult DefScriptPackage::func_bbinit(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.Get(_NormalizeVarName(Set.defaultarg,Set.myname));
    bb->clear(); // also resets wpos & rpos
    if (!Set.arg[0].empty())
        bb->reserve((size_t)DefScriptTools::toUint64(Set.arg[0]));
    return true;
}

// Cleans memory and deletes ByteBuffer from storage
DefReturnResult DefScriptPackage::func_bbdelete(CmdSet& Set)
{
    bytebuffers.Delete(_NormalizeVarName(Set.defaultarg,Set.myname));
    return true;
}
		
// Appends data to ByteBuffer
// @def - data
// @0 - bytebuffer identifier
// @1 - datatype of added data (uint8,uint16,uint32,uint64,float,double,string,strnz)
DefReturnResult DefScriptPackage::func_bbappend(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.Get(_NormalizeVarName(Set.arg[0],Set.myname));

    std::string dtype = DefScriptTools::stringToLower(Set.arg[1]);

    if (dtype == "string") 
    {
        *bb << Set.defaultarg;
        return true;
    }
    else if(dtype == "strnz")
    {
        bb->append(Set.defaultarg.c_str(), Set.defaultarg.length()); // append the string skipping \0
        return true;
    }

    BB_MACRO_INSERT_I(bb, dtype, uint8);
    BB_MACRO_INSERT_I(bb, dtype, uint16);
    BB_MACRO_INSERT_I(bb, dtype, uint32);
    BB_MACRO_INSERT_I(bb, dtype, uint64);
    BB_MACRO_INSERT_F(bb, dtype, float);
    BB_MACRO_INSERT_F(bb, dtype, double);

    return false;
}	

// Extracts data from bytebuffer
// @def - datatype to extract (uint8,uint16,uint32,uint64,float,double,string)
// @0 - bytebuffer identifier
DefReturnResult DefScriptPackage::func_bbread(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if (!bb)
        return false;

    std::string dtype = DefScriptTools::stringToLower(Set.defaultarg);

    if (dtype == "string") {
        std::string g;
        *bb >> g;
        return g;
	}
    else if(dtype == "strnz") // extract some amount of bytes not terminated by \0
    {
        uint32 bytes = (uint32)toUint64(Set.arg[1]);
        if(bytes)
        {
            std::string g;
            char *buf = new char[bytes+1]; // +1 for \0
            buf[bytes] = 0;
            bb->read((uint8*)buf,bytes);
            g = buf;
            delete [] buf;
            return g;
        }
        return "";
    }

    BB_MACRO_EXTRACT_I(bb, dtype, uint8);
    BB_MACRO_EXTRACT_I(bb, dtype, uint16);
    BB_MACRO_EXTRACT_I(bb, dtype, uint32);
    BB_MACRO_EXTRACT_I(bb, dtype, uint64);
    BB_MACRO_EXTRACT_F(bb, dtype, float);
    BB_MACRO_EXTRACT_F(bb, dtype, double);

    return ""; // if extraction unsuccessful, return empty string
}
	
// Sets read position in bytebuffer
// @0 - bytebuffer identifier
// @def - setted rpos (if "end", automatically set at end)
DefReturnResult DefScriptPackage::func_bbsetrpos(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if (!bb)
        return false;

    if (Set.defaultarg == "end")
        bb->rpos( bb->size() );
    else
        bb->rpos( (unsigned int)toNumber(Set.defaultarg) );
    return true;
}
	
// Sets write position in bytebuffer
// @0 - bytebuffer identifier
// @def - setted wpos (if "end", automatically set at end)
DefReturnResult DefScriptPackage::func_bbsetwpos(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if (!bb)
        return false;

    if (Set.defaultarg == "end")
        bb->wpos( bb->size() );
    else
        bb->wpos( (unsigned int)toNumber(Set.defaultarg) );
    return true;
}

// Hexlike output of bytebuffer to console
// @def - bytebuffer identifier
DefReturnResult DefScriptPackage::func_bbhexlike(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if (!bb)
        return false;

    bb->hexlike();
    return true;
}

// Textlike output of bytebuffer to console
// @def - bytebuffer identifier
DefReturnResult DefScriptPackage::func_bbtextlike(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if (!bb)
        return false;

    bb->textlike();
    return true;
}

// Returns size of bytebuffer
// @def - bytebuffer identifier
DefReturnResult DefScriptPackage::func_bbsize(CmdSet& Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if (!bb)
        return false;

    return DefScriptTools::toString((uint64)bb->size());
}
