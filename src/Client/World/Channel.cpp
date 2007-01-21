#include "common.h"
#include "PseuWoW.h"
#include <map>
#include "Channel.h"

void Channel::Join(std::string channel, std::string password)
{
	if (IsOnChannel(channel))
		return;

	// Send join channel request
	WorldPacket worldPacket;
	worldPacket.SetOpcode(CMSG_JOIN_CHANNEL);
	worldPacket << channel << password;
	_worldSession->SendWorldPacket(worldPacket);
}

void Channel::Leave(std::string channel)
{
    for(std::vector<std::string>::iterator i = channels.begin(); i != channels.end(); i++)
	{
        if (*i == channel)
		{
		    // Send leave channel request
		    WorldPacket worldPacket;
		    worldPacket.SetOpcode(CMSG_LEAVE_CHANNEL);
		    worldPacket << channel;
		    _worldSession->SendWorldPacket(worldPacket);
            return;
		}
	}
    log("Can't leave channel \"%s\": not joined",channel.c_str());
}

void Channel::Say(std::string channel, std::string text, uint32 lang)
{
	_worldSession->SendChatMessage(CHAT_MSG_CHANNEL, lang, text, channel);
}

bool Channel::IsOnChannel(std::string channel)
{
    for(std::vector<std::string>::iterator i = channels.begin(); i != channels.end(); i++)
	{
		if (*i == channel)
		{
		    return true;
		}
	}
	return false;
}

void Channel::HandleNotifyOpcode(WorldPacket &packet)
{
	uint8 code;
	uint64 guid;

	std::string channel, name;

	packet >> code >> channel;

	switch (code)
	{
		// Player joined channel you are on
		case 0x00:
			packet >> guid;
			if(guid){
				name = _worldSession->plrNameCache.GetName(guid);
				if (name.empty())
				{
					_worldSession->SendQueryPlayerName(guid);
					name = "Unknown Entity";
				}
			}

			logdetail("%s joined channel %s", channel.c_str());
			break;

		// Player leaved channel you are on
		case 0x01:
			packet >> guid;
			if(guid){
				name = _worldSession->plrNameCache.GetName(guid);
				if (name.empty())
				{
					_worldSession->SendQueryPlayerName(guid);
					name = "Unknown Entity";
				}
			}

			logdetail("%s leaved channel %s", channel.c_str());
			break;

		// You joined channel successfully
		case 0x02:
			log("Joined channel %s", channel.c_str());
            channels.push_back(channel);
			break;

		// You leaved channel successfully
		case 0x03:
            for(std::vector<std::string>::iterator i = channels.begin(); i != channels.end(); i++)
            {
                if(*i == channel)
                {
                    channels.erase(i);
                    break;
                }
            }
			log("Left channel %s", channel.c_str());
			break;

		// Wrong password while trying to join channel
		case 0x04:
			log("Could not join channel %s (Wrong password)", channel.c_str());
			break;

		// Not on channel while trying to write to channel etc.
		case 0x05:
			log("You are not on channel %s", channel.c_str());
			break;
	}

	// TODO: Even more channel notices to handle
	/*
	printf("Channel notice not handled! Code: %d - Channel name: %s\nData:\n", code, channel.c_str());
	packet.textlike();
	printf("\n");
	*/
}
