#ifndef _PLAYER_H
#define _PLAYER_H

#include <vector>
#include "WorldPacket.h"
#include "SysDefs.h"
#include "Unit.h"


/*
TODO: Update me or use UpdateFields directly
#define PLAYER_SLOT_START            0

#define EQUIPMENT_SLOT_START         0
#define EQUIPMENT_SLOT_HEAD          0
#define EQUIPMENT_SLOT_NECK          1
#define EQUIPMENT_SLOT_SHOULDERS     2
#define EQUIPMENT_SLOT_BODY          3
#define EQUIPMENT_SLOT_CHEST         4
#define EQUIPMENT_SLOT_WAIST         5
#define EQUIPMENT_SLOT_LEGS          6
#define EQUIPMENT_SLOT_FEET          7
#define EQUIPMENT_SLOT_WRISTS        8
#define EQUIPMENT_SLOT_HANDS         9
#define EQUIPMENT_SLOT_FINGER1       10
#define EQUIPMENT_SLOT_FINGER2       11
#define EQUIPMENT_SLOT_TRINKET1      12
#define EQUIPMENT_SLOT_TRINKET2      13
#define EQUIPMENT_SLOT_BACK          14
#define EQUIPMENT_SLOT_MAINHAND      15
#define EQUIPMENT_SLOT_OFFHAND       16
#define EQUIPMENT_SLOT_RANGED        17
#define EQUIPMENT_SLOT_TABARD        18
#define EQUIPMENT_SLOT_END           19

#define INVENTORY_SLOT_BAG_0         255
#define INVENTORY_SLOT_BAG_START     19
#define INVENTORY_SLOT_BAG_1         19
#define INVENTORY_SLOT_BAG_2         20
#define INVENTORY_SLOT_BAG_3         21
#define INVENTORY_SLOT_BAG_4         22
#define INVENTORY_SLOT_BAG_END       23

#define INVENTORY_SLOT_ITEM_START    23
#define INVENTORY_SLOT_ITEM_1        23
#define INVENTORY_SLOT_ITEM_2        24
#define INVENTORY_SLOT_ITEM_3        25
#define INVENTORY_SLOT_ITEM_4        26
#define INVENTORY_SLOT_ITEM_5        27
#define INVENTORY_SLOT_ITEM_6        28
#define INVENTORY_SLOT_ITEM_7        29
#define INVENTORY_SLOT_ITEM_8        30
#define INVENTORY_SLOT_ITEM_9        31
#define INVENTORY_SLOT_ITEM_10       32
#define INVENTORY_SLOT_ITEM_11       33
#define INVENTORY_SLOT_ITEM_12       34
#define INVENTORY_SLOT_ITEM_13       35
#define INVENTORY_SLOT_ITEM_14       36
#define INVENTORY_SLOT_ITEM_15       37
#define INVENTORY_SLOT_ITEM_16       38
#define INVENTORY_SLOT_ITEM_END      39

#define BANK_SLOT_ITEM_START         39
#define BANK_SLOT_ITEM_1             39
#define BANK_SLOT_ITEM_2             40
#define BANK_SLOT_ITEM_3             41
#define BANK_SLOT_ITEM_4             42
#define BANK_SLOT_ITEM_5             43
#define BANK_SLOT_ITEM_6             44
#define BANK_SLOT_ITEM_7             45
#define BANK_SLOT_ITEM_8             46
#define BANK_SLOT_ITEM_9             47
#define BANK_SLOT_ITEM_10            48
#define BANK_SLOT_ITEM_11            49
#define BANK_SLOT_ITEM_12            50
#define BANK_SLOT_ITEM_13            51
#define BANK_SLOT_ITEM_14            52
#define BANK_SLOT_ITEM_15            53
#define BANK_SLOT_ITEM_16            54
#define BANK_SLOT_ITEM_17            55
#define BANK_SLOT_ITEM_18            56
#define BANK_SLOT_ITEM_19            57
#define BANK_SLOT_ITEM_20            58
#define BANK_SLOT_ITEM_21            59
#define BANK_SLOT_ITEM_22            60
#define BANK_SLOT_ITEM_23            61
#define BANK_SLOT_ITEM_24            62
#define BANK_SLOT_ITEM_END           63

#define BANK_SLOT_BAG_START          63
#define BANK_SLOT_BAG_1              63
#define BANK_SLOT_BAG_2              64
#define BANK_SLOT_BAG_3              65
#define BANK_SLOT_BAG_4              66
#define BANK_SLOT_BAG_5              67
#define BANK_SLOT_BAG_6              68
#define BANK_SLOT_BAG_END            69

// strored in m_buybackitems
#define BUYBACK_SLOT_START           69
#define BUYBACK_SLOT_1               69
#define BUYBACK_SLOT_2               70
#define BUYBACK_SLOT_3               71
#define BUYBACK_SLOT_4               72
#define BUYBACK_SLOT_5               73
#define BUYBACK_SLOT_6               74
#define BUYBACK_SLOT_7               75
#define BUYBACK_SLOT_8               76
#define BUYBACK_SLOT_9               77
#define BUYBACK_SLOT_10              78
#define BUYBACK_SLOT_11              79
#define BUYBACK_SLOT_12              80
#define BUYBACK_SLOT_END             81

#define KEYRING_SLOT_START           81
#define KEYRING_SLOT_END             97

// last+1 slot for item stored (in any way in player m_items data)
#define PLAYER_SLOT_END              97
#define PLAYER_SLOTS_COUNT           (PLAYER_SLOT_END - PLAYER_SLOT_START)

#define TRADE_SLOT_COUNT             7
#define TRADE_SLOT_TRADED_COUNT      6
#define TRADE_SLOT_NONTRADED         6
*/

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
	uint32 _flags;
	uint32 _petInfoId;
	uint32 _petLevel;
	uint32 _petFamilyId;
	PlayerEnumItem _items[20];

private:

    
};

struct SpellBookEntry
{
    uint32 id;
    uint16 slot;
};

class Player : public Unit
{
public:
    Player();
    void Create(uint64);
    inline uint8 GetGender() { return GetUInt32Value(PLAYER_BYTES_3); }
    inline uint8 GetSkinId() { return (GetUInt32Value(PLAYER_BYTES) & 0x000000FF); }
    inline uint8 GetFaceId() { return (GetUInt32Value(PLAYER_BYTES) & 0x0000FF00) >> 8; }
    inline uint8 GetHairStyleId() { return (GetUInt32Value(PLAYER_BYTES) & 0x00FF0000) >> 16; }
    inline uint8 GetHairColorId() { return (GetUInt32Value(PLAYER_BYTES) & 0xFF000000) >> 24; }
    inline uint8 GetFaceTraitsId() { return (GetUInt32Value(PLAYER_BYTES_2) & 0x000000FF); }

private:

};


// class about the character that is used to login.
// needs to store known spells, action buttons,...
// basically everything that is needed to play.
class MyCharacter : public Player
{
public:
    MyCharacter();
    ~MyCharacter();

	void SetActionButtons(WorldPacket &data);
	void AddSpell(uint32 spellid, uint16 spellslot);
    void RemoveSpell(uint32 spellid);
    void ClearSpells(void) { _spells.clear(); }
    uint64 GetTarget(void) { return _target; }
    void SetTarget(uint64 guid) { _target = guid; } // should only be called by WorldSession::SendSetSelection() !!
    bool HasSpell(uint32 spellid);
    uint16 GetSpellSlot(uint32 spellid);

private:
	// bool _castingSpell; // this is something we dont really need for now



	std::vector<SpellBookEntry> _spells;
    uint64 _target; // currently targeted object
};


#endif
