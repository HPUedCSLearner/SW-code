#include<athread.h>
#include <unistd.h>

extern void SLAVE_FUN(fun());

unsigned long getOneSecondClock();

int flag =0; 

void testHostClock()
{
    printf("in host: %ld\n", getOneSecondClock());
    printf("in host: %ld\n", getOneSecondClock());
    printf("in host: %ld\n", getOneSecondClock());
    printf("in host: %ld\n", getOneSecondClock());
    printf("in host: %ld\n", getOneSecondClock());
    printf("in host: %ld\n", getOneSecondClock());
}

int main(){
    testHostClock();

    athread_init();
    athread_spawn(fun,0);
    athread_join();
    

    
    athread_halt();
    return 0;
}


unsigned long getOneSecondClock()
{
    unsigned long start = athread_time_cycle();
    // sleep(1);
    unsigned long end = athread_time_cycle();
    return end - start;
}