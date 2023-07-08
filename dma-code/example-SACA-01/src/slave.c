#include<slave.h>

int a[10] ={10,9,8,7,6,5,4,3,2,1};
__thread_local int b[10];
int c[10];
volatile __thread_local int reply;
extern int flag;

void fun(){
    athread_dma_get(b,a,sizeof(int));
    if(b[0]!=10){
        flag+=1;
        printf("athread_dma_get fail!\n");
    } 
    athread_dma_put(c,b,sizeof(int));
    if(c[0]!=10){
        flag+=1;
        printf("athread_dma_put fail!\n");
    }

    athread_dma_get_stride(b,a,sizeof(int)*5,sizeof(int),sizeof(int));
    if(b[0]!=10||b[1]!=8||b[2]!=6||b[3]!=4||b[4]!=2){
        flag+=1;
        printf("athread_dma_get_stride fail!\n");
    }

    athread_dma_put_stride(c,b,sizeof(int)*5,sizeof(int),sizeof(int));
    if(c[0]!=10||c[2]!=8||c[4]!=6||c[6]!=4||c[8]!=2){
        flag+=1;
        printf("athread_dma_put_stride fail!\n");
    }

//  ====================================================================================
    memset(b,0,sizeof(int)*10);
    reply=0;
    athread_get(PE_MODE,a,b,sizeof(int),&reply,0,0,0);
    while(reply!=1); 
        if(b[0]!=10){
        flag+=1;
        printf("athread_get fail!\n");
    } 

    reply=0;
    athread_get(PE_MODE,a,b,sizeof(int)*5,&reply,0,sizeof(int),sizeof(int)); 
    while(reply!=1); 
        if(b[0]!=10||b[1]!=8||b[2]!=6||b[3]!=4||b[4]!=2){
        flag+=1;
        printf("athread_get1 fail!\n");
    } 

    reply=0;
    athread_put(PE_MODE,b,c,sizeof(int),&reply,0,0);
    while(reply!=1); 
        if(c[0]!=10){
        flag+=1;
        printf("athread_put fail!\n");
    }

    reply=0;
    athread_put(PE_MODE,b,c,sizeof(int)*5,&reply,sizeof(int),sizeof(int));
    while(reply!=1);
        if(c[0]!=10||c[2]!=8||c[4]!=6||c[6]!=4||c[8]!=2){
        flag+=1;
        printf("athread_put1 fail!\n");
    }

//  ====================================================================================

    // DMA 栏栅。
    // 用于本核心发出的 DMA 的保序操作。
    // 组内栏栅保证此命令前本从核发出的所有 DMA 命令执行完毕，才执行后续的 DMA 操作。
    athread_dma_barrier(); 

//  ====================================================================================

    memset(b,0,sizeof(int)*10);
    reply=0;
    athread_dma_iget(b,a,sizeof(int),&reply);
    athread_dma_wait_value(&reply,1);
    if(b[0]!=10){
        flag+=1;
        printf("athread_dma_iget fail!\n");
    }


//  ====================================================================================
    // DMA 全栏栅。
    // 兼具 DMA 栏栅和 RDMA 栏栅的语义，从而保证同一个从核发出的先后 DMA/RMA 操作的序。
    athread_dma_all_barrier();
//  ====================================================================================


    reply=0;
    athread_dma_iget_stride(b,a,sizeof(int)*5,sizeof(int),sizeof(int),&reply);
    athread_dma_wait_value(&reply,1);
    if(b[0]!=10||b[1]!=8||b[2]!=6||b[3]!=4||b[4]!=2){
        flag+=1;
        printf("athread_dma_iget_stride fail!\n");
    }

    reply=0;
    athread_dma_iput(c,b,sizeof(int),&reply);
    athread_dma_wait_value(&reply,1);
    if(c[0]!=10){
        flag+=1;
        printf("athread_dma_iput fail!\n");
    }

    reply=0;
    athread_dma_iput_stride(c,b,sizeof(int)*5,sizeof(int),sizeof(int),&reply);
    athread_dma_wait_value(&reply,1);
    if(c[0]!=10||c[2]!=8||c[4]!=6||c[6]!=4||c[8]!=2){
        flag+=1;
        printf("athread_dma_iput_stride fail!\n");
    }
}



