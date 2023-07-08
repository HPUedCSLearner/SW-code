#include <crts.h>

#include <stdio.h>

#define BLOCK_SIZE 16
#define WA 4*BLOCK_SIZE
#define HA 4*BLOCK_SIZE
#define WB 5*BLOCK_SIZE

typedef struct {
    int width;
    int height;
    int stride;
    double* elements;
} Matrix;

typedef struct{
    int widthA;
    int heightA;
    int widthB;
    int heightB;
    double* eA; 
    double* eB; 
    double* eC; 
} Para;

#define WC WB/8
#define HC HA/8

__thread_local double C_s[HC][WC] __attribute__ ((aligned(64)));
__thread_local crts_rply_t dma_rply = 0;
__thread_local unsigned int D_COUNT = 0;

void MatrixMulSW(void* para){
    Para para_s;
    int rstartC, cstartC;
    int row, col;/* global index of C_s[i][j] */
    int wA, wB; 
    int i, j, e;
    rstartC = CRTS_rid * HC; 
    cstartC = CRTS_cid * WC; 

    if (CRTS_tid == 0 || CRTS_tid == 1) {
        printf("CRTS_rid=%d, CRTS_cid= %d\n", CRTS_rid, CRTS_cid);
        printf("CRTS_tid=%d, rstartC= %d\n", CRTS_tid, rstartC);
        printf("CRTS_tid=%d, cstartC= %d\n", CRTS_tid, cstartC);
        printf("%s:%d, %s(): ....\n", __FILE__, __LINE__,__func__);
    }

    if (CRTS_tid == 0) {
        printf("&row= %p\n", &row);
        printf("&dma_rply= %p\n", &dma_rply);
        printf("&D_COUNT= %p\n", &D_COUNT);
    }

    /* 1. get parameters from host */
    CRTS_dma_iget(&para_s, para, sizeof(Para), &dma_rply);
    D_COUNT++;
    CRTS_dma_wait_value(&dma_rply, D_COUNT);
    wA = para_s.widthA;
    wB = para_s.widthB;
    for(i=0; i<HC; i++)
        for(j=0; j<WC; j++)
            C_s[i][j] = 0.0;

    /* 2. coculate C */
    for(i=0; i<HC; i++){
        row = rstartC + i;
        for(j=0; j<WC; j++){
            col = cstartC + j;
            for(e=0; e<wA; e++){
                C_s[i][j] += para_s.eA[row * wA + e] * para_s.eB[e * wB + col];
            } 
        } 
    }

    /* 3. put C back to host */
    CRTS_dma_iput_stride(para_s.eC+rstartC*wB+cstartC, &C_s[0][0], HC*WC*sizeof(double), WC*sizeof(double), (wB-WC)*sizeof(double), &dma_rply);
    D_COUNT++;
    CRTS_dma_wait_value(&dma_rply, D_COUNT);
}