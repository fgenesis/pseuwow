#ifndef _SCPDATABASE_H
#define _SCPDATABASE_H

#include <map>

/*
 * Some preparations for compiled databases. i will continue this later. [FG]

enum SCPTypes
{
    SCP_TYPE_STRING = 0,
    SCP_TYPE_INT,
    SCP_TYPE_UINT,
    SCP_TYPE_FLOAT
};

class SCPEntry
{
private:
    char *str;

public:

    SCPEntry() { str = NULL; }
    ~SCPEntry() { if(str) delete [] str; }

    union
    {
        int32 intvalue
        uint32 uintvalue;
        float floatvalue;
    };

    inline char *GetString(uint8 type)
    {
        if(str)
            return str;
        else
        {
            char buf[25];
            char *fmt;
            switch (type)
            {
                case SCP_TYPE_INT: sprintf(buf,"%d",intvalue); break;
                case SCP_TYPE_UINT: sprintf(buf,"%u",uintvalue); break;
                case SCP_TYPE_FLOAT: sprintf(buf,"%f",floatvalue); break;
            }
            str = new char[strlen(buf) + 1];
            memcpy(str,buf,strlen(buf) + 1);
            return str;
        }
    }
};

*/

typedef std::map<std::string,std::string> SCPEntryMap;

class SCPField
{
public:
    std::string GetString(std::string);
    inline uint64 GetInteger(std::string entry) { return toInt(GetString(entry)); }
    inline double GetDouble(std::string entry) { return strtod(GetString(entry).c_str(),NULL); }
    inline void Set(std::string entry,std::string value) { _map[entry]=value; }
    bool HasEntry(std::string);

private:
    SCPEntryMap _map;
};

typedef std::map<uint32,SCPField> SCPFieldMap;


class SCPDatabase
{
public:
    inline SCPField& GetField(uint32 id) { return _map[id]; }
    bool HasField(uint32 id);
    uint32 LoadFromFile(char*);
    uint32 LoadFromMem(char*,uint32);
    uint32 GetFieldByValue(std::string entry, std::string value);

private:
    SCPFieldMap _map;

};

typedef std::map<std::string,SCPDatabase> SCPDatabaseMap;

class SCPDatabaseMgr
{
public:
    bool HasDB(std::string);
    SCPDatabase& GetDB(std::string);
    uint32 AutoLoadFile(char *fn);
    inline void DropDB(std::string s) { _map.erase(stringToLower(s)); }

    //////////////////////
    // helper functions //
    //////////////////////
    std::string GetZoneName(uint32 id);
    std::string GetRaceName(uint32 id);
    std::string GetClassName_(uint32 id);
    std::string GetGenderName(uint32 id);
    std::string GetMapName(uint32 id);
    std::string GetLangName(uint32 id);

private:
    SCPDatabaseMap _map;
};




#endif
