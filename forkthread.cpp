#include "forkthread.h"
pid_t fork_create( void (*FuncPoint),void * arg )
{
    pid_t pid=fork();/*传递参数*/
    if(pid==0)/*如果(进程标记<0)*/
    {
        FuncPoint(arg);
    }
return pid;
}