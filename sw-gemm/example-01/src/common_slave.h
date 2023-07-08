
#ifndef __COMMON_SLAVE_H__
#define __COMMON_SLAVE_H__
// common_slave.h文件是存放从核函数API、lib-API等...

#include "common.h"
#include <string.h>


// slave fun-api
void slvae_fun_example();
void slvae_pusu_gemm(SWARGS* args);


// slvate lib-api
unsigned long slave_timer(); 

#endif