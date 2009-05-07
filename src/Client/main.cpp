#include <new>

#include "common.h"
#include "main.h"
#include "PseuWoW.h"
#include "MemoryDataHolder.h"


std::list<PseuInstanceRunnable*> instanceList; // TODO: move this to a "Master" class later


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

void _UnhookSignals(void)
{
    signal(SIGINT, 0);
    signal(SIGQUIT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
    #ifdef _WIN32
    signal(SIGBREAK, 0);
    #endif
}

void _OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            quitproc();            
            break;
        case SIGABRT:
        #ifndef _DEBUG
        case SIGSEGV:
        #endif
        #ifdef _WIN32
        case SIGBREAK:
        #endif
            abortproc();
            break;
    }
    signal(s, _OnSignal);
}

void quitproc(void)
{
    log("Waiting for all instances to finish... [%u]\n",instanceList.size());
    for(std::list<PseuInstanceRunnable*>::iterator i=instanceList.begin();i!=instanceList.end();i++)
    {
        (*i)->GetInstance()->Stop();
    }
}

void abortproc(void)
{
    log("Terminating all instances... [%u]\n",instanceList.size());
    for(std::list<PseuInstanceRunnable*>::iterator i=instanceList.begin();i!=instanceList.end();i++)
    {
        (*i)->GetInstance()->SetFastQuit(true);
        (*i)->GetInstance()->Stop();
    }
}

void _new_handler(void)
{
    logcritical("ERROR: Out of memory!");
    throw;
}

#if PLATFORM == PLATFORM_WIN32 && !defined(_CONSOLE)
int CALLBACK WinMain( IN HINSTANCE hInstance, IN HINSTANCE hPrevInstance, IN LPSTR lpCmdLine, IN int nShowCmd)
{
    main(0, NULL);
}
#endif

int main(int argc, char* argv[])
{
    try
    {
        std::set_new_handler(_new_handler);
        log_prepare("logfile.txt","a");
        logcustom(0,LGREEN,"+----------------------------------+");
        logcustom(0,LGREEN,"| (C) 2006-2009 Snowstorm Software |");
        logcustom(0,LGREEN,"|  http://www.mangosclient.org     |");
        logcustom(0,LGREEN,"+----------------------------------+");
        logcustom(0,GREEN,"Platform: %s",PLATFORM_NAME);
        logcustom(0,GREEN,"Compiler: %s ("COMPILER_VERSION_OUT")",COMPILER_NAME,COMPILER_VERSION);
        logcustom(0,GREEN,"Compiled: %s  %s",__DATE__,__TIME__);
        
        _HookSignals();
        MemoryDataHolder::Init();

        // 1 instance is enough for now
        PseuInstanceRunnable *r=new PseuInstanceRunnable();
        ZThread::Thread t(r);
        instanceList.push_back(r);
        t.setPriority((ZThread::Priority)2);
        //...
        t.wait();
        //...
        log_close();
        _UnhookSignals();
        raise(SIGABRT);  // this way to terminate is not nice but the only way to quit the CLI thread
        raise(SIGQUIT);
        return 0;
	} 
    catch (...)
    {
        printf("ERROR: Unhandled exception in main thread!\n"); 
        raise(SIGABRT);
        return 1;
    }
}
