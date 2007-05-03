#include "common.h"
#include "PseuWoW.h"
#include <map>
#include "Channel.h"

enum NotifyTypes
{
	JOINED          = 0x00,
	LEFT            = 0x01,
	YOUJOINED       = 0x02,
	YOULEFT         = 0x03,
	WRONGPASS       = 0x04,
	NOTON1          = 0x05, // Not on channel channel name.
	NOTMOD          = 0x06,
	SETPASS         = 0x07,
	CHANGEOWNER     = 0x08,
	NOTON2          = 0x09, // Player %s is not on channel.
	NOTOWNER        = 0x0A,
	WHOOWNER        = 0x0B,
	MODECHANGE      = 0x0C,
	ANNOUNCEON      = 0x0D,
	ANNOUNCEOFF     = 0x0E,
	MODERATED       = 0x0F,
	UNMODERATED     = 0x10,
	YOUCANTSPEAK    = 0x11,
	KICKED          = 0x12,
	YOUAREBANNED    = 0x13,
	BANNED          = 0x14,
	UNBANNED        = 0x15,
	UNKNOWN1        = 0x16, // is not banned
	ALREADYON       = 0x17,
	INVITED         = 0x18,
	WRONGALLIANCE   = 0x19, // target is in the wrong alliance for channel name
	UNKNOWN2        = 0x1A, // wrong alliance for channel name
	UNKNOWN3        = 0x1B, // invalid channel name
	ISNOTMODERATED  = 0x1C,
	YOUINVITED      = 0x1D,
	UNKNOWN4        = 0x1E, // %s has been banned.
	UNKNOWN5        = 0x1F, // The number of messages that can be sent to this channel is limited, please wait to send another message.
	UNKNOWN6        = 0x20  // You are in not the correct area for this channel.
};

enum PlayerChannelModes
{
	// 0x01 = ??
	MODERATOR = 0x02,
	MUTED = 0x04
};

typedef std::map<uint64,uint8> ChannelPlayerList;


void Channel::Join(std::string channel, std::string password)
{
	if (IsOnChannel(channel))
		return;

	// Send join channel request
	WorldPacket worldPacket;
	worldPacket.SetOpcode(CMSG_JOIN_CHANNEL);
	worldPacket << (uint32)0 << (uint8)0; // new since 2.0.x // uint32: some channel ID? server answers us with that number later if channel joined
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
			worldPacket << (uint32)0; // new since 2.0.x
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

			log("%s joined channel %s", channel.c_str());
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

			log("%s left channel %s", channel.c_str());
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

void Channel::RequestList(std::string ch)
{
	if(!IsOnChannel(ch))
		logdebug("Requesting list of not joined channel '%s'",ch.c_str());
	WorldPacket wp;
	wp.SetOpcode(CMSG_CHANNEL_LIST);
	wp << ch;
	_worldSession->SendWorldPacket(wp);
}

void Channel::HandleListRequest(WorldPacket& recvPacket)
{
	ChannelPlayerList cpl;
	uint8 unk;
	uint32 size;
	uint64 guid;
	uint8 mode;
	recvPacket >> unk >> size;
	for(uint32 i = 0; i < size; i++)
	{
		recvPacket >> guid >> mode;
		cpl[guid] = mode;
	}

	// now we could do something with that list, but for now, only request names of unknown players
	std::string pname;
	bool muted,mod;
	log("Player channel list, %u players:",size);
	for(ChannelPlayerList::iterator i = cpl.begin(); i != cpl.end(); i++)
	{
		pname = _worldSession->plrNameCache.GetName(i->first);
		mode = i->second;
		if(pname.empty())
		{
			pname="<unknown>";
			_worldSession->SendQueryPlayerName(i->first);
		}
		muted = mode & MUTED;
		mod = mode & MODERATOR;

		while(pname.length()<12)
			pname += " "; // for better formatting

		log("%s ["I64FMT"] %s %s",pname.c_str(),i->first,muted?"(muted)":"",mod?"(moderator)":"");
	}
}




