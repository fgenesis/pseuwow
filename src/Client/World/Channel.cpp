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

enum ChannelFlags
{
    CHANNEL_FLAG_CUSTOM     = 0x01,
    // 0x02
    CHANNEL_FLAG_TRADE      = 0x04,
    CHANNEL_FLAG_NOT_LFG    = 0x08,
    CHANNEL_FLAG_GENERAL    = 0x10,
    CHANNEL_FLAG_CITY       = 0x20,
    CHANNEL_FLAG_LFG        = 0x40,
    CHANNEL_FLAG_VOICE      = 0x80
    // General                  0x18 = 0x10 | 0x08
    // Trade                    0x3C = 0x20 | 0x10 | 0x08 | 0x04
    // LocalDefence             0x18 = 0x10 | 0x08
    // GuildRecruitment         0x38 = 0x20 | 0x10 | 0x08
    // LookingForGroup          0x50 = 0x40 | 0x10
};

enum ChannelMemberFlags
{
    MEMBER_FLAG_NONE        = 0x00,
    MEMBER_FLAG_OWNER       = 0x01,
    MEMBER_FLAG_MODERATOR   = 0x02,
    MEMBER_FLAG_VOICED      = 0x04,
    MEMBER_FLAG_MUTED       = 0x08,
    MEMBER_FLAG_CUSTOM      = 0x10,
    MEMBER_FLAG_MIC_MUTED   = 0x20,
    // 0x40
    // 0x80
};

typedef std::map<uint64,uint8> ChannelPlayerList;


void Channel::Join(std::string channel, std::string password)
{
	if (IsOnChannel(channel))
		return;

	// Send join channel request
	WorldPacket worldPacket;
	worldPacket.SetOpcode(CMSG_JOIN_CHANNEL);
	worldPacket << (uint32)0; // new since 2.0.x, some channel ID? server answers us with that number later if channel joined
    worldPacket << (uint8)0;  // unk
    worldPacket << (uint8)0;  // unk, new since 2.2.x
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
			worldPacket << (uint32)0; // new since 2.0.x, maybe channel id
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
		case JOINED:
			packet >> guid;
			if(guid)
            {
				name = _worldSession->GetOrRequestPlayerName(guid);
				if (name.empty())
				{
					_worldSession->_DelayWorldPacket(packet, uint32(_worldSession->GetLagMS() * 1.2f));
					return;
				}
			}

			log("%s joined channel %s",name.c_str(),channel.c_str());
			break;

		// Player leaved channel you are on
		case LEFT:
			packet >> guid;
			if(guid)
            {
                name = _worldSession->GetOrRequestPlayerName(guid);
                if (name.empty())
                {
                    _worldSession->_DelayWorldPacket(packet, uint32(_worldSession->GetLagMS() * 1.2f));
                    return;
                }
			}

			log("%s left channel %s", name.c_str(), channel.c_str());
			break;

		// You joined channel successfully
		case YOUJOINED:
			log("Joined channel %s", channel.c_str());
            channels.push_back(channel);
			break;

		// You leaved channel successfully
		case YOULEFT:
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
		case WRONGPASS:
			log("Could not join channel %s (Wrong password)", channel.c_str());
			break;

		// Not on channel while trying to write to channel etc.
		case NOTON1:
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
	uint8 mode, flags; // mode: player flags; flags: channel flags
    std::string name;
    bool must_delay = false;

	recvPacket >> unk >> name >> flags >> size;

	for(uint32 i = 0; i < size; i++)
	{
		recvPacket >> guid >> mode;
        // all player names in this packet must be known before we can continue
        if(_worldSession->GetOrRequestPlayerName(guid).empty())
        {
            must_delay = true;
        }
		cpl[guid] = mode;
	}
    if(must_delay)
    {
        _worldSession->_DelayWorldPacket(recvPacket, uint32(_worldSession->GetLagMS() * 1.2f));
        return;
    }

    // store list of GUIDs in: @ChannelList - see below
    DefList *l = _worldSession->GetInstance()->GetScripts()->lists.Get("@ChannelList");
    l->clear();

	std::string pname;
	bool muted,mod;
	logcustom(0,WHITE,"Player channel list, %u players:",size);
	for(ChannelPlayerList::iterator i = cpl.begin(); i != cpl.end(); i++)
	{
		pname = _worldSession->GetOrRequestPlayerName(i->first); // all names should be known now
		mode = i->second;
		if(pname.empty())
            pname = "<unknown>";

		muted = mode & MEMBER_FLAG_MUTED;
		mod = mode & MEMBER_FLAG_MODERATOR;

		while(pname.length() < MAX_PLAYERNAME_LENGTH)
			pname += " "; // for better formatting

		logcustom(0,WHITE,"%s ["I64FMT"] %s %s",pname.c_str(),i->first,muted?"(muted)":"",mod?"(moderator)":"");

        // DefScript binding
        l->push_back(DefScriptTools::toString(guid));
	}
}





