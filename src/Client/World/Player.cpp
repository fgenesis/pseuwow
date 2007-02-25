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


/*void PlayerSettings::SetSpells(WorldPacket &data)
{
	if (!init)
		return;

	uint8 unk;
	uint16 numSpells;

	data >> unk >> numSpells;

	logdetail("Got %d spells", numSpells);

	// TODO: Finish implenting this
}

void PlayerSettings::CastSpell(uint32 spellId, uint64 target)
{
	if (castingSpell || !init)
		return;

	castingSpell = !castingSpell;

	WorldPacket packet;
	packet.SetOpcode(CMSG_CAST_SPELL);
	packet << spellId << (uint16)2  << (uint8)1 << (uint8)target; // 2 = TARGET_FLAG_UNIT
	// Can be bugged, not fully tested, probably doesn't work when the guid is high
	// Damn packed guid stuff! xD

	_worldSession->SendWorldPacket(packet);
}

void PlayerSettings::HandleCastResultOpcode(WorldPacket &packet)
{
	if (!init)
		return;

	uint32 spellId;
	uint8 statusFail;
	uint8 failProblem;
	char l[150];

	packet >> spellId >> statusFail;

	castingSpell = false;

	sprintf(l, "Received cast result opcode. Spell = %d, statusFail = %d", spellId, statusFail);

	if (statusFail == 2) // Spell cast failed
	{
		packet >> failProblem;
		sprintf(l, "%s, failProblem = %d", l, failProblem);
	}


	//logdetail(l);
}*/