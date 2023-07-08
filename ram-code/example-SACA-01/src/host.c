#include<athread.h>

extern void SLAVE_FUN(fun());

int flag = 0; 

int main(){
    athread_init();
    athread_spawn(fun,0);
    athread_join();
    if(flag==0)
        printf("result ok!\n");
    else
        printf("result fail\n");
    
    athread_halt();
    return 0;
}