#include "common.h"
#include "PseuWoW.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldSession.h"
#include "Channel.h"

void WorldSession::SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to){
    if((!_valid) || (!_logged) || msg.empty())
        return;
	WorldPacket packet;
	packet<<type<<lang;
	switch(type){
		case CHAT_MSG_SAY:
		case CHAT_MSG_YELL:
		case CHAT_MSG_PARTY:
		case CHAT_MSG_GUILD:
		case CHAT_MSG_OFFICER: // not sure about that
			packet<<msg;
			break;
		case CHAT_MSG_WHISPER:
            if(to.empty())
                return;
			packet<<to<<msg;
			break;
		case CHAT_MSG_CHANNEL:
            if(to.empty() /*|| !_channels->IsOnChannel(to)*/)
                return;
			packet<<to<<msg;
			break;
	}
    packet.SetOpcode(CMSG_MESSAGECHAT);
	SendWorldPacket(packet);
}

void WorldSession::SendQueryPlayerName(uint64 guid){
    if((!_logged) || guid==0)
        return;
    WorldPacket packet;
    packet << guid;
    packet.SetOpcode(CMSG_NAME_QUERY);
    SendWorldPacket(packet);
    // to prevent opcode spam, we need to make a list with already requested names
}

void WorldSession::SendPing(uint32 ping){
    if(!_logged)
        return;
    WorldPacket packet;
    packet << ping;
    packet.SetOpcode(CMSG_PING);
    SendWorldPacket(packet);
}

void WorldSession::SendEmote(uint32 id){
    if(!_logged)
        return;
    WorldPacket packet;
    packet << id << id << _targetGUID; // TODO: correct this!
    packet.SetOpcode(CMSG_TEXT_EMOTE);
    SendWorldPacket(packet);
}

void WorldSession::SendQueryItem(uint32 id, uint64 guid) // is it a guid? not sure
{
    if(objmgr.ItemNonExistent(id))
    {
        logdebug("Skipped query of item %u (was marked as nonexistent before)",id);
        return;
    }
    logdebug("Sending Item query, id=%u",id);
    WorldPacket packet;
    packet << id << guid;
    packet.SetOpcode(CMSG_ITEM_QUERY_SINGLE);
    SendWorldPacket(packet);
}

// use ONLY this function to target objects and notify the server about it.
// (server & client need to stay synced)
void WorldSession::SendSetSelection(uint64 guid)
{
    ASSERT(GetMyChar()) // we need to be logged in to select something
    if(guid==GetMyChar()->GetTarget())
        return; // no need to select already selected target
    GetMyChar()->SetTarget(guid);
    logdebug("SetSelection GUID="I64FMT,guid);
    WorldPacket packet;
    packet << guid;
    packet.SetOpcode(CMSG_SET_SELECTION);
    SendWorldPacket(packet);
}

void WorldSession::SendCastSpell(uint32 spellid)
{
    if(!spellid)
        return;
    MyCharacter *my = GetMyChar();
    bool known = my->HasSpell(spellid);
    if( (!known) && (!GetInstance()->GetConf()->disablespellcheck) )
    {
        logerror("Attempt to cast not-known spell %u",spellid);
        return;
    }

    Object *target = objmgr.GetObj(my->GetTarget());

    if(!target) // this is wrong, some spells dont require a target (areaspells, self-only spells)
        return; // but for now, this should be ok, until a db is used that provides spell info

    WorldPacket packet;
    ByteBuffer temp;
    uint16 flags=TARGET_FLAG_SELF; // target mask. spellcast implementeation needs to be changed if TARGET_MASK_SELF is != 0
    packet << spellid;
    if(my->GetTarget() != GetGuid()) // self cast?
    {
        if(target->GetTypeId() == TYPEID_PLAYER || target->GetTypeId() == TYPEID_UNIT)
        {
            flags |= TARGET_FLAG_UNIT;
            temp << (uint8)0xFF << my->GetTarget(); // need to send packed guid?
        }     
        if(target->GetTypeId() == TYPEID_OBJECT)
        {
            flags |= TARGET_FLAG_OBJECT;
            temp << (uint8)0xFF <<my->GetTarget(); // need to send packed guid?
        }
        // TODO: need implementation of areaspells & item targets (enchant) here (temp << itemGUID)!
        // TODO: append floats x,y,z according to target type srcloc & dstloc to temp
        // TODO: append string to temp if TARGET_FLAG_STRING is set. what string for what purpose??
        // and whats with TARGET_CORPSE?
    }
    packet << flags;
    packet.append(temp);

    // cast it
    packet.SetOpcode(CMSG_CAST_SPELL);
    SendWorldPacket(packet);
    logdetail("Casting spell %u on target "I64FMT,spellid,my->GetTarget());
    if(!known)
        logerror(" - WARNING: spell is NOT known!");
}



