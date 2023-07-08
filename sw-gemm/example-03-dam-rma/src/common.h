#ifndef __COMMON_H__
#define __COMMON_H__

// common.h文件是存放主核函数和从核函数公用的头文件、数据结构、等...

#include <crts.h>
#include <stdio.h>

// const unsigned long  SLAVE_CLOCK_CYCLE = 2.25 * 1e9;
// const unsigned long MASTER_CLOCK_CYCLE = 2.1  * 1e9;

typedef struct SWARGS
{
    int m, n, k;
    int lda, ldb, ldc;
    double *A, *B, *C;
}SWARGS;



#endif