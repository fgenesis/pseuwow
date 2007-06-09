#ifndef DEFSCRIPT_LISTSTORAGE_H
#define DEFSCRIPT_LISTSTORAGE_H

#include <string>
#include <deque>
#include <map>

typedef std::deque<std::string> DefList;
typedef std::map<std::string,DefList*> DefListMap;

class ListStorage
{
public:
	bool Exists(std::string);
	void Delete(std::string);
	DefList *Get(std::string);
    DefList *GetNoCreate(std::string);
    inline void Assign(std::string s,DefList *l) { _storage[s] = l; }

private:
	DefList *_Create(std::string);
	DefListMap _storage;
};

#endif
