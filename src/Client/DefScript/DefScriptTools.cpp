#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <math.h>
#include "DefScriptDefines.h"
#include "DefScriptTools.h"


std::string DefScriptTools::stringToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

std::string DefScriptTools::stringToUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}

std::string DefScriptTools::toString(ldbl num)
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

// convert a string into ldbl
// valid input formats:
// normal numbers: 5439
// hex numbers: 0xa56ff, 0XFF, 0xDEADBABE, etc (must begin with 0x)
// float numbers: 99.65, 0.025
// negative numbers: -100, -0x3d, -55.123
ldbl DefScriptTools::toNumber(std::string str)
{
    ldbl num=0;
    uint64 u=0;
    bool negative=false;
    if(str.empty())
        return 0;
    if(str[0]=='-')
    {
        str.erase(0,1);
        negative=true;
    }
    size_t ppos=str.find('.');
    str=stringToUpper(str);

    if(str.length() > 2 && str[0]=='0' && str[1]=='X')
    {
        std::string lo(str.c_str()+2);
        std::string hi;
        while(lo.length()>8)
        {
            hi+=lo.at(0);
            lo.erase(0,1);
        }
        unsigned int hibits,lobits;
        hibits=strtoul(hi.c_str(),NULL,16);
        lobits=strtoul(lo.c_str(),NULL,16);
        u|=hibits;
        u<<=32;
        u|=lobits;
    }
	else
		u = atoi64(str);

    if(ppos!=std::string::npos)
    {
		std::string mantissa("0");
		mantissa+=str.c_str()+ppos;
		num=(ldbl)atof(mantissa.c_str());
    }

    num=(long double)num + u;
	num=Round(num,10);


    if(negative)
        num = -num;
    return num;
}

bool DefScriptTools::isTrue(std::string s)
{
    if(s.empty() || s=="false" || s=="0")
        return false;
    return true;
}

uint64 DefScriptTools::toUint64(std::string str)
{
    bool negative=false;
    uint64 num = 0;
    if(str.empty())
        return 0;
    if(str[0]=='-')
    {
        str.erase(0,1);
        negative=true;
    }
    str=stringToUpper(str);
    if(str.length() > 2 && str[0]=='0' && str[1]=='X')
    {
        std::string lo(str.c_str()+2);
        std::string hi;
        while(lo.length()>8)
        {
            hi+=lo.at(0);
            lo.erase(0,1);
        }
        unsigned int hibits,lobits;
        hibits=strtoul(hi.c_str(),NULL,16);
        lobits=strtoul(lo.c_str(),NULL,16);
        num|=hibits;
        num<<=32;
        num|=lobits;
    }
    else
        num = atoi64(str);
    if(negative)
        num = (uint64)(-1) - num; // is this correct?
    return num;
}

uint64 DefScriptTools::atoi64(std::string str)
{
    uint64 l = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if(!isdigit(str[i]))
            return l;
        l = l * 10 + str[i] - 48;
    }
    return l;
}

inline long double DefScriptTools::Round(long double z,unsigned int n)
{
    static long double v[] = { 1, 10, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16 };
    return floor(z * v[n] + 0.5) / v[n];
}
