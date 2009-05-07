#include <stdarg.h>
#include "common.h"
#include "log.h"

#if PLATFORM == PLATFORM_WIN32
#include <windows.h>
#endif

FILE *logfile = NULL;
uint8 loglevel = 0;
bool logtime = false;

void log_prepare(const char *fn, const char *mode = NULL)
{
    if(!mode)
        mode = "a";
    if(logfile)
    {
        fflush(logfile);
        fclose(logfile);
    }
    logfile = fopen(fn,mode);
}

void log_setloglevel(uint8 lvl)
{
    loglevel = lvl;
}

void log_setlogtime(bool b)
{
    logtime = b;
}

void log(const char *str, ...)
{
    if(!str)
        return;
    va_list ap;
    _log_setcolor(true,GREY);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    _log_resetcolor(true);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
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
    if(!str || loglevel < 1)
        return;
    va_list ap;
    _log_setcolor(true,LCYAN);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    _log_resetcolor(true);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
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
    if(!str || loglevel < 2)
        return;
    va_list ap;
    _log_setcolor(true,LBLUE);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    _log_resetcolor(true);


    printf("\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void logdev(const char *str, ...)
{
	if(!str || loglevel < 3)
		return;
	va_list ap;
	_log_setcolor(true,LMAGENTA);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
	va_start(ap, str);
	vprintf( str, ap );
	va_end(ap);
	_log_resetcolor(true);


	printf("\n");

	if(logfile)
	{
		fprintf(logfile, "%s", getDateString().c_str());
		va_start(ap, str);
		vfprintf(logfile, str, ap);
		fprintf(logfile, "\n" );
		va_end(ap);
		fflush(logfile);
	}
	fflush(stdout);
}

void logerror(const char *str, ...)
{
    va_list ap;
    _log_setcolor(false,LRED);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vfprintf( stderr,  str, ap );
    va_end(ap);
    _log_resetcolor(false);

    fprintf(stderr,"\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void logcritical(const char *str, ...)
{
    va_list ap;
    _log_setcolor(false,RED);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vfprintf( stderr, str, ap );
    va_end(ap);
    _log_resetcolor(false);

    fprintf(stderr,"\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void logcustom(uint8 lvl, Color color, const char *str, ...)
{
    if(!str || loglevel < lvl)
        return;
    va_list ap;
    _log_setcolor(true,color);
    if(logtime)
        printf("%s ", GetTimeString().c_str());
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
    _log_resetcolor(true);

    printf("\n");

    if(logfile)
    {
        fprintf(logfile, "%s", getDateString().c_str());
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void log_close()
{
	fclose(logfile);
}

void _log_setcolor(bool stdout_stream, Color color)
{
#if PLATFORM == PLATFORM_WIN32

    static WORD WinColorFG[Color_count] =
    {
        0,                                                  // BLACK
            FOREGROUND_RED,                                     // RED
            FOREGROUND_GREEN,                                   // GREEN
            FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
            FOREGROUND_BLUE,                                    // BLUE
            FOREGROUND_RED |                    FOREGROUND_BLUE,// MAGENTA
            FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,// WHITE
            // YELLOW
            FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
            // RED_BOLD
            FOREGROUND_RED |                                      FOREGROUND_INTENSITY,
            // GREEN_BOLD
            FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
            FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
            // MAGENTA_BOLD
            FOREGROUND_RED |                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            // CYAN_BOLD
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            // WHITE_BOLD
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
#else

    enum ANSITextAttr
    {
        TA_NORMAL=0,
        TA_BOLD=1,
        TA_BLINK=5,
        TA_REVERSE=7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK=30, FG_RED,  FG_GREEN, FG_BROWN, FG_BLUE,
        FG_MAGENTA,  FG_CYAN, FG_WHITE, FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK=40, BG_RED,  BG_GREEN, BG_BROWN, BG_BLUE,
        BG_MAGENTA,  BG_CYAN, BG_WHITE
    };

    static uint8 UnixColorFG[Color_count] =
    {
        FG_BLACK,                                           // BLACK
            FG_RED,                                             // RED
            FG_GREEN,                                           // GREEN
            FG_BROWN,                                           // BROWN
            FG_BLUE,                                            // BLUE
            FG_MAGENTA,                                         // MAGENTA
            FG_CYAN,                                            // CYAN
            FG_WHITE,                                           // WHITE
            FG_YELLOW,                                          // YELLOW
            FG_RED,                                             // LRED
            FG_GREEN,                                           // LGREEN
            FG_BLUE,                                            // LBLUE
            FG_MAGENTA,                                         // LMAGENTA
            FG_CYAN,                                            // LCYAN
            FG_WHITE                                            // LWHITE
    };

    fprintf((stdout_stream? stdout : stderr), "\x1b[%d%sm",UnixColorFG[color],(color>=YELLOW&&color<Color_count ?";1":""));
#endif
}

void _log_resetcolor(bool stdout_stream)
{
#if PLATFORM == PLATFORM_WIN32
    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
#else
    fprintf(( stdout_stream ? stdout : stderr ), "\x1b[0m");
#endif
}
