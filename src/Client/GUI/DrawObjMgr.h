#ifndef DRAWOBJMGR_H
#define DRAWOBJMGR_H

#include <utility>

class DrawObject;

typedef std::map<uint64,DrawObject*> DrawObjStorage;

class DrawObjMgr
{
public:
    DrawObjMgr();
    ~DrawObjMgr();
    void Add(uint64,DrawObject*);
    void Delete(uint64);
    void Clear(void);
    void Update(void); // Threadsafe! delete code must be called from here!
    uint32 StorageSize(void) { return _storage.size(); }
    void UnlinkAll(void);

private:
    DrawObjStorage _storage;
    ZThread::LockedQueue<uint64,ZThread::FastMutex> _del;
    ZThread::LockedQueue<std::pair<uint64,DrawObject*>,ZThread::FastMutex > _add;

};

#endif
