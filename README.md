# SW-code

神威常用的代码
编码片段



## 常用接口汇总

```c
// Debug
printf("%s:%d, %s(): ....\n", __FILE__, __LINE__, __func__);
printf("[row %d, col %d, num %d]: \n", _ROW, _COL, _PEN);
if (_PEN==0） printf("\n",)

// 从核位置接口变量
CRTS_tid 从核号（硬件编号，0-63）
CRTS_rid 从核所在行号（硬件编号，0-7）
CRTS_cid 从核所在列号（硬件编号，0-7）
CRTS_cgn 从核所在核组号（硬件编号，0-5）
CRTS_spc_tid 从核簇方式的逻辑从核号（0-63），可用于 LDM 共享
CRTS_spcn 从核簇号（0-15）

_ROW
_COL
_PEN

// LDM
void *CRTS_pldm_malloc (size_t size);  
void *ldm_malloc (size_t size);

int CRTS_pldm_get_free_size (void);  
int get_allocatable_size (void); 

void CRTS_pldm_free (void *p, size_t size);  
void ldm_free (void *p, size_t size); 


// DMA
int CRTS_dma_get (void *dst, void *src, int len); 
int CRTS_dma_put (void *dst, void *src, int len); 
int CRTS_dma_get_stride (void *dst, void *src, int len, int bsize, int stride);
int CRTS_dma_put_stride (void *dst, void *src, int len, int bsize, int stride);  

int CRTS_dma_iget(void *dst, void *src, int len, crts_rply_t *rply); 
int CRTS_dma_iput(void *dst, void *src, int len, crts_rply_t *rply);
int CRTS_dma_iget_stride(void *dst, void *src, int len, int bsize, int stride, crts_rply_t *rply); 
int CRTS_dma_iput_stride(void *dst, void *src, int len, int bsize, int stride, crts_rply_t *rply);
int CRTS_dma_wait_value(crts_rply_t *rply, int value); 



// 计时
    // 主核 2.1GHz
unsigned long athread_time_cycle();
unsigned long CRTS_time_cycle (void); 

    // 从核 2.25GHz
unsigned long athread_stime_cycle();
unsigned long CRTS_stime_cycle (void); 


//  -faddress_align=64 
-faddress_align=64 

// RMA
int athread_rma_get (void *l_addr, int len, int r_tid, void *r_addr, athread_rply_t *r_rply);
l_addr：本地接收地址，必须为局存地址（4B 对界/对齐）； 
len：数据传输量，单位为字节（4B 的整数倍）； 
r_tid: 远程从核号； 
r_addr：远程源地址，必须为局存地址（4B 对界/对齐）； 
r_rply：远程回答字地址，必须局存地址，用于远程从核判断该 RMA 操作是否完成。 

int athread_rma_iget (void *l_addr, athread_rply_t *l_rply, int len,int r_tid, void *r_addr, athread_rply_t *r_rply);
l_addr：本地接收地址，必须为局存地址（4B 对界/对齐）； 
l_rply：本地回答字，用于发起 RMA 的从核判断 RMA 操作是否完成； 
len：数据传输量，单位为字节（4B 的整数倍）； 
r_tid：远程从核号； 
r_addr：远程源地址，必须为局存地址（4B 对界/对齐）； 
r_rply：远程回答字地址，必须局存地址，用于远程从核判断该 RMA 操作是否完成。 


// RMA读：从远程从核号的变量 读到 本地从核变量
// RMA读
int athread_rma_get (void *l_addr, int len, 
                     int r_tid, void *r_addr, athread_rply_t *r_rply);
// RMA非阻塞读
int athread_rma_iget(void *l_addr, athread_rply_t *l_rply, int len,
                     int r_tid, void *r_addr, athread_rply_t *r_rply);
                     
int athread_rma_wait_value (athread_rply_t *rply, int value);

// RMA写：将本地从核数据 写到 远程从核（加上限定）
// RMA写
int athread_rma_put (void *l_addr, int len,
                     int r_tid, void *r_addr, athread_rply_t *r_rply); 



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

// 浮点效率


// 判断是否在 主副对角线
    if (!(_COL ^ _ROW)) { // _COL == _ROW
        printf("row %d, col %d\n", _ROW, _COL);
        // printf("row ^ col: %d\n", _COL ^ _ROW);
    }

    // if ((_COL ^ _ROW) == 7) {
    if (!(_COL ^ _ROW ^ 7)) { // _COL ^ _ROW == 7
        printf("row %d, col %d\n", _ROW, _COL);
    }

```


### 从核位置技巧
```c

if (!(_COL ^ _ROW)) { // _COL == _ROW
    printf("row %d, col %d\n", _ROW, _COL);
    // printf("row ^ col: %d\n", _COL ^ _ROW);
}

// if ((_COL ^ _ROW) == 7) {
if (!(_COL ^ _ROW ^ 7)) { // _COL ^ _ROW == 7
    printf("row %d, col %d\n", _ROW, _COL);
}


[_ROW, _COL] 从核号 _PEN == _ROW * 8 + _COL
[_ROW, _COL] 转置 [_ROW, _COL]
[_ROW, _COL] 左边从核 [_ROW, (_COL - 1 + 8) % 8]
[_ROW, _COL] 右边从核 [_ROW, (_COL + 1 + 8) % 8]
[_ROW, _COL] 上边从核 [(_ROW - 1 + 8) % 8, _COL]
[_ROW, _COL] 下边从核 [(_ROW + 1 + 8) % 8, _COL]

```