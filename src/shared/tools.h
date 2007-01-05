#ifndef _TOOLS_H
#define _TOOLS_H

#include "common.h"


// old obsoelete functions
//char *triml(char*,int);
//void nullify(char*,int);
void printchex(std::string,bool);
//char *strl(char*,int);
//char *strr(char*,int);
//char *trimr(char*,int);
//char *genrndstr(int);
//char *StrToHex(char*,int);
//char *HexToStr(char*,int);
//char *NewNullString(int);
//char *Reverse(char*,int);
//void rawcpy(char*,char*,int);
//void rawcat(char*,char*,int,int);
//void TrimQuotes(char*);

// new functions
std::string stringToUpper(std::string);
std::string stringToLower(std::string);
std::string toString(uint64 num);
std::string toString(int64 num);
std::string getDateString(void);

#endif