#include "TypeStorage.h"

template<class T> bool TypeStorage<T>::Exists(std::string s)
{
    for(DefListMap::iterator it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->first == s)
            return true;
    }
    return false;
}

template<class T> T *TypeStorage<T>::_Create(std::string s)
{
    T *l = new DefList();
    _storage[s] = l;
    return l;
}

template<class T> void TypeStorage<T>::Delete(std::string s)
{
    for(DefListMap::iterator it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->first == s)
        {
            delete it->second;
            _storage.erase(it);
            return;
        }
    }
}

template <class T> T *TypeStorage<T>::GetNoCreate(std::string s)
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end(); it++)
        if(it->first == s)
            return it->second;
    return NULL;
}

template<class T> T *TypeStorage<T>::Get(std::string s)
{
    DefList *l = GetNoCreate(s);
    return l ? l : _Create(s);
}

template<class T> TypeStorage<T>::~TypeStorage()
{
	for(DefListMap::iterator it = _storage.begin(); it != _storage.end();)
	{
		delete it->second;
		_storage.erase(it++);
	}
}
