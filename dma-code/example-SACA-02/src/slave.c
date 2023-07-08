#include<slave.h>

int a[10] ={10,9,8,7,6,5,4,3,2,1};
__thread_local int b[10];
int c[10];
volatile __thread_local int reply;
__thread_local_share int d;
int e;
extern int flag;

// 集合广播：   需要所有从核调用
// 集合行广播： 需要从核阵列同一行上所有从核都调用
// 集合列广播： 需要从核阵列同一列上所有从核都调用

void fun(){
    // 阻塞DMA集合广播
    athread_dma_bcast_coll(b,a,sizeof(int));
    if(b[0]!=10){
        flag+=1;
        printf("athread_dma_bcast_coll error!\n");
    } 

    // 带（主存）跨步的阻塞DMA集合广播
    athread_dma_bcast_stride_coll(b,a,sizeof(int)*5,sizeof(int),sizeof(int));
    if(b[0]!=10||b[1]!=8||b[2]!=6||b[3]!=4||b[4]!=2){
        flag+=1;
        printf("athread_dma_bcast_stride_coll error!\n");
    } 

    // 非阻塞DMA行广播
    if(_PEN==0){
        reply=0;
        athread_dma_row_ibcast(b, &a[1], sizeof(int), &reply);
        athread_dma_wait_value(&reply,1);
    } 
    athread_ssync_array();
    if(_ROW==0&&b[0]!=9){
        flag+=1;
        printf("athread_dma_row_ibcast error!\n");
    } 

    // 带（主存）跨步的非阻塞DAM行广播
    if(_PEN==0){
        reply=0;
        athread_dma_row_ibcast_stride(b,a,sizeof(int)*2,sizeof(int),sizeof(int),&reply);
        athread_dma_wait_value(&reply,1);
    } 
    athread_ssync_array();
    if(_ROW==0&&(b[0]!=10||b[1]!=8)){
        flag+=1;
        printf("athread_dma_row_ibcast_stride error!\n");
    } 

    b[0]=0;
    // 保证所有核组的b[0]都已被赋值
    athread_ssync_array();

    // 阻塞DMA集合行广播
    athread_dma_row_bcast_coll(b,a,sizeof(int));
    athread_ssync_array();
    if(b[0]!=10){
        flag+=1;
        printf("athread_dma_row_bcast_coll error!\n");
    }

    // 带（主存）跨步的阻塞DMA集合行广播
    athread_dma_row_bcast_stride_coll(b,a,sizeof(int)*2,sizeof(int),sizeof(int));
    athread_ssync_array();
    if((b[0]!=10||b[1]!=8)){
        flag+=1;
        printf("athread_dma_row_bcast_stride_coll error!\n");
    }

    b[0]=0;
    athread_ssync_array(); // 从核同步

    // 阻塞DMA集合列广播
    athread_dma_col_bcast_coll(b,a,sizeof(int));
    athread_ssync_array();
    if(b[0]!=10){
        flag+=1;
        printf("athread_dma_col_bcast_coll error!\n");
    }

    // athread_ssync_array();
    // 带（主存）跨步的阻塞DMA集合列广播
    athread_dma_col_bcast_stride_coll(b,a,sizeof(int)*2,sizeof(int),sizeof(int));
    athread_ssync_array();
    if((b[0]!=10||b[1]!=8)){
        flag+=1;
        printf("athread_dma_col_bcast_stride_coll error!\n");
    }

    // 非阻塞DMA列广播
    if(_PEN==0){
        reply=0;
        athread_dma_col_ibcast (b, &a[1], sizeof(int), &reply);
        athread_dma_wait_value(&reply,1);
    }
    athread_ssync_array(); // 强制从核同步
    if(_COL==0&&b[0]!=9){
        flag+=1;
        printf("athread_dma_col_ibcast error!\n");
    }

    // 带（主存）跨步的非阻塞DMA列广播
    if(_PEN==0){
        reply=0;
        athread_dma_col_ibcast_stride(b,a,sizeof(int)*2,sizeof(int),sizeof(int),&reply);
        athread_dma_wait_value(&reply,1);
    }
    // 从核接口。共享 ldm 的从核同步
    athread_ssync_sldm(); 
    if(_COL==0&&(b[0]!=10||b[1]!=8)){
        flag+=1;
        printf("athread_dma_col_ibcast_stride error!\n");
    }

    // athread_memcpy_sldm(&d,&a,sizeof(int),MEM_TO_LDM);
    // athread_memcpy_sldm(&e,&d,sizeof(int),LDM_TO_MEM);
    // if(d!=10||e!=10){
    //     flag+=1;
    //     printf("athread_memcpy_sldm error!\n");
    // }

    if (!(_COL ^ _ROW)) {
        printf("row %d, col %d\n", _ROW, _COL);
        // printf("row ^ col: %d\n", _COL ^ _ROW);
    }

    // if ((_COL ^ _ROW) == 7) {
    if (!(_COL ^ _ROW ^ 7)) {
        printf("row %d, col %d\n", _ROW, _COL);
    }
}



