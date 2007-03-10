#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#include "DefScriptDefines.h"
#include "DefScriptTools.h"


std::string DefScriptTools::stringToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), std::tolower);
    return s;
}

std::string DefScriptTools::stringToUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), std::toupper);
    return s;
}

std::string DefScriptTools::toString(double num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string DefScriptTools::toString(uint64 num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}
// convert a string into double
// valid input formats:
// normal numbers: 5439
// hex numbers: 0xa56ff, 0XFF, 0xDEADBABE, etc (must begin with 0x)
// float numbers: 99.65, 0.025
// negative numbers: -100, -0x3d, -55.123
double DefScriptTools::toNumber(std::string str)
{
    double num;
    bool negative=false;
    if(str.empty())
        return 0;
    if(str[0]=='-')
    {
        str.erase(0,1);
        negative=true;
    }
    if(str.find('.')==std::string::npos)
    {
        str=stringToUpper(str);
        if(str.length() > 2 && str[0]=='0' && str[1]=='X')
            num = strtoul(&(str.c_str()[2]),NULL,16);
        else
            num = strtoul(str.c_str(),NULL,10);
    }
    else
    {
        num = strtod(str.c_str(),NULL);
    }
    if(negative)
        num = -num;
    return num;
}