#include <vector>
#include <fstream>
#include "common.h"
#include "PseuWoW.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "Player.h"
#include "NameTables.h"
#include "DefScript/DefScript.h"

bool PlayerNameCache::AddInfo(uint64 guid, std::string name){
    PlayerNameCacheItem *cacheItem=new PlayerNameCacheItem;
    cacheItem->_name=name;
    cacheItem->_guid=guid;
    return AddInfo(cacheItem);
}

bool PlayerNameCache::AddInfo(PlayerNameCacheItem* cacheItem){
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(cacheItem->_guid==(*i)->_guid)
            return false;
    _cache.push_back(cacheItem);
    return true;
}

std::string PlayerNameCache::GetName(uint64 guid){
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(guid==(*i)->_guid)
            return (*i)->_name;
    return "";
}

uint64 PlayerNameCache::GetGuid(std::string name){
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(name==(*i)->_name)
            return (*i)->_guid;
    return 0;
}

bool PlayerNameCache::SaveToFile(void){
	printf("Saving PlayerNameCache...\n");
	char *fn="./cache/playernames.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
	{
		printf("ERROR: could not open file '%s'!\n",fn);
        return false;
    }
    uint32 size=_cache.size();
    if(size==0)
        return false;
    uint8 len;
    fh.write((char*)&size,sizeof(uint32));

    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
    {
        fh.write( (char*)&((*i)->_guid),sizeof(uint64) );
        len=(*i)->_name.length();
        fh.write( (char*)&len,sizeof(uint8) );
        fh.write( (char*)(*i)->_name.c_str(),len );
        DEBUG(printf( "PlayerNameCache << " I64FMT " -> %s\n", (*i)->_guid, (*i)->_name.c_str()););
    }
    fh.close();
	printf("PlayerNameCache saved successfully.\n");
    return true;
}

bool PlayerNameCache::ReadFromFile(void){
	char *fn="./cache/playernames.cache";
	printf("Loading PlayerNameCache...\n");
    bool success=true;
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
	{
		printf("ERROR: could not open file '%s'!\n",fn);
        return false;
    }
    uint32 size;
    fh.read((char*)&size,sizeof(uint32));
    std::string tmp;
    uint8 len;
    char *nameptr=new char[13];
    for(unsigned int i=0;i<size;i++)
    {
        len=255;
        memset(nameptr,0,13);
        PlayerNameCacheItem *cacheItem=new PlayerNameCacheItem;
        fh.read((char*)&(cacheItem->_guid),sizeof(uint64));
        fh.read((char*)&len,sizeof(uint8));
        if(len>12 || len<2){
            printf("\nERROR: PlayerNameCache data seem corrupt [namelength=%d, should be <=12}]\n",len);
            printf("-> Clearing cache, creating new.\n");
            _cache.clear();
            success=false;
            break;
        }
        fh.read(nameptr,len);
        cacheItem->_name=nameptr;
        AddInfo(cacheItem);
        printf("\rPlayerNameCache [ %u / %u ] items loaded",i+1,size);
        DEBUG(printf( " >> " I64FMT " -> %s\n", cacheItem->_guid, nameptr););
    }
    printf("\n");
    delete nameptr;
    fh.close();
	if(success)
		printf("PlayerNameCache successfully loaded.\n");
    return success;
}

uint32 PlayerNameCache::GetSize(void){
    return _cache.size();
}
