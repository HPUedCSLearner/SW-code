#ifndef __COMMON_MASTER_H__
#define __COMMON_MASTER_H__

// common_master.h文件是存放主核函数用的通用API、spawn-join-API、lib-API等...

#include "common.h"
#include <math.h>

// master ge-api
void test_master_api();
void test_gemm(int m, int n, int k);

// master spawn-fun-join-api
void test_spawn_join();
void sw_pusu_gemm(int m, int n, int k, double *A, int lda, double *B, int ldb, double *C, int ldc);

// master lib-api
void crts_env_init();
void crts_env_halt();

unsigned long master_timer();
void dgemm_ref(int m, int n, int k, double *A, int lda, double *B, int ldb, double *C, int ldc);
void dgemm_self(int m, int n, int k, double *A, int lda, double *B, int ldb, double *C, int ldc);
void computeError(int ldc, int ldc_ref, int m, int n, double *C, double *C_ref);

void alloc_matrix_mem(double** matrix, int m, int n);
void init_matrix_with_value(double* matrix, int m, int n, int ld, double value);
void init_matrix_with_rand_value(double* matrix, int m, int n, int ld);
void free_matrix_mem(double* matrix);
void print_matrix(const double * matirx, int m, int n, int ld);


#endif