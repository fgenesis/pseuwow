#include "common.h"
#include "PseuWoW.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldSession.h"
#include "Channel.h"

void WorldSession::SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to)
{
    if((!_logged) || msg.empty())
        return;
	WorldPacket packet;
	packet << type << lang;
	switch(type){
		case CHAT_MSG_WHISPER:
            if(to.empty())
                return;
			packet << to << msg;
			break;
		case CHAT_MSG_CHANNEL:
            if(to.empty() /*|| !_channels->IsOnChannel(to)*/)
                return;
			packet << to << msg;
			break;
        default:
            packet << msg;
	}
    packet.SetOpcode(CMSG_MESSAGECHAT);
	SendWorldPacket(packet);
}

void WorldSession::SendQueryPlayerName(uint64 guid)
{
    if((!_logged) || guid==0)
        return;
    WorldPacket packet;
    packet << guid;
    packet.SetOpcode(CMSG_NAME_QUERY);
    SendWorldPacket(packet);
    // to prevent opcode spam, we need to make a list with already requested names
}

void WorldSession::SendPing(uint32 ping)
{
    if(!_logged)
        return;
    WorldPacket packet;
    packet << ping;
    packet << GetLagMS();
    packet.SetOpcode(CMSG_PING);
    SendWorldPacket(packet);
}

void WorldSession::SendEmote(uint32 id)
{
    if(!_logged)
        return;
    WorldPacket packet;
    int32 variation = 0; // randomized usually
    packet << id << (uint32)variation << GetMyChar()->GetTarget();
    packet.SetOpcode(CMSG_TEXT_EMOTE);
    SendWorldPacket(packet);
}

void WorldSession::SendQueryItem(uint32 entry, uint64 guid) // is it a guid? not sure
{
    if(objmgr.ItemNonExistent(entry))
    {
        logdebug("Skipped query of item %u (was marked as nonexistent before)",entry);
        return;
    }
    logdebug("Sending Item query, id=%u",entry);
    WorldPacket packet;
    packet << entry << guid;
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

void WorldSession::SendCastSpell(uint32 spellid, bool nocheck)
{
    if(!spellid)
        return;
    MyCharacter *my = GetMyChar();
    bool known = nocheck ? true : my->HasSpell(spellid);

    //-- TEMP FIX --// TODO: need a list of spells that is excluded from the check and can always be casted.
    // settable via DefScript?
    if(spellid==836)
        known=true;

    if( (!known) && (!GetInstance()->GetConf()->disablespellcheck) )
    {
        logerror("Attempt to cast not-known spell %u",spellid);
        return;
    }

    Object *target = objmgr.GetObj(my->GetTarget());

    //if(!target) // this is wrong, some spells dont require a target (areaspells, self-only spells)
    //    return; // but for now, this should be ok, until a db is used that provides spell info

    WorldPacket packet;
    ByteBuffer temp;
    uint32 flags=TARGET_FLAG_SELF; // target mask. spellcast implementeation needs to be changed if TARGET_MASK_SELF is != 0
    packet << (uint8)0; // unk
    packet << spellid;
    packet << (uint8)0; // unk
    
    if(target && my->GetTarget() != GetGuid()) // self cast?
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
        logcustom(1,LRED," - WARNING: spell is NOT known!");
}

void WorldSession::SendWhoListRequest(uint32 minlvl, uint32 maxlvl, uint32 racemask, uint32 classmask, std::string name, std::string guildname, std::vector<uint32> *zonelist, std::vector<std::string> *strlist)
{
    WorldPacket pkt(CMSG_WHO, 50); // guess size
    pkt << minlvl;
    pkt << maxlvl;
    pkt << name;
    pkt << guildname;
    pkt << racemask;
    pkt << classmask;

    if(zonelist)
    {
        pkt << (uint32)zonelist->size();
        for(uint32 i = 0; i < zonelist->size(); i++)
            pkt << (*zonelist)[i];
    }
    else
        pkt << uint32(0);

    if(strlist)
    {
        pkt << (uint32)strlist->size();
        for(uint32 i = 0; i < strlist->size(); i++)
            pkt << (*strlist)[i];
    }
    else
        pkt << uint32(0);

    SendWorldPacket(pkt);
}

void WorldSession::SendQueryCreature(uint32 entry, uint64 guid)
{
    if(objmgr.CreatureNonExistent(entry))
    {
        logdebug("Skipped query of creature %u (was marked as nonexistent before)",entry);
        return;
    }
    logdebug("Sending creature query, id=%u",entry);
    WorldPacket wp(CMSG_CREATURE_QUERY,4+8);
    wp << entry << guid;
    SendWorldPacket(wp);
}

void WorldSession::SendQueryGameobject(uint32 entry, uint64 guid)
{
    if(objmgr.GONonExistent(entry))
    {
        logdebug("Skipped query of gameobject %u (was marked as nonexistent before)",entry);
        return;
    }
    logdebug("Sending gameobject query, id=%u",entry);
    WorldPacket wp(CMSG_GAMEOBJECT_QUERY,4+8);
    wp << entry << guid;
    SendWorldPacket(wp);
}

void WorldSession::SendCharCreate(std::string name, uint8 race, uint8 class_, // below here all values default is 0
                    uint8 gender, uint8 skin, uint8 face,
                    uint8 hairstyle, uint8 haircolor, uint8 facial, uint8 outfit)
{
    log("Creating Character '%s', race=%u, class=%u gender=%u", name.c_str(), race, class_, gender);
    WorldPacket wp(CMSG_CHAR_CREATE, name.length()+1 + 9);
    wp << name;
    wp << race;
    wp << class_;
    wp << gender;
    wp << skin;
    wp << face;
    wp << hairstyle;
    wp << haircolor;
    wp << facial;
    wp << outfit;
    AddSendWorldPacket(wp);
}


