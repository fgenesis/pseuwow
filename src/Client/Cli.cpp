#include "common.h"
#include "DefScript/DefScript.h"
#include "PseuWoW.h"
#include "Cli.h"

CliRunnable::CliRunnable(PseuInstance *p)
{
    _stop = false;
    _instance = p;
}

void CliRunnable::run(void)
{
    char buf[400],*in;
    std::string cur,out;

    while(!_stop)
    {
        printf("<%s>:",cur.c_str());
        fflush(stdout);
        in = fgets(buf,sizeof(buf),stdin);
        if (in == NULL)
            return;
        for(int i=0;in[i];i++)
            if(in[i]=='\r'||in[i]=='\n')
            {
                in[i]=0;
                break;
            }
        if(in[0]==0)
            continue;
        if(in[0]=='!')
            cur = &in[1];
        else
        {
            out = cur.empty() ? in : (cur+" "+in);
            _instance->AddCliCommand(out);
            // add delay just if necessary
            //ZThread::Thread::sleep(50);
        }
    }
}
