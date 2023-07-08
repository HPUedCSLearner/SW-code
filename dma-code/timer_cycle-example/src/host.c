#include<athread.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

extern void SLAVE_FUN(fun(Args* arg));


int main(){
    Args arg;
    memset(&arg, 0, sizeof(&arg));

    athread_init();
    athread_spawn(fun, &arg);
    athread_join();
    athread_halt();

    return 0;
}


unsigned long master_timer()
{
    return athread_time_cycle();
}

