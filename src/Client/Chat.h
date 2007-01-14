#ifndef _CHAT_H
#define _CHAT_H

#include "SysDefs.h"

class Chat
{
public:
	Chat(std::string sentence);
	std::string GetResult();

private:
	std::string result;
	void Answer(std::string sentence, std::string type);
};

#endif