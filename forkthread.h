#include<unistd.h>
#include<sys/types.h>
typedef void (*FuncPoint);
pid_t fork_create(  void (*),void * arg);
