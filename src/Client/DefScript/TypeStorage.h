#ifndef DEFSCRIPT_TYPESTORAGE_H
#define DEFSCRIPT_TYPESTORAGE_H

#include <string>
#include <map>

template <class T> class TypeStorage
{
public:
    typedef typename std::map<std::string,T*> _TypeMap;
    typedef typename _TypeMap::iterator _TypeIter;
    TypeStorage() { _keep = false; }
	~TypeStorage();
	bool Exists(std::string);
	void Delete(std::string);
    void DeleteByPtr(T*);
	T *Get(std::string);
    T *GetNoCreate(std::string);
    void Assign(std::string,T*,bool overwrite = true);
    void Unlink(std::string);
    void UnlinkByPtr(T*);
    std::string GetNameByPtr(T*);
    inline std::map<std::string,T*> &GetMap(void) { return _storage; }
    inline void SetKeepOnDestruct(bool b = true) { _keep = true; }
    inline unsigned int Size(void) { return _storage.size(); }
    void Clear(bool keep = false);

    void dump(void);



private:
    T *_Create(std::string);
    _TypeMap _storage;
    bool _keep;
};

template <class T> void TypeStorage<T>::dump(void)
{
    printf("TypeStorage dump, size=%u\n",Size());
    _TypeIter it;
    for(it = _storage.begin(); it != _storage.end(); it++)
    {
        printf("[%s] => 0x%X\n", it->first.c_str(), it->second);
    }
}

// check whether an object with this name is already present
template<class T> bool TypeStorage<T>::Exists(std::string s)
{
    return _storage.find(s) != _storage.end();
}

// helper to create and assign a new object
template<class T> T *TypeStorage<T>::_Create(std::string s)
{
    T *elem = new T;
    Assign(s,elem);
    return elem;
}

// delete object with that name, if present
template<class T> void TypeStorage<T>::Delete(std::string s)
{
    _TypeIter it = _storage.find(s);
    if(it != _storage.end())
    {
        delete it->second;
        _storage.erase(it);
    }
}

// delete object with that ptr, if present
template<class T> void TypeStorage<T>::DeleteByPtr(T *ptr)
{
    for(_TypeIter it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->second == ptr)
        {
            delete it->second;
            _storage.erase(it);
            return;
        }
    }
}

// return the the object with that name. return NULL if not found
template <class T> T *TypeStorage<T>::GetNoCreate(std::string s)
{
    _TypeIter it = _storage.find(s);
    if(it != _storage.end())
        return it->second;
    return NULL;
}

// return the the object with that name. create and return new object if not found
template<class T> T *TypeStorage<T>::Get(std::string s)
{
    T *elem = GetNoCreate(s);
    return elem ? elem : _Create(s);
}

// when destroying the TypeStorage, delete all stored objects
template<class T> TypeStorage<T>::~TypeStorage()
{
    Clear(_keep);
}

template<class T> void TypeStorage<T>::Clear(bool keep)
{
    if(keep)
    {
        _storage.clear();
        return;
    }
    for(_TypeIter it = _storage.begin(); it != _storage.end(); )
    {
        delete it->second;
        _storage.erase(it++);
    }
}

// stores an already existing object's pointer under a specific name; deletes and overwrites old if present
template<class T> void TypeStorage<T>::Assign(std::string s,T *elem, bool overwrite /* = true */ )
{
    _TypeIter it = _storage.find(s);
    if(it != _storage.end())
    {
        if(overwrite)
        {
            delete it->second;
        }
        _storage.erase(it);
    }
    _storage.insert(make_pair(s,elem));
}

// removes the pointer from the storage without deleting it
template<class T> void TypeStorage<T>::Unlink(std::string s)
{
    _storage.erase(s);
}

// removes the pointer from the storage without deleting it, if name is unknown
template<class T> void TypeStorage<T>::UnlinkByPtr(T *ptr)
{
    for(_TypeIter it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->second == ptr)
        {
            Unlink(it->first);
            return;
        }
    }
}

template<class T> std::string TypeStorage<T>::GetNameByPtr(T *ptr)
{
    for(_TypeIter it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->second == ptr)
        {
            return it->first;
        }
    }
    return "";
}

#endif
