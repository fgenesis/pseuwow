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
#   include <direct.h>
#else
#   include <sys/dir.h>
#   include <sys/stat.h>
#   if defined(__FreeBSD__) || defined(__APPLE_CC__)
#       include <time.h>
#   endif
#   include <sys/timeb.h>
#   include <unistd.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
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
	std::transform(s.begin(), s.end(), s.begin(), tolower);
	return s;
}

std::string stringToUpper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), toupper);
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

std::string GetTimeString(void)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    char str[15];
    sprintf(str,"%02d:%02d:%02d", aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
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

std::string toHexDump(uint8* array, uint32 size, bool spaces, uint32 per_line)
{
    std::stringstream ss;
    char buf[5];
    for(uint32 i=0;i<size;i++)
    {
        if(array[i])
        {
            sprintf(buf,(array[i]<=0x0F)?"0%lX":"%lX",(uint32)array[i]);
            ss << buf;
        }
        else
            ss << "00"; // little hacklike fix

        if(per_line && !((i+1) % per_line))
        {
            ss << "\n";
            continue;
        }

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
        if( !(fil.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            files.push_back(std::string(fil.cFileName));
        while(FindNextFile(hFil,&fil))
        {
            if( !(fil.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                files.push_back(std::string(fil.cFileName));
        }
    }

# endif

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
	result = mkdir(dir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
    if(!sFileName || !*sFileName)
        return 0;
    std::ifstream f;
    f.open(sFileName, std::ios_base::binary | std::ios_base::in);
    if (!f.good() || f.eof() || !f.is_open()) { return 0; }
    f.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = f.tellg(), end_pos;
    f.seekg(0, std::ios_base::end);
    end_pos = f.tellg();
    f.close();
    return end_pos - begin_pos;
}

// fix filenames for linux ( '/' instead of windows '\')
void _FixFileName(std::string& str)
{
    for(uint32 i = 0; i < str.length(); i++)
        if(str[i]=='\\')
            str[i]='/';
}

// extracts the file name from a given path
std::string _PathToFileName(std::string str)
{
    size_t pathend = str.find_last_of("/\\");
    if(pathend != std::string::npos)
    {
        return str.substr(pathend+1);
    }
    return str;
}

std::string NormalizeFilename(std::string s)
{
    size_t p;
    while( (p = s.find('\\')) != std::string::npos)//Replace \ by /
    {
        s.replace(p,1,"/");
    }
    while( (p = s.find(' ')) != std::string::npos)//Replace space by _
    {
        s.replace(p,1,"_");
    }
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

std::string FilesizeFormat(uint32 b)
{
    char buf[15];
    if (b < 1024)
    {
        sprintf(buf,"%lu B",b);
    }
    else if(b < 1024*1024)
    {
        sprintf(buf,"%.2f kB",(b / 1024.0f));
    }
    else if(b < 1024*1024*1024)
    {
        sprintf(buf,"%.2f MB",(b / double(1024*1024)));
    }
    else
    {
        sprintf(buf,"%.2f GB",(b / double(1024*1024*1024)));
    }
    return buf;
}

//! Returns the string of the current working directory
std::string GetWorkingDir(void)
{
    char d[MAX_PATH];
#if PLATFORM == PLATFORM_WIN32
    _getcwd(d, MAX_PATH);
#else
    getcwd(d, MAX_PATH);
#endif
    return d;
}


//! Changes the current Working Directory to the given string.
bool SetWorkingDir(const char *dir)
{
    bool success=false;
#ifdef _MSC_VER
    success=(_chdir(dir) == 0);
#else
    success=(chdir(dir) == 0);
#endif
    return success;
}


std::string GetAbsolutePath(const char *filename)
{
    char *p = NULL;

#if PLATFORM == PLATFORM_WIN32
    char fpath[MAX_PATH];
    p = _fullpath( fpath, filename, MAX_PATH);

#else
    char fpath[4096];
    fpath[0]=0;
    p = realpath(filename, fpath);
    if (!p)
    {
        // content in fpath is undefined at this point
        if ('0'==fpath[0]) // seems like fpath wasn't altered
        {
            // at least remove a ./ prefix
            if ('.'==filename[0] && '/'==filename[1])
                return &filename[2]; // skip first 2 chars
            else
                return filename;
        }
        else
            return fpath;
    }

#endif

    return p;
}
