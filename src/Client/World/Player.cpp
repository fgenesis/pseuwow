#include <vector>
#include <fstream>
#include "common.h"
#include "PseuWoW.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "Player.h"
#include "DefScript/DefScript.h"
#include "WorldSession.h"


Player::Player() : Unit()
{
    _type |= TYPE_PLAYER;
    _typeid = TYPEID_PLAYER;
    _valuescount = PLAYER_END;
}

void Player::Create(uint64 guid)
{
    Object::Create(guid);
}

MyCharacter::MyCharacter() : Player()
{
    DEBUG(logdebug("MyCharacter() constructor, this=0x%x",this)); 
    SetTarget(0);
}

MyCharacter::~MyCharacter()
{
    DEBUG(logdebug("~MyCharacter() destructor, this=0x%X guid="I64FMT,this,GetGUID())); // this _could_ crash if Player::Create(guid) wasnt called before!
}

void MyCharacter::SetActionButtons(WorldPacket &data)
{

}

void MyCharacter::AddSpell(uint32 spellid, uint16 spellslot)
{
	SpellBookEntry _spell;
	_spell.id = spellid;
	_spell.slot = spellslot;

	_spells.push_back(_spell);
}

void MyCharacter::RemoveSpell(uint32 spellid)
{
    for(std::vector<SpellBookEntry>::iterator i=_spells.begin(); i != _spells.end(); i++)
        if(i->id == spellid)
        {
            _spells.erase(i);
            break;
        }
}

bool MyCharacter::HasSpell(uint32 spellid)
{
    for(std::vector<SpellBookEntry>::iterator i=_spells.begin(); i != _spells.end(); i++)
        if(i->id == spellid)
            return true;
    return false;
}


uint16 MyCharacter::GetSpellSlot(uint32 spellid)
{
    for(std::vector<SpellBookEntry>::iterator i=_spells.begin(); i != _spells.end(); i++)
        if(i->id == spellid)
            return i->slot;
    return 0;
}
