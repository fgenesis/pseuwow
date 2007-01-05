#include "../shared/common.h"
#include "main.h"
#include "PseuWoW.h"

#ifndef SIGQUIT
#define SIGQUIT 3
#endif

void _HookSignals(void)
{
	signal(SIGINT, _OnSignal);
    signal(SIGQUIT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
    #ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
    #endif
}

void _OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGABRT:
            quitproc();            
            break;
        #ifdef _WIN32
        case SIGBREAK:
            exit(0);
            break;
        #endif
    }
    signal(s, _OnSignal);
}

void quitproc(void){
    exit(0);
}


int main(int argc, char* argv[]) {
    try 
    {
        _HookSignals();
        ZThread::Thread t(new PseuInstanceRunnable);
        t.setPriority((ZThread::Priority)2);
        //...
        t.wait();
        //...
        return 0;
	} 
    catch (...)
    {
        printf("ERROR: Unhandled exception in main thread!\n"); 
        raise(SIGABRT);
        return 1;
    }
}