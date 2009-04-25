#ifndef _CACHEHANDLER_H
#define _CACHEHANDLER_H

typedef std::map<uint64,std::string> PlayerNameMap; 

class PlayerNameCache
{
public:
	~PlayerNameCache();

    std::string GetName(uint64);
    bool IsKnown(uint64);
    uint64 GetGuid(std::string);
    void Add(uint64 guid, std::string name);
    bool SaveToFile(void);
    bool ReadFromFile(void);
    uint32 GetSize(void);
private:
    PlayerNameMap _cache;
};

void ItemProtoCache_InsertDataToSession(WorldSession *session);
void ItemProtoCache_WriteDataToCache(WorldSession *session);

void CreatureTemplateCache_InsertDataToSession(WorldSession *session);
void CreatureTemplateCache_WriteDataToCache(WorldSession *session);

void GOTemplateCache_InsertDataToSession(WorldSession *session);
void GOTemplateCache_WriteDataToCache(WorldSession *session);

#endif
