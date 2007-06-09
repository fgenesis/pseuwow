#include "ListStorage.h"

bool ListStorage::Exists(std::string s)
{
    for(DefListMap::iterator it = _storage.begin(); it != _storage.end(); it++)
    {
        if(it->first == s)
            return true;
    }
    return false;
}

DefList *ListStorage::_Create(std::string s)
{
    DefList *l = new DefList();
    _storage[s] = l;
    return l;
}

void ListStorage::Delete(std::string s)
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

DefList *ListStorage::GetNoCreate(std::string s)
{
    for(DefListMap::iterator it = _storage.begin(); it != _storage.end(); it++)
        if(it->first == s)
            return it->second;
    return NULL;
}

DefList *ListStorage::Get(std::string s)
{
    DefList *l = GetNoCreate(s);
    return l ? l : _Create(s);
}
