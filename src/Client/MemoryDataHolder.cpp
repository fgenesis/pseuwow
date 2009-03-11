#include <fstream>
#include "MemoryDataHolder.h"
#include "DefScript/TypeStorage.h"
#include "zthread/Condition.h"
#include "zthread/Task.h"
#include "zthread/PoolExecutor.h"

namespace MemoryDataHolder
{
    class DataLoaderRunnable;
    ZThread::PoolExecutor *executor = NULL;

    ZThread::FastMutex mutex;
    TypeStorage<memblock> storage;
    TypeStorage<DataLoaderRunnable> loaders;
    TypeStorage<uint32> refs;
    bool alwaysSingleThreaded = false;

    void Init(void)
    {
        if(!executor)
            executor = new ZThread::PoolExecutor(1); // TODO: fix memleak on shutdown?
    }
    
    void SetThreadCount(uint32 t)
    {
        // 0 threads used means we use no threading at all
        if(!t)
        {
            logdetail("MemoryDataHolder: Single-threaded mode.");
            alwaysSingleThreaded = true;
            executor->size(1);
        }
        else
        {
            logdetail("MemoryDataHolder: Using %u threads.", t);
            alwaysSingleThreaded = false;
            executor->size(t);
        }
    }
    
    class DataLoaderRunnable : public ZThread::Runnable
    {
    public:
        DataLoaderRunnable()
        {
            _threaded = false;
        }
        ~DataLoaderRunnable()
        {
            logdev("~DataLoaderRunnable(%s)", _name.c_str());
        }
        // the threaded part
        void run()
        {
            const char *name = _name.c_str();
            memblock *mb = new memblock();

            mb->size = GetFileSize(name);
            // couldnt open file if size is 0
            if(!mb->size)
            {
                logerror("DataLoaderRunnable: Error opening file: '%s'", name);
                loaders.Unlink(name);
                DoCallbacks(name, MDH_FILE_ERROR); // call callback func, 'false' to indicate file couldnt be loaded
                delete mb;
                return;
            }
            mb->alloc(mb->size);
            std::ifstream fh;
            fh.open(name, std::ios_base::in | std::ios_base::binary);
            if(!fh.is_open())
            {
                logerror("DataLoaderRunnable: Error opening file: '%s'", name);
                loaders.Unlink(name);
                mb->free();
                delete mb;
                DoCallbacks(name, MDH_FILE_ERROR);
                return;
            }
            logdev("DataLoaderRunnable: Reading '%s'... (%s)", name, FilesizeFormat(mb->size).c_str());
            fh.read((char*)mb->ptr, mb->size);
            fh.close();
            storage.Assign(name, mb);
            loaders.Unlink(name); // must be unlinked after the file is fully loaded, but before the callbacks are processed!
            logdev("DataLoaderRunnable: Done with '%s' (%s)", name, FilesizeFormat(mb->size).c_str());
            DoCallbacks(name, MDH_FILE_OK | MDH_FILE_JUST_LOADED);
        }

        inline void AddCallback(callback_func func, void *ptr = NULL, ZThread::Condition *cond = NULL)
        {
            callback_struct cbs;
            cbs.func = func;
            cbs.ptr = ptr;
            cbs.cond = cond;
            _callbacks.push_back(cbs);
        }
        inline void DoCallbacks(std::string fn, uint32 flags)
        {
            for(CallbackStore::iterator it = _callbacks.begin(); it != _callbacks.end(); it++)
            {
                if(it->cond)
                    it->cond->broadcast();
                if(it->func)
                    (*(it->func))(it->ptr, fn, flags);
            }
        }
        inline void SetThreaded(bool t)
        {
            _threaded = t;
        }
        inline bool IsThreaded(void)
        {
            return _threaded;
        }
        inline void SetName(std::string n)
        {
            _name = n;
        }

       CallbackStore _callbacks;
       bool _threaded;
       std::string _name;
    };


    memblock GetFile(std::string s, bool threaded, callback_func func, void *ptr, ZThread::Condition *cond, bool ref_counted)
    {
        mutex.acquire(); // we need exclusive access, other threads might unload the requested file during checking

        if(alwaysSingleThreaded)
            threaded = false;

        // manage reference counter
        uint32 *refcount = refs.GetNoCreate(s);
        if(!refcount || !*refcount)
        {
            refcount = new uint32;
            *refcount = ref_counted ? 1 : 0;
            refs.Assign(s,refcount);
        }
        else
        {
            if(ref_counted)
            {
                (*refcount)++;
            }
        }

        if(memblock *mb = storage.GetNoCreate(s))
        {
            // the file was requested some other time, is still present in memory and the pointer can simply be returned...
            mutex.release(); // everything ok, mutex can be unloaded safely
            // execute callback and broadcast condition (must check for MDH_FILE_ALREADY_EXIST in callback func)
            if(func)
                (*func)(ptr, s, MDH_FILE_OK | MDH_FILE_ALREADY_EXIST);
            if(cond)
                cond->broadcast();

            return *mb;
        }
        else
        {
            DataLoaderRunnable *r = loaders.GetNoCreate(s);
            if(r == NULL)
            {
                // no loader thread is working on that file...
                r = new DataLoaderRunnable();
                loaders.Assign(s,r);
                r->AddCallback(func,ptr,cond); // not threadsafe!
                r->SetThreaded(threaded);
                r->SetName(s); // here we set the filename the thread should load
                // the mutex can be released safely now
                mutex.release();
                if(threaded)
                {
                    ZThread::Task task(r);
                    executor->execute(task);
                }
                else
                {
                    r->run(); // will exit after the whole file is loaded and the callbacks were run
                    memblock *mb = storage.GetNoCreate(s);
                    delete r;
                    return *mb;
                }
            }
            else // if a loader is already existing, add callbacks to that loader.
            {  
                r->AddCallback(func,ptr,cond);
                mutex.release();
            }
        }
        return memblock();
    }

    bool IsLoaded(std::string s)
    {
        ZThread::Guard<ZThread::FastMutex> g(mutex);
        return storage.Exists(s);
    }

    // ensure the file is present in memory, but do not touch the reference counter
    void BackgroundLoadFile(std::string s)
    {
        GetFile(s, true, NULL, NULL, NULL, false);
    }


    bool Delete(std::string s)
    {
        ZThread::Guard<ZThread::FastMutex> g(mutex);
        uint32 *refcount = refs.GetNoCreate(s);
        if(!refcount)
        {
            logerror("MemoryDataHolder:Delete(\"%s\"): no refcount", s.c_str());
            return false;
        }
        else
        {
            if(*refcount)
                (*refcount)--;
            logdev("MemoryDataHolder::Delete(\"%s\"): refcount dropped to %u", s.c_str(), *refcount);
        }
        if(!*refcount)
        {
            refs.Delete(s);
            if(memblock *mb = storage.GetNoCreate(s))
            {
                logdev("MemoryDataHolder:: deleting 0x%X (size %u)", mb->ptr, mb->size);
                mb->free();
                storage.Delete(s);
                return true;
            }
            else
            {
                logerror("MemoryDataHolder::Delete(\"%s\"): no buf existing",s.c_str());
                return false;
            }
        }
        return true;
    }





};
