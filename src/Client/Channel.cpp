#include "common.h"
#include "PseuWoW.h"

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

	// Put in channels-joined list, but don't flag as joined yet
	ChannelStruct channelStruct;
	channelStruct.channel = "TestChannel";
	channelStruct.password = "";
	channelStruct.joined = false;

	channels.push_back(channelStruct);
}

void Channel::Leave(std::string channel)
{
	for(std::vector<ChannelStruct>::iterator i = channels.begin(); i != channels.end(); i++)
	{
		ChannelStruct s = *(i);
		if (s.channel == channel)
		{
			if (s.joined)
			{
				// Send leave channel request
				WorldPacket worldPacket;
				worldPacket.SetOpcode(CMSG_LEAVE_CHANNEL);
				worldPacket << channel;
				_worldSession->SendWorldPacket(worldPacket);
			}
		}
	}
}

void Channel::Say(std::string channel, std::string text, uint32 lang)
{
	_worldSession->SendChatMessage(CHAT_MSG_CHANNEL, lang, text, channel);
}

bool Channel::IsOnChannel(std::string channel)
{
	for(std::vector<ChannelStruct>::iterator i = channels.begin(); i != channels.end(); i++)
	{
		ChannelStruct s = *(i);
		if (s.channel == channel)
		{
			if (s.joined)
			{
				return true;
			}
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

	for(std::vector<ChannelStruct>::iterator i = channels.begin(); i != channels.end(); i++)
	{
		ChannelStruct s = *(i);
		if (s.channel == channel)
		{
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

					printf("%s joined channel %s\n", channel.c_str());
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

					printf("%s leaved channel %s\n", channel.c_str());
					break;

				// You joined channel successfully
				case 0x02:
					s.joined = true;
					printf("You joined channel %s\n", channel.c_str());
					break;

				// You leaved channel successfully
				case 0x03:
					channels.erase(i);
					printf("You leaved channel %s\n", channel.c_str());
					break;

				// Wrong password while trying to join channel
				case 0x04:
					channels.erase(i);
					printf("Could not join channel %s (Wrong password)\n", channel.c_str());
					break;

				// Not on channel while trying to write to channel etc.
				case 0x05:
					channels.erase(i);
					printf("Your are not on channel %s\n", channel.c_str());
					break;
			}
			return;
		}
	}

	// TODO: Even more channel notices to handle
	/*
	printf("Channel notice not handled! Code: %d - Channel name: %s\nData:\n", code, channel.c_str());
	packet.textlike();
	printf("\n");
	*/
}
