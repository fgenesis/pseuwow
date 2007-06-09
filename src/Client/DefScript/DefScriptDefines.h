#ifndef DEFSCRIPTDEFINES_H
#define DEFSCRIPTDEFINES_H

#ifdef _DEBUG
#    define _DEFSC_DEBUG(code) code;
#else
#    define _DEFSC_DEBUG(code)
#endif

#if COMPILER == COMPILER_MICROSOFT
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef __int64_t int64;
typedef __uint64_t uint64;
// TODO: correct ATOI64 for linux if necessary
#endif

enum VariableType
{
    DEFSCRIPT_NONE=0,
    DEFSCRIPT_VAR,
    DEFSCRIPT_FUNC
};

typedef long double ldbl;

#endif