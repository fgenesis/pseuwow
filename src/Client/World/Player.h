#ifndef _PLAYER_H
#define _PLAYER_H

#include <vector>

struct PlayerNameCacheItem {
    uint64 _guid;
    std::string _name;
};

class PlayerNameCache {
public:
    std::string GetName(uint64);
    uint64 GetGuid(std::string);
    bool AddInfo(uint64 guid, std::string name);
    bool AddInfo(PlayerNameCacheItem*);
    bool SaveToFile(void);
    bool ReadFromFile(void);
    uint32 GetSize(void);
private:
    std::vector<PlayerNameCacheItem*> _cache;
};

class PlayerEnum {
public:
	uint64 _guid;
	std::string _name;
	uint8 _race;
	uint8 _class;
	uint8 _gender;
	uint8 _bytes1;
	uint8 _bytes2;
	uint8 _bytes3;
	uint8 _bytes4;
	uint8 _bytesx;
	uint8 _level;
	uint32 _zoneId;
	uint32 _mapId;
	float _x;
	float _y;
	float _z;
	uint32 _guildId;
	uint8 _flags;
	uint32 _petInfoId;
	uint32 _petLevel;
	uint32 _petFamilyId;
// more to come...[items]

private:

    
};
/*
class PlayerCache {
public:
    void Add(Player*);
    void Remove(Player*);
    void Remove(uint64);
    uint32 GetCount(void) { return _players.size(); }

private:
    std::vector<Player*> _players;

};
*/



#endif