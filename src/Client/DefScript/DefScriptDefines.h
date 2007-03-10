#ifndef DEFSCRIPTDEFINES_H
#define DEFSCRIPTDEFINES_H

#define MAXARGS 99
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
typedef unsigned __int64_t uint64;
#endif

enum VariableType
{
    DEFSCRIPT_NONE=0,
    DEFSCRIPT_VAR,
    DEFSCRIPT_FUNC
};

#endif