#ifndef _SYSDEFS_H
#define _SYSDEFS_H

//////////////////////////////////////
// Platform defines
//////////////////////////////////////

#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2
#define PLATFORM_INTEL 3

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM PLATFORM_WIN32
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __INTEL_COMPILER )
#  define COMPILER COMPILER_INTEL
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if COMPILER == COMPILER_MICROSOFT
#  pragma warning( disable : 4267 )    // conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4786 )    // identifier was truncated to '255' characters in the debug information
#  pragma warning( disable : 4800 )    // conversion to bool, performance warning
#  pragma warning( disable : 4244 )    // conversion from 'uint64' to 'int16', possible loss of data
#  pragma warning( disable : 4996 )    // disable warning for "too old" functions (VC80)
#endif

////////////////////////////////////
// Compiler defines
////////////////////////////////////

#if COMPILER == COMPILER_MICROSOFT
    #define I64FMT "%016I64X"
    #define I64FMTD "%I64u"
    #define SI64FMTD "%I64d"
    #define snprintf _snprintf
    #define atoll __atoi64
    #define vsnprintf _vsnprintf
    #define strdup _strdup
    typedef __int64            int64;
    typedef long               int32;
    typedef short              int16;
    typedef char               int8;
    typedef unsigned __int64   uint64;
    typedef unsigned long      uint32;
    typedef unsigned short     uint16;
    typedef unsigned char      uint8;
#else
    #define stricmp strcasecmp
    #define strnicmp strncasecmp
    #define I64FMT "%016llX"
    #define I64FMTD "%llu"
    #define SI64FMTD "%lld"
    typedef __int64_t   int64;
    typedef __int32_t   int32;
    typedef __int16_t   int16;
    typedef __int8_t    int8;
    typedef __uint64_t  uint64;
    typedef __uint32_t  uint32;
    typedef __uint16_t  uint16;
    typedef __uint8_t   uint8;
    typedef uint16      WORD;
    typedef uint32      DWORD;
#endif

#ifndef SIGQUIT
#define SIGQUIT 3
#endif


#endif