#ifndef _SCPDATABASE_H
#define _SCPDATABASE_H

#include <map>

struct SCPEntry
{
    std::string entry;
    std::string value;
};

typedef std::map<std::string,std::string> SCPEntryMap;

class SCPField
{
public:
    std::string GetString(std::string entry) { return _map[entry]; }
    uint64 GetInteger(std::string entry) { return toInt(_map[entry]); }
    double GetDouble(std::string entry) { return strtod(_map[entry].c_str(),NULL); }
    void Set(std::string entry,std::string value) { _map[entry]=value; }

private:
    SCPEntryMap _map;
};

typedef std::map<uint32,SCPField> SCPFieldMap; 


class SCPDatabase
{
public:
    SCPField& GetField(uint32 id) { return _map[id]; }
    uint32 LoadFromFile(char*);

private:
    SCPFieldMap _map;

};

#endif
