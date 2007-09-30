#include "common.h"
#include "Locale.h"
#include <fstream>
#include "ByteBuffer.h"

bool locale_set=false;
char my_locale[5];
char *cconf = "WTF/config.wtf";
char *cconfentry = "SET locale \"";

void SetLocale(char *loc)
{
    my_locale[4] = 0;
    if(loc && strlen(loc))
    {
	    memcpy(my_locale,loc,4);
    }
    else
    {
        uint32 fs = GetFileSize(cconf);
        std::fstream fh;
        std::string s;
        fh.open(cconf,std::ios_base::in);
        if(!fh.is_open())
        {
            printf("ERROR: unable to detect locale, could not open '%s'\n",cconf);
            return;
        }
        char *buf = new char[fs];
        fh.read((char*)buf,fs);
        fh.close();
        
        for(uint32 i=0; i<fs; i++)
        {
            if(buf[i]=='\n')
            {
                if(s.length() >= strlen(cconfentry) && !memcmp(s.c_str(),cconfentry,strlen(cconfentry)))
                {
                    memcpy(my_locale,s.c_str() + strlen(cconfentry), 4);
                    printf("Auto-detected locale '%s'\n",my_locale);
                    break;
                }
            s.clear();
            }
            else
            {
                s += buf[i];
            }
        }
        delete [] buf;
    }
    locale_set = true;
}

char *GetLocale(void)
{
    if(locale_set)
	    return &my_locale[0];
    return NULL;
}