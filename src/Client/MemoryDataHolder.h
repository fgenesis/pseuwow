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
        MDH_FILE_ERROR = 0, // file doesnt exist, cant be loaded, etc
        MDH_FILE_OK = 1, // file was loaded properly or already present in memory. point is: we have good data
        MDH_FILE_ALREADY_EXIST = 2, // file was loaded before
        MDH_FILE_JUST_LOADED = 4, // file was freshly loaded
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

    void Init(void);
    void SetThreadCount(uint32);

    memblock GetFile(std::string s, bool threaded = false, callback_func func = NULL,void *ptr = NULL, ZThread::Condition *cond = NULL, bool ref_counted = true);
    inline memblock GetFileBasic(std::string s) { return GetFile(s, false, NULL, NULL, NULL, false); }
    bool IsLoaded(std::string);
    void BackgroundLoadFile(std::string);
    bool Delete(std::string);
};

#endif
