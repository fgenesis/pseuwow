#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <fstream>
#include <errno.h>
#include "tools.h"

#if PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <mmsystem.h>
#   include <time.h>
#else
#   include <sys/dir.h>
#   if defined(__FreeBSD__) || defined(__APPLE_CC__)
#       include <time.h>
#   endif
#   include <sys/timeb.h>
#endif


void printchex(std::string in, bool spaces=true)
{
	unsigned int len=0,i;
    len=in.length();
	printf("[");
	if(spaces)
		for(i=0;i<len;i++)printf("%x ",(unsigned char)in[i]); 
	else
		for(i=0;i<len;i++)printf("%x",(unsigned char)in[i]); 
	printf("]\n");
}

void printchex(char *in, uint32 len, bool spaces=true)
{
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

std::string toHexDump(uint8* array,uint32 size,bool spaces)
{
    std::stringstream ss;
    char buf[5];
    for(uint32 i=0;i<size;i++)
    {
        sprintf(buf,(array[i]<0x0F)?"0%X":"%X",(uint32)array[i]);
        ss << buf;
        if(spaces)
            ss << ' ';
    }
    return ss.str();
}

std::deque<std::string> GetFileList(std::string path)
{
    std::deque<std::string> files;

# ifndef _WIN32 // TODO: fix this function for linux if needed
        const char *p = path.c_str();
    DIR * dirp;
    struct dirent * dp;
    dirp = opendir(p);
    while (dirp)
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
            files.push_back(std::string(dp->d_name));
        else
            break;
    }
    if(dirp)
        closedir(dirp);

# else

    if(path.at(path.length()-1)!='/')
        path += "/";
    path += "*.*";
    const char *p = path.c_str();
    WIN32_FIND_DATA fil;
    HANDLE hFil=FindFirstFile(p,&fil);
    if(hFil!=INVALID_HANDLE_VALUE)
    {
        files.push_back(std::string(fil.cFileName));
        while(FindNextFile(hFil,&fil))
            files.push_back(std::string(fil.cFileName));
    }

# endif

    while(files.size() && (files.front()=="." || files.front()==".."))
        files.pop_front();

    return files;
}

bool FileExists(std::string fn)
{
	std::fstream f;
	f.open(fn.c_str(),std::ios_base::in);
	if (f.is_open())
	{
		f.close();
		return true;
	}
	return false;
}

// must return true if creating the directory was successful
bool CreateDir(const char *dir)
{
	bool result;
# ifdef _WIN32
	result = ::CreateDirectory(dir,NULL);
# else
	// NOT tested for Linux!! whats the return value on success?
	// TODO: fix me!
	result = mkdir(dir);
#endif
	return result;
}

// current system time in ms
uint32 getMSTime(void)
{
    uint32 time_in_ms = 0;
#if PLATFORM == PLATFORM_WIN32
    time_in_ms = timeGetTime();
#else
    struct timeb tp;
    ftime(&tp);

    time_in_ms = tp.time * 1000 + tp.millitm;
#endif

    return time_in_ms;
}

uint32 GetFileSize(const char* sFileName)
{
    std::ifstream f;
    f.open(sFileName, std::ios_base::binary | std::ios_base::in);
    if (!f.good() || f.eof() || !f.is_open()) { return 0; }
    f.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = f.tellg();
    f.seekg(0, std::ios_base::end);
    return f.tellg() - begin_pos;
}


