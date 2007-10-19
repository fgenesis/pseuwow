#ifndef _CACHEHANDLER_H
#define _CACHEHANDLER_H

struct PlayerNameCacheItem {
    uint64 _guid;
    std::string _name;
};

class PlayerNameCache {
public:
	~PlayerNameCache();

    std::string GetName(uint64);
    bool IsKnown(uint64);
    uint64 GetGuid(std::string);
    bool AddInfo(uint64 guid, std::string name);
    bool AddInfo(PlayerNameCacheItem*);
    bool SaveToFile(void);
    bool ReadFromFile(void);
    uint32 GetSize(void);
private:
    std::vector<PlayerNameCacheItem*> _cache;
};

void ItemProtoCache_InsertDataToSession(WorldSession *session);
void ItemProtoCache_WriteDataToCache(WorldSession *session);

#endif
