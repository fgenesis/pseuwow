#include <stdarg.h>
#include "common.h"
#include "PseuWoW.h"
#include "log.h"

PseuInstance *instance=NULL;
FILE *logfile=NULL;


void log_prepare(char *fn, PseuInstance* p)
{
    logfile = fopen(fn,"a");
    instance = p;
}

void log(const char *str, ...)
{
    if(!str)
        return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void logdetail(const char *str, ...)
{
    if(!str || instance->GetConf()->debug < 1)
        return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void logdebug(const char *str, ...)
{
    if(!str || instance->GetConf()->debug < 2)
        return;
    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}
    