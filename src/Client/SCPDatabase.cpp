#include <fstream>
#include "common.h"
#include "SCPDatabase.h"


uint32 SCPDatabase::LoadFromFile(char *fn)
{
    std::fstream fh;
    std::string line,value,entry,storage;
    uint32 id=0,sections=0;
    char c;

    fh.open(fn,std::ios_base::in);
    if( !fh.is_open() )
        return 0;
    while(!fh.eof())
    {
        c=fh.get();
        if(c=='\n' || fh.eof())
        {
            if(line.empty())
                continue;
            while(line[0]==' ' || line[0]=='\t')
                line.erase(0,1);
            if(line.empty() || (line.length() > 1 && (line[0]=='/' && line[1]=='/')) )
            {
                line.clear();
                continue;
            }
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
            line+=c; // fill up line until a newline is reached (see above)
    }
    fh.close();
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
        if(i->first == e)
            return true;
    return false;
}

std::string SCPField::GetString(std::string entry)
{
    return HasEntry(entry) ? _map[entry] : "";
}