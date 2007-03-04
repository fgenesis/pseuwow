#include <vector>
#include <fstream>
#include "common.h"
#include "PseuWoW.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "Player.h"
#include "NameTables.h"
#include "DefScript/DefScript.h"
#include "WorldSession.h"


Player::Player() : Unit()
{
    _type = TYPE_PLAYER;
    _typeid = TYPEID_PLAYER;
    _valuescount = PLAYER_END;
}

void Player::Create(uint64 guid)
{
    Object::Create(guid);
}

MyCharacter::MyCharacter()
{
	_castingSpell = false;
}

void MyCharacter::SetActionButtons(WorldPacket &data)
{

}

void MyCharacter::SetSpells(WorldPacket &data)
{
	uint8 unk;
	uint16 spellid,spellslot,count;
	data >> unk >> count;
	logdebug("Got initial spells list, %u spells.",count);
	for(uint16 i = 0; i < count; i++)
	{
		data >> spellid >> spellslot;
		logdebug("Initial Spell: id=%u slot=%u",spellid,spellslot);

		spell _spell;
		_spell.spellId = spellid;
		_spell.spellSlot = spellslot;

		_spells.push_back(_spell);
	}
}

void MyCharacter::CastSpell(uint32 spellId, uint64 target)
{
	/*
	if (_castingSpell)
		return;

	_castingSpell = !_castingSpell;

	WorldPacket packet;
	packet.SetOpcode(CMSG_CAST_SPELL);
	packet << spellId << (uint16)2  << (uint8)1 << (uint8)target; // 2 = TARGET_FLAG_UNIT
	// Can be bugged, not fully tested, probably doesn't work when the guid is high
	// Damn packed guid stuff! xD

	_worldSession->SendWorldPacket(packet);
	*/
}

void MyCharacter::HandleCastResultOpcode(WorldPacket &packet)
{
	/*
	uint32 spellId;
	uint8 statusFail;
	uint8 failProblem;
	char l[150];

	packet >> spellId >> statusFail;

	_castingSpell = false;

	sprintf(l, "Received cast result opcode. Spell = %d, statusFail = %d", spellId, statusFail);

	if (statusFail == 2) // Spell cast failed
	{
		packet >> failProblem;
		sprintf(l, "%s, failProblem = %d", l, failProblem);
	}


	//logdetail(l);
	*/
}