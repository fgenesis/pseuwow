#ifndef _DEFSCRIPTTOOLS_H
#define _DEFSCRIPTTOOLS_H

#include <sstream>

namespace DefScriptTools
{
    std::string stringToUpper(std::string);
    std::string stringToLower(std::string);
    ldbl toNumber(std::string);
    bool isTrue(std::string);
    uint64 toUint64(std::string);
    uint64 atoi64(const char*);
	inline long double Round(long double z,unsigned int n);
    template <class T> inline std::string toString(T num)
    {
        std::stringstream ss;
        ss << num;
        return ss.str();
    }
    template <ldbl> inline std::string toString(ldbl num)
    {
        std::stringstream ss;
        ss.setf(std::ios_base::fixed);
        ss.precision(15);
        ss << Round(num,15);
        std::string s(ss.str());
        while(s[s.length()-1]=='0')
            s.erase(s.length()-1,1);
        if(s[s.length()-1]=='.')
            s.erase(s.length()-1,1);
        return s;
    }
    template <float> inline std::string toString(float num)
    {
        return toString<ldbl>(num);
    }

}



#endif
