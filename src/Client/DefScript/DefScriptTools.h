#ifndef _DEFSCRIPTTOOLS_H
#define _DEFSCRIPTTOOLS_H

#include <sstream>

namespace DefScriptTools
{
    std::string stringToUpper(std::string);
    std::string stringToLower(std::string);
    std::string toString(ldbl);
    inline std::string toString(double num) { return toString(ldbl(num)); }
    inline std::string toString(float num) { return toString(ldbl(num)); }

    template <class T> inline std::string toString(T num)
    {
        std::stringstream ss;
        ss << num;
        return ss.str();
    }

    ldbl toNumber(std::string);
    bool isTrue(std::string);
    uint64 toUint64(std::string);
    uint64 atoi64(std::string);
	inline long double Round(long double z,unsigned int n);
}



#endif
