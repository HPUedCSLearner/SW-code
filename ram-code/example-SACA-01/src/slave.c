#include<slave.h>

extern int flag;
__thread_local int a;
__thread_local int reply;

int fun(){
    //阻塞 RMA 读
    reply=0;
    if(_PEN==0)
        a=1;
    athread_ssync_array();
    athread_rma_get(&a,4,0,&a,&reply); // 可以想象，每个从核都从0号从核LDM空间读取数据
    if(a!=1)
        flag+=1;
    
    //非阻塞 RMA 读
    athread_ssync_array();
    reply=0;
    if(_PEN==0)
        a=2;
    athread_ssync_array();
    athread_rma_iget(&a,&reply,4,0,&a,&reply);
    athread_rma_wait_value(&reply,1);
    if(a!=2)
        flag+=1;

    //阻塞 RMA 写
    // 0号从核 和 1号从核通信示例
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=3;
        athread_rma_put(&a,4,1,&a,&reply);
    }else if(_PEN==1){
        athread_rma_wait_value(&reply,1);
        if(a!=3)
        flag+=1;
    }

    //非阻塞 RMA 写
    athread_ssync_array();
    reply=0;
    if(_PEN==0){
        a=4;
        athread_rma_iput(&a,&reply,4,1,&a,&reply);
    }else if(_PEN==1){
        athread_rma_wait_value(&reply,1);
        if(a!=4)
        flag+=1;
    }

    //阻塞 RMA 广播
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=5;
        athread_rma_bcast(&a,&a,4,&reply);
    }
    athread_ssync_array();
    if(a!=5)
        flag+=1;
    
    //非阻塞 RMA 广播
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=6;
        athread_rma_ibcast(&a,&a,&reply,4,&reply);
        athread_rma_wait_value(&reply,1);
    }
    athread_ssync_array();
    if(a!=6)
        flag+=1;

    
    //集合方式的 RMA 广播
    athread_ssync_array();
    reply=0;
    if(_PEN==0){
        a=7;
    }
    athread_ssync_array();
    athread_rma_bcast_coll(&a,&a,4,0);
    if(a!=7)
        flag+=1;
    
    //RMA 行广播
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=8;
        athread_rma_row_bcast(&a,&a,4,&reply);
    }
    athread_ssync_array();
    if(_ROW==0&&a!=8)
        flag+=1;
    
    //非阻塞 RMA 行广播
    athread_ssync_array(); reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=9;
        athread_rma_row_ibcast(&a,&a,4,&reply,&reply);
        athread_rma_wait_value(&reply,1);
    }
    athread_ssync_array();
    if(_ROW==0&&a!=9)
        flag+=1;
    
    //集合方式的 RMA 行广播
    athread_ssync_array();
    reply=0;
    a=_PEN;
    athread_ssync_array();
    athread_rma_row_bcast_coll(&a,&a,4,0);
    athread_ssync_array();
    if(a!=(_PEN/8)*8)
        flag+=1;
    
    //RMA 行多播
    athread_ssync_array();
    reply=0;
    if(_PEN==0)
        a=11;
    athread_ssync_array();
    if(_PEN==0)
        athread_rma_row_mcast(&a,&a,4,0xff,&reply);
    athread_ssync_array();
    if(a!=11)
        flag+=1;

    //非阻塞 RMA 行多播
    athread_ssync_array();
    reply=0;
    if(_PEN==0)
        a=12;
    athread_ssync_array();
    if(_PEN==0)
        athread_rma_row_imcast(&a,&a,4,&reply,0xff,&reply);
    athread_rma_wait_value(&reply,1);
    athread_ssync_array();
    if(a!=12)
        flag+=1;
    
    //RMA 列广播
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=13;
        athread_rma_col_bcast(&a,&a,4,&reply);
    }
    athread_ssync_array();
    if(_COL==0&&a!=13)
        flag+=1;

    
    //非阻塞 RMA 列广播
    athread_ssync_array();
    reply=0;
    athread_ssync_array();
    if(_PEN==0){
        a=14;
        athread_rma_col_ibcast(&a,&a,4,&reply,&reply);
        athread_rma_wait_value(&reply,1);
    }
    athread_ssync_array();
    if(_COL==0&&a!=14)
        flag+=1;
    
    //集合方式的 RMA 列广播
    athread_ssync_array();
    reply=0;
    a=_PEN;
    athread_ssync_array();
    athread_rma_col_bcast_coll(&a,&a,4,0);
    athread_ssync_array();
    if(a!=(_PEN%8))
        flag+=1;

    //RMA 列多播
    athread_ssync_array();
    reply=0;
    if(_PEN==0)
        a=16;
    athread_ssync_array();
    if(_PEN==0)
        athread_rma_col_mcast(&a,&a,4,0xff,&reply);
    athread_ssync_array();
    if(a!=16)
        flag+=1;
    
    //非阻塞 RMA 列多播
    athread_ssync_array();
    reply=0;
    if(_PEN==0)
        a=17;
    athread_ssync_array();
    if(_PEN==0)
        athread_rma_col_imcast(&a,&a,4,&reply,0xff,&reply);
    athread_rma_wait_value(&reply,1);
    athread_ssync_array();
    if(a!=17)
        flag+=1;
}