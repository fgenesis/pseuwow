#ifndef _TOOLS_H
#define _TOOLS_H

#include "common.h"

#define SETBIT(var,bit) ( (var)|=(1<<(bit)) )
#define UNSETBIT(var,bit) ( (var)&=(~(1<<(bit))) )
#define HASBIT(var,bit) ( (var)&(1<<(bit)) )

void printchex(std::string,bool);
void printchex(char *in, uint32 len, bool);
std::string stringToUpper(std::string);
std::string stringToLower(std::string);
std::string toString(uint64);
std::string getDateString(void);
uint64 toInt(std::string);
std::string toHexDump(uint8* array,uint32 size,bool spaces=true);
std::deque<std::string> GetFileList(std::string);

#endif