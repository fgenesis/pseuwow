#ifndef _LOG_H
#define _LOG_H

class PseuInstance;

void log_prepare(char *fn, PseuInstance* p);
void log(const char *str, ...);
void logdetail(const char *str, ...);
void logdebug(const char *str, ...);
void log_close();

#endif
    