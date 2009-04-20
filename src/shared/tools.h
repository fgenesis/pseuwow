
#ifndef _TOOLS_H
#define _TOOLS_H

#include "common.h"

#define M_SETBIT(var,bit) ( (var)|=(1<<(bit)) )
#define M_UNSETBIT(var,bit) ( (var)&=(~(1<<(bit))) )
#define M_HASBIT(var,bit) ( (var)&(1<<(bit)) )


void printchex(std::string,bool);
void printchex(char *in, uint32 len, bool);
std::string stringToUpper(std::string);
std::string stringToLower(std::string);
std::string toString(uint64);
std::string getDateString(void);
std::string GetTimeString(void);
uint64 toInt(std::string);
std::string toHexDump(uint8* array,uint32 size,bool spaces=true,uint32 per_line=0);
std::deque<std::string> GetFileList(std::string);
bool FileExists(std::string);
bool CreateDir(const char*);
uint32 getMSTime(void);
uint32 GetFileSize(const char*);
void _FixFileName(std::string&);
std::string _PathToFileName(std::string);
std::string NormalizeFilename(std::string);
std::string FilesizeFormat(uint32);
std::string GetWorkingDir(void);
bool SetWorkingDir(const char*);
std::string GetAbsolutePath(const char*);

#endif
