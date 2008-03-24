#ifndef _MAIN_H
#define _MAIN_H

void _HookSignals(void);
void _UnhookSignals(void);
void _OnSignal(int);
void quitproc(void);
void abortproc(void);
void _new_handler(void);
int main(int,char**);

#endif
