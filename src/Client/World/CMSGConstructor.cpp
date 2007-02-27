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

void WorldSession::SendSetSelection(uint64 guid)
{
    // TODO: MyCharacter.SetTarget(guid);
    logdebug("SetSelection GUID="I64FMT,guid);
    WorldPacket packet;
    packet << guid;
    packet.SetOpcode(CMSG_SET_SELECTION);
    SendWorldPacket(packet);
}



