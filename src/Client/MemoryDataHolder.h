#ifndef MEMORYDATAHOLDER_H
#define MEMORYDATAHOLDER_H

#include "common.h"

namespace ZThread
{
    class Condition;
};

namespace MemoryDataHolder
{
    enum ResultFlags
    {
        MDH_FILE_ERROR         = 0x00, // file doesnt exist, cant be loaded, etc
        MDH_FILE_OK            = 0x01, // file was loaded properly or already present in memory. point is: we have good data
        MDH_FILE_ALREADY_EXIST = 0x02, // file was loaded before
        MDH_FILE_JUST_LOADED   = 0x04, // file was freshly loaded
        MDH_FILE_LOADING       = 0x08, // file is currently beeing loaded (returned only in multithreaded mode)
    };

    typedef void (*callback_func)(void *ptr,std::string filename, uint32 flags);
    struct callback_struct
    {
        callback_func func;
        void *ptr;
        ZThread::Condition *cond;
    };
    typedef std::deque<callback_struct> CallbackStore;

    struct memblock
    {
        memblock() : ptr(NULL), size(0) {}
        memblock(uint8 *p, uint32 s) : ptr(p), size(s) {}
        void alloc(uint32 s) { size = s; ptr = new uint8[s]; }
        void free(void) { delete [] ptr; }
        uint8 *ptr;
        uint32 size;
    };

    struct MemoryDataResult
    {
        MemoryDataResult(memblock mb, uint32 f) { data = mb; flags = f; }
        memblock data;
        uint32 flags; // see ResultFlags enum
    };

    void Init(void);
    void SetThreadCount(uint32);

    MemoryDataResult GetFile(std::string s, bool threaded = false, callback_func func = NULL,void *ptr = NULL, ZThread::Condition *cond = NULL, bool ref_counted = true);
    inline MemoryDataResult GetFileBasic(std::string s) { return GetFile(s, false, NULL, NULL, NULL, false); }
    bool IsLoaded(std::string);
    void BackgroundLoadFile(std::string);
    bool Delete(std::string);
};

#endif
