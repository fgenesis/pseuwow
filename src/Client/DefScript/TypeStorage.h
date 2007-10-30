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
    void Unlink(std::string);

private:
    T *_Create(std::string);
    std::map<std::string,T*> _storage;
};

// check whether an object with this name is already present
template<class T> bool TypeStorage<T>::Exists(std::string s)
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->first == s)
            return true;
    }
    return false;
}

// helper to create and assign a new object
template<class T> T *TypeStorage<T>::_Create(std::string s)
{
    T *elem = new T();
    _storage[s] = elem;
    return elem;
}

// delete object with that name, if present
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

// return the the object with that name. return NULL if not found
template <class T> T *TypeStorage<T>::GetNoCreate(std::string s)
{
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end(); it++)
        if(it->first == s)
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
    for(std::map<std::string,T*>::iterator it = _storage.begin(); it != _storage.end();)
    {
        delete it->second;
        _storage.erase(it++);
    }
}

// stores an already existing object's pointer under a specific name; deletes and overwrites old of present
template<class T> void TypeStorage<T>::Assign(std::string s,T *elem)
{
    if(Exists(s))
        Delete(s);
    _storage[s] = elem;
}

// removes the pointer from the storage without deleting it
template<class T> void TypeStorage<T>::Unlink(std::string s)
{
    _storage.erase(s);
}


#endif
