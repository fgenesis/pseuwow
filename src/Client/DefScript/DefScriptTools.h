#ifndef _DEFSCRIPTTOOLS_H
#define _DEFSCRIPTTOOLS_H

namespace DefScriptTools
{
    std::string stringToUpper(std::string);
    std::string stringToLower(std::string);
    std::string toString(ldbl);
    std::string toString(uint64);
    ldbl toNumber(std::string);
    bool isTrue(std::string);
    uint64 toUint64(std::string);
    uint64 atoi64(const char*);
}



#endif
