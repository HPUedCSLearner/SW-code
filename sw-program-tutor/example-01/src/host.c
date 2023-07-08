#include <crts.h>

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

extern SLAVE_FUN(MatrixMulSW)(void*);

void MatMul(Matrix *A, Matrix *B, Matrix *C){
    Para para;
    para.widthA = A->width;
    para.heightA = A->height;
    para.widthB = B->width;
    para.heightB = B->height;
    para.eA = A->elements;
    para.eB = B->elements;
    para.eC = C->elements;

    athread_spawn(MatrixMulSW, &para);
    athread_join();
}