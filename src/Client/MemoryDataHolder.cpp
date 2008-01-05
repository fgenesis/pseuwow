#include <fstream>
#include "MemoryDataHolder.h"
#include "DefScript/TypeStorage.h"

namespace MemoryDataHolder
{
    class DataLoaderRunnable;

    ZThread::FastMutex mutex;
    TypeStorage<uint8*> storage;
    TypeStorage<DataLoaderRunnable> loaders;
    
    // instances of this class MUST be created with new-operator, or Destroy() will cause a crash!
    class DataLoaderRunnable : public ZThread::Runnable
    {
    public:
        DataLoaderRunnable()
        {
            _buf = NULL;
            _threaded = false;
        }
        // the threaded part
        void run()
        {
            uint32 size = GetFileSize(_name.c_str());
            // couldnt open file if size is 0
            if(!size)
            {
                logerror("DataLoaderRunnable: Error opening file: '%s'",_name.c_str());
                DoCallbacks(false); // call callback func, 'false' to indicate file coulsnt be loaded
                Destroy();
                return;
            }
            _buf = new uint8[size];
            std::ifstream fh;
            fh.open(_name.c_str(), std::ios_base::in | std::ios_base::binary);
            if(!fh.is_open())
            {
                logerror("DataLoaderRunnable: Error opening file: '%s'",_name.c_str());
                delete _buf;
                _buf = NULL;
                DoCallbacks(false);
                Destroy();
                return;
            }
            fh.read((char*)_buf,size);
            fh.close();
            storage.Assign(_name,&_buf);
            loaders.UnlinkByPtr(this); // must be unlinked after the file is fully loaded, but before the callbacks are processed!
            DoCallbacks(true);
            Destroy();
        }

        inline void AddCallback(callback_func func, void *ptr = NULL)
        {
            _callbacks[func] = ptr;
        }
        inline void SetName(std::string name)
        {
            _name = name;
        }
        // if this class has done its work, delete self
        inline void Destroy(void)
        {
            delete this;
        }
        inline uint8 *GetBuf(void)
        {
            return _buf;
        }
        inline void DoCallbacks(bool success = true)
        {
            for(std::map<callback_func,void*>::iterator it = _callbacks.begin(); it != _callbacks.end(); it++)
            {
                (*(it->first))(it->second,success);
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
        inline bool HasCallbackFunc(callback_func f)
        {
            return _callbacks.find(f) != _callbacks.end();
        }


   private:
       std::string _name;
       std::map<callback_func, void*> _callbacks;
       uint8 *_buf;
       bool _threaded;
    };


    uint8 *GetFile(std::string s, bool threaded = false, callback_func func = NULL,void *ptr = NULL)
    {
        mutex.acquire(); // we need excusive access, other threads might unload the requested file during checking
        if(uint8 **buf = storage.GetNoCreate(s))
        {
            // the file was requested some other time, is still present in memory and the pointer can simply be returned
            mutex.release(); // everything ok, mutex can be unloaded safely before returning
            return *buf;
        }
        else
        {
            DataLoaderRunnable *r = loaders.GetNoCreate(s);
            if(r == NULL)
            {
                // no loader thread is working on that file...
                r = new DataLoaderRunnable();
                loaders.Assign(s,r);
                r->AddCallback(func,ptr); // not threadsafe!
                // after assigning/registering a new loader to the file, the mutex can be released safely
                mutex.release();
                r->SetName(s);
                r->SetThreaded(threaded);
                if(threaded)
                {
                    ZThread::Thread t(r); // start thread
                }
                else
                {
                    r->run(); // will exit after the whole file is loaded and the (one) callback is run
                    return r->GetBuf();
                }
            }
            else // if a loader is already existing, add callbacks to that loader.
            {  
                r->AddCallback(func,ptr);
                mutex.release();
            }
        }
        return NULL;
    }



};
