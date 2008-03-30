#include "common.h"
#include "log.h"
#include "DrawObject.h"
#include "DrawObjMgr.h"

DrawObjMgr::DrawObjMgr()
{
    DEBUG( logdebug("DrawObjMgr created") );
}

DrawObjMgr::~DrawObjMgr()
{
    DEBUG( logdebug("~DrawObjMgr(), deleting %u DrawObjects...", _storage.size() ) );
    for(DrawObjStorage::iterator i = _storage.begin(); i != _storage.end(); i++)
    {
        DEBUG( logdebug("del for guid "I64FMT, i->first) );
        delete i->second; // this can be done safely, since the object ptrs are not accessed
    }

    while(_add.size())
    {
        delete _add.next().second;
    }
}

void DrawObjMgr::Add(uint64 objguid, DrawObject *o)
{
    _add.add(std::pair<uint64,DrawObject*>(objguid,o));
}

void DrawObjMgr::Delete(uint64 guid)
{
    _del.add(guid);
}

void DrawObjMgr::Update(void)
{
    ZThread::FastMutex mut;

    // now for the threadsafe part: lock every thread except this one
    // to prevent obj ptr corruption caused by other running threads
    // TODO: lock only main thread (that should be the only one to delete objects anyway!)
    mut.acquire();

    // add objects waiting on the add queue to the real storage
    while(_add.size())
    {
        std::pair<uint64,DrawObject*> p = _add.next();
        DEBUG(logdebug("DrawObjMgr: adding DrawObj 0x%X guid "I64FMT" to main storage",p.second,p.first));
        _storage[p.first] = p.second;
    }

    // same for objects that should be deleted
    while(_del.size())
    {
        uint64 guid = _del.next();
        if(_storage.find(guid) != _storage.end())
        {
            
            DrawObject *o = _storage[guid];
            DEBUG(logdebug("DrawObjMgr: removing DrawObj 0x%X guid "I64FMT" from main storage",o,guid));
            _storage.erase(guid);
            delete o;
        }
        else
        {
            DEBUG(logdebug("DrawObjMgr: ERROR: removable DrawObject "I64FMT" not exising",guid));
        }
    }

    // now draw everything
    for(DrawObjStorage::iterator i = _storage.begin(); i != _storage.end(); i++)
    {
        i->second->Draw();
    }

    mut.release();

}
