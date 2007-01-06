#ifndef _TOOLS_H
#define _TOOLS_H

#include "common.h"


void printchex(std::string,bool);
void printchex(char *in, uint32 len, bool);
std::string stringToUpper(std::string);
std::string stringToLower(std::string);
std::string toString(uint64);
std::string getDateString(void);

#endif