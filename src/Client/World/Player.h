#ifndef _PLAYER_H
#define _PLAYER_H

#include <vector>
#include "WorldPacket.h"
#include "SysDefs.h"

struct PlayerItem
{
	uint32 itemID;
	uint8 inventorytype;
};

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
	PlayerItem _items[20];

private:

    
};

class Player
{
public:
	void Init(PlayerEnum player);

private:
	int hp;
	int bar; // Mana/Energy/Rage
	PlayerEnum player;
};

class PlayerSettings
{
public:
	PlayerSettings()
	{
		castingSpell = false;
		init = false;
	}

	void Init(WorldSession *worldSession)
	{
		_worldSession = worldSession;
	}

	void SetActionButtons(WorldPacket &data);
	void SetSpells(WorldPacket &data);
	void CastSpell(uint32 spellId, uint64 target);
	void HandleCastResultOpcode(WorldPacket &packet);

private:
	bool castingSpell;
	WorldSession *_worldSession;
	bool init;
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