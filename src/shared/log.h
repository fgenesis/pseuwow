#ifndef _LOG_H
#define _LOG_H

enum Color
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE
};

void log_prepare(const char *fn, const char *mode);
void log_setloglevel(uint8 lvl);
void log_setlogtime(bool b);
void log(const char *str, ...);
void logdetail(const char *str, ...);
void logdebug(const char *str, ...);
void logdev(const char *str, ...);
void logerror(const char *str, ...);
void logcritical(const char *str, ...);
void logcustom(uint8 loglevel, Color color, const char *str, ...);
void log_close();
void _log_setcolor(bool,Color);
void _log_resetcolor(bool);


const int Color_count = int(WHITE)+1;

#endif
