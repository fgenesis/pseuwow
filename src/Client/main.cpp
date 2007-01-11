#include "common.h"
#include "main.h"
#include "PseuWoW.h"

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
    printf("Waiting for all instances to finish... [%u]\n",instanceList.size());
    for(std::list<PseuInstanceRunnable*>::iterator i=instanceList.begin();i!=instanceList.end();i++)
    {
        (*i)->GetInstance()->Stop();
    }
}

void abortproc(void)
{
    printf("Terminating all instances... [%u]\n",instanceList.size());
    for(std::list<PseuInstanceRunnable*>::iterator i=instanceList.begin();i!=instanceList.end();i++)
    {
        (*i)->GetInstance()->SetFastQuit(true);
        (*i)->GetInstance()->Stop();
    }
}

int main(int argc, char* argv[]) {
    try 
    {
        
        _HookSignals();

        // 1 instance is enough for now
        PseuInstanceRunnable *r=new PseuInstanceRunnable();
        ZThread::Thread t(r);
        instanceList.push_back(r);
        t.setPriority((ZThread::Priority)2);
        //...
        t.wait();
        //...
        _UnhookSignals();
        raise(SIGQUIT);
	} 
    catch (...)
    {
        printf("ERROR: Unhandled exception in main thread!\n"); 
        raise(SIGABRT);
        return 1;
    }
}