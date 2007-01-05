#include "common.h"
#include "PseuWoW.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldSession.h"

void WorldSession::SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to){
    if((!_logged) || msg.empty())
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
			//DEBUG2(printf("W:CHAT->: \"%s\"\n",msg.c_str()););
			break;
		case CHAT_MSG_WHISPER:
            if(to.empty())
                return;
			packet<<to<<msg;
			break;
		case CHAT_MSG_CHANNEL:
            if(to.empty())
                return;
			// TODO: be sure the channel is joined before writing into this channel
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
}

void WorldSession::SendPing(uint32 ping){
    if(!_logged)
        return;
    WorldPacket packet;
    packet << ping;
    packet.SetOpcode(CMSG_PING);
    SendWorldPacket(packet);
    //DEBUG2(printf("Sent CMSG_PING, clock=%u\n",ping););
}

void WorldSession::SendEmote(uint32 id){
    if(!_logged)
        return;
    WorldPacket packet;
    packet << id << id << _targetGUID; // TODO: correct this!
    packet.SetOpcode(CMSG_TEXT_EMOTE);
    SendWorldPacket(packet);
}




