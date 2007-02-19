#ifndef _PLAYER_H
#define _PLAYER_H

#include <vector>
#include "WorldPacket.h"
#include "SysDefs.h"
#include "Unit.h"

struct PlayerEnumItem
{
	uint32 displayId;
	uint8 inventorytype;
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
	PlayerEnumItem _items[20];

private:

    
};

class Player : public Unit
{
public:
    Player();
    void Create(uint64);

private:

};


// class about the character that is used to login.
// needs to store known spells, action buttons,...
// basically everything that is needed to play.
class MyCharacter : public Player
{
public:
    MyCharacter();

	/*void SetActionButtons(WorldPacket &data);
	void SetSpells(WorldPacket &data);
	void CastSpell(uint32 spellId, uint64 target);
	void HandleCastResultOpcode(WorldPacket &packet);*/

private:

};


#endif