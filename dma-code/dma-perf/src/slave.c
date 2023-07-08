#include<slave.h>
#include<unistd.h>
#include<stdio.h>

unsigned long getOneSecondClock()
{
    unsigned long start = athread_stime_cycle();
    sleep(1);
    unsigned long end = athread_stime_cycle();
    return end - start;
}

void testHostClock()
{
    printf("in slave: %ld\n", getOneSecondClock());
    printf("in slave: %ld\n", getOneSecondClock());
    printf("in slave: %ld\n", getOneSecondClock());
    printf("in slave: %ld\n", getOneSecondClock());
    printf("in slave: %ld\n", getOneSecondClock());
    printf("in slave: %ld\n", getOneSecondClock());
}

void fun(){
    int myid = _PEN;
    if (myid == 0) {
        testHostClock();
    }
}



