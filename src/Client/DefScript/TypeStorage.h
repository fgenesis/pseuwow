#ifndef DEFSCRIPT_TYPESTORAGE_H
#define DEFSCRIPT_TYPESTORAGE_H

#include <string>
#include <map>

template <class T> class TypeStorage
{
public:
	~TypeStorage();
	bool Exists(std::string);
	void Delete(std::string);
	T *Get(std::string);
    T *GetNoCreate(std::string);
    void Assign(std::string,T*);

private:
    T *_Create(std::string);
    std::map<std::string,T*> _storage;
};


template<class T> bool TypeStorage<T>::Exists(std::string s)
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->first == s)
            return true;
    }
    return false;
}

template<class T> T *TypeStorage<T>::_Create(std::string s)
{
    T *elem = new T();
    _storage[s] = elem;
    return elem;
}

template<class T> void TypeStorage<T>::Delete(std::string s)
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end(); it++)
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
    T *elem = GetNoCreate(s);
    return elem ? elem : _Create(s);
}

template<class T> TypeStorage<T>::~TypeStorage()
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end();)
    {
        delete it->second;
        _storage.erase(it++);
    }
}

template<class T> void TypeStorage<T>::Assign(std::string s,T *elem)
{
    if(Exists(s))
        Delete(s)
    _storage[s] = elem;
}


#endif
