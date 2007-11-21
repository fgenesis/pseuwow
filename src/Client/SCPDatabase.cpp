#include <fstream>
#include "common.h"
#include "SCPDatabase.h"


uint32 SCPDatabase::LoadFromFile(char *fn)
{
    std::fstream fh;
    uint32 size = GetFileSize(fn);
    if(!size)
        return 0;

    fh.open(fn,std::ios_base::in | std::ios_base::binary);
    if( !fh.is_open() )
        return 0;

    char *buf = new char[size];
    fh.read(buf,size);
    fh.close();

    uint32 sections = LoadFromMem(buf,size);
    delete [] buf;
    return sections;
}


uint32 SCPDatabase::LoadFromMem(char *buf, uint32 size)
{
    std::string line,value,entry,storage;
    uint32 id=0,sections=0;

    for(uint32 pos = 0; pos < size; pos++)
    {
        if(buf[pos] == '\n')
        {
            if(line.empty())
                continue;
            while(line[0]==' ' || line[0]=='\t')
                line.erase(0,1);
            if(line.empty() || (line.length() > 1 && (line[0]=='#' || (line[0]=='/' && line[1]=='/'))) )
            {
                line.clear();
                continue;
            }
            if(line[line.size()-1] == 13 || line[line.size()-1] == 10) // this fixes the annoying newline problems on windows + binary mode
                line[line.size()-1] = 0;
            if(line[0]=='[')
            {
                id=(uint32)toInt(line.c_str()+1); // start reading after '['
                sections++;
            }
            else
            {
                uint32 pos=line.find("=");
                if(pos!=std::string::npos)
                {
                    entry=stringToLower(line.substr(0,pos));
                    value=line.substr(pos+1,line.length()-1);
                    _map[id].Set(entry,value);
                }
                // else invalid line, must have '='
            }
            line.clear();
        }
        else
            line += buf[pos]; // fill up line until a newline is reached (see above)
    }
    return sections;
}

bool SCPDatabase::HasField(uint32 id)
{
    for(SCPFieldMap::iterator i = _map.begin(); i != _map.end(); i++)
        if(i->first == id)
            return true;
    return false;
}

bool SCPField::HasEntry(std::string e)
{
    for(SCPEntryMap::iterator i = _map.begin(); i != _map.end(); i++)
    {
        std::string ch = i->first;
        if(ch == e)
            return true;
    }
    return false;
}

std::string SCPField::GetString(std::string entry)
{
    //return HasEntry(entry) ? _map[entry] : "";
    return _map[entry];
}

// note that this can take a while depending on the size of the database!
uint32 SCPDatabase::GetFieldByValue(std::string entry, std::string value)
{
    for(SCPFieldMap::iterator fm = _map.begin(); fm != _map.end(); fm++)
        if(fm->second.HasEntry(entry) && fm->second.GetString(entry)==value)
            return fm->first;
    return uint32(-1);
}

bool SCPDatabaseMgr::HasDB(std::string n)
{
    for(SCPDatabaseMap::iterator i = _map.begin(); i != _map.end(); i++)
        if(i->first == n)
            return true;
    return false;
}

SCPDatabase& SCPDatabaseMgr::GetDB(std::string n)
{
    return _map[n];
}

uint32 SCPDatabaseMgr::AutoLoadFile(char *fn)
{
    std::fstream fh;
    uint32 size = GetFileSize(fn);
    if(!size)
        return 0;

    fh.open(fn,std::ios_base::in | std::ios_base::binary);
    if( !fh.is_open() )
        return 0;

    char *buf = new char[size];

    fh.read(buf,size);
    fh.close();

    std::string line,dbname;
    for(uint32 pos = 0; pos < size; pos++)
    {
        if(buf[pos] == '\n')
        {
            if(line.empty())
                continue;
            while(line[0]==' ' || line[0]=='\t')
                line.erase(0,1);
            if(line[0] == '#')
            {
                uint32 eq = line.find("=");
                if(eq != std::string::npos)
                {
                    std::string info = stringToLower(line.substr(0,pos));
                    std::string value = stringToLower(line.substr(pos+1,line.length()-1));
                    if(info == "#dbname")
                    {
                        dbname = value;
                        break;
                    }
                }
            }
        }
        else
            line += buf[pos];
    }
    delete [] buf;

    if(dbname.empty())
        return 0;

    uint32 sections = GetDB(dbname).LoadFromMem(buf,size);
    return sections;

}

// -- helper functions -- //

std::string SCPDatabaseMgr::GetZoneName(uint32 id)
{
    return GetDB("zone").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetRaceName(uint32 id)
{
    std::string r = GetDB("race").GetField(id).GetString("name");
    //if(r.empty())
    //    r = raceName[id];
    return r;
}

std::string SCPDatabaseMgr::GetMapName(uint32 id)
{
    return GetDB("map").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetClassName_(uint32 id)
{
    std::string r = GetDB("class").GetField(id).GetString("name");
    //if(r.empty())
    //    r = className[id];
    return r;
}

std::string SCPDatabaseMgr::GetGenderName(uint32 id)
{
    return GetDB("gender").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetLangName(uint32 id)
{
    std::string r = GetDB("language").GetField(id).GetString("name");
    //if(r.empty())
    //    r = LookupName(id,langNames);
    return r;
}
