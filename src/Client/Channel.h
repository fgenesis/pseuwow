#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "SysDefs.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"

struct ChannelStruct
{
	std::string channel;
	std::string password;
	bool joined;
};

class Channel
{
public:
	Channel(WorldSession *worldSession)
	{
		_worldSession = worldSession;
	}

	void Join(std::string channel, std::string password);
	void Leave(std::string channel);
	void Say(std::string channel, std::string text, uint32 lang);
	bool IsOnChannel(std::string channel);
	void HandleNotifyOpcode(WorldPacket &packet);
	
	// TODO: Add Kick/Ban/Mode/Owner/Mute/Invite and all that stuff

private:
	std::vector<ChannelStruct>channels;
	WorldSession *_worldSession;
};

#endif