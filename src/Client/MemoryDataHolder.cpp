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
            executor = new ZThread::PoolExecutor(1);
    }

    void Shutdown(void)
    {
        //ZThread::Guard<ZThread::FastMutex> g(mutex);
        logdev("MDH: Interrupting work...");
        executor->cancel(); // stop accepting new threads
        executor->interrupt(); // interrupt all working threads
        // executor will delete itself automatically
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
            logdev("~DataLoaderRunnable(%s) 0x%X", _name.c_str(), this);
        }
        void SetStores(TypeStorage<memblock> *mem, TypeStorage<DataLoaderRunnable> *ldrs)
        {
            _storage = mem;
            _loaders = ldrs;
        }
        // the threaded part
        void run()
        { 
            memblock *mb = new memblock();

            mb->size = GetFileSize(_name.c_str());
            // couldnt open file if size is 0
            if(!mb->size)
            {
                ZThread::Guard<ZThread::FastMutex> g(_mut);
                logerror("DataLoaderRunnable: Error opening file: '%s'", _name.c_str());
                _loaders->Unlink(_name);
                DoCallbacks(_name, MDH_FILE_ERROR); // call callback func, 'false' to indicate file couldnt be loaded
                delete mb;
                return;
            }
            mb->alloc(mb->size);
            std::ifstream fh;
            fh.open(_name.c_str(), std::ios_base::in | std::ios_base::binary);
            if(!fh.is_open())
            {
                {
                    ZThread::Guard<ZThread::FastMutex> g(_mut);
                    logerror("DataLoaderRunnable: Error opening file: '%s'", _name.c_str());
                    _loaders->Unlink(_name);
                }
                mb->free();
                delete mb;
                DoCallbacks(_name, MDH_FILE_ERROR);
                return;
            }
            logdev("DataLoaderRunnable: Reading '%s'... (%s)", _name.c_str(), FilesizeFormat(mb->size).c_str());
            fh.read((char*)mb->ptr, mb->size);
            fh.close();
            {
                ZThread::Guard<ZThread::FastMutex> g(_mut);
                _storage->Assign(_name, mb);
                _loaders->Unlink(_name); // must be unlinked after the file is fully loaded, but before the callbacks are processed!
            }
            logdev("DataLoaderRunnable: Done with '%s' (%s)", _name.c_str(), FilesizeFormat(mb->size).c_str());
            DoCallbacks(_name, MDH_FILE_OK | MDH_FILE_JUST_LOADED);
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
       ZThread::FastMutex _mut;
       TypeStorage<memblock> *_storage;
       TypeStorage<DataLoaderRunnable> *_loaders;
       
    };


    MemoryDataResult GetFile(std::string s, bool threaded, callback_func func, void *ptr, ZThread::Condition *cond, bool ref_counted)
    {
        mutex.acquire(); // we need exclusive access, other threads might unload the requested file during checking

        if(alwaysSingleThreaded)
            threaded = false;

        // manage reference counter
        uint32 *refcount = refs.GetNoCreate(s);
        if(!refcount)
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
            DEBUG(logdev("MDH: Reusing '%s' from memory",s.c_str()));
            // the file was requested some other time, is still present in memory and the pointer can simply be returned...
            mutex.release(); // everything ok, mutex can be unloaded safely
            // execute callback and broadcast condition (must check for MDH_FILE_ALREADY_EXIST in callback func)
            uint32 rf = MDH_FILE_OK | MDH_FILE_ALREADY_EXIST;
            if(func)
                (*func)(ptr, s, rf);
            if(cond)
                cond->broadcast();

            return MemoryDataResult(*mb, rf);
        }
        else
        {
            DataLoaderRunnable *ldr = loaders.GetNoCreate(s);
            DEBUG(logdev("MDH: Found Loader 0x%X for '%s'",ldr,s.c_str()));
            if(ldr == NULL)
            {
                // no loader thread is working on that file...
                ldr = loaders.Get(s);
                ldr->SetStores(&storage,&loaders);
                ldr->AddCallback(func,ptr,cond); // not threadsafe!

                mutex.release(); // the mutex can be released safely now

                ldr->SetThreaded(threaded);
                ldr->SetName(s); // here we set the filename the thread should load

                if(threaded)
                {
                    ZThread::Task task(ldr);
                    executor->execute(task);
                }
                else
                {
                    ldr->run(); // will exit after the whole file is loaded and the callbacks were run
                    delete ldr;
                    memblock *mbret = storage.GetNoCreate(s);
                    DEBUG(logdev("Non-threaded loader returning memblock at 0x%X",mbret));
                    uint32 rf = MDH_FILE_JUST_LOADED;
                    if(mbret)
                        rf |= MDH_FILE_OK;
                    return MemoryDataResult(mbret ? *mbret : memblock(), rf);
                }
            }
            else // if a loader is already existing, add callbacks to that loader.
            {  
                ldr->AddCallback(func,ptr,cond);
                mutex.release();
            }
        }
        return MemoryDataResult(memblock(), MDH_FILE_LOADING); // we reach this point only in multithreaded mode
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
            if(*refcount > 0)
                (*refcount)--;
            logdev("MemoryDataHolder::Delete(\"%s\"): refcount dropped to %u", s.c_str(), *refcount);
        }
        if(!*refcount)
        {
            refs.Delete(s);
            if(memblock *mb = storage.GetNoCreate(s))
            {
                logdev("MemoryDataHolder:: deleting 0x%X (size %s)", mb->ptr, FilesizeFormat(mb->size).c_str());
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
