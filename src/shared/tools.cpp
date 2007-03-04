#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <time.h>
#include "tools.h"

void printchex(std::string in, bool spaces=true){
	unsigned int len=0,i;
    len=in.length();
	printf("[");
	if(spaces)
		for(i=0;i<len;i++)printf("%x ",(unsigned char)in[i]); 
	else
		for(i=0;i<len;i++)printf("%x",(unsigned char)in[i]); 
	printf("]\n");
}

void printchex(char *in, uint32 len, bool spaces=true){
	unsigned int i;
	printf("[");
	if(spaces)
		for(i=0;i<len;i++)printf("%x ",(unsigned char)in[i]); 
	else
		for(i=0;i<len;i++)printf("%x",(unsigned char)in[i]); 
	printf("]\n");
}

std::string stringToLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
	return s;
}

std::string stringToUpper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::toupper);
	return s;
}

std::string toString(uint64 num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string getDateString(void)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    char str[30];
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    sprintf(str,"%-4d-%02d-%02d %02d:%02d:%02d ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
    return std::string(str);
}

uint64 toInt(std::string str)
{
    if(str.empty())
        return 0;
    str=stringToUpper(str);
    if(str.length() > 2 && str[0]=='0' && str[1]=='X')
        return strtoul(&(str.c_str()[2]),NULL,16);
    else
        return strtoul(str.c_str(),NULL,10);
}

