#include<slave.h>
#include<unistd.h>
#include<stdio.h>
#include<athread.h>

#define ldm_buf_size (256 * 32)
__thread_local double ldm_buf[ldm_buf_size];

#include "common.h"

void fun(Args* arg){
    unsigned long start, end;
    int myid = _PEN;
    int i;
    if (1) {
        for (i = 0; i < 1024; i+= 64) {
            start = slave_timer();
            athread_dma_get(ldm_buf, arg->data[myid], i * sizeof(double));
            // athread_dma_get(ldm_buf, arg->data[myid], ldm_buf_size * sizeof(double));
            end = slave_timer();
            if(myid ==0) printf("timer: %ld\n", end - start);
        }
    }
}


unsigned long slave_timer()
{
    return athread_stime_cycle();
}

