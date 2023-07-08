#include <crts.h>

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

extern void MatMul(Matrix *A, Matrix *B, Matrix *C);

int main(int argc, char* argv[]){
    Matrix A, B, C;
    int size_A, size_B, size_C;
    int i, j;

    CRTS_init();

    A.width = WA;
    A.height = HA;
    B.width = WB;
    B.height = WA;
    C.width = B.width;
    C.height = A.height;

    /* Allocate A and B */
    size_A = A.width * A.height;
    A.elements = (double*)malloc(size_A*sizeof(double));
    size_B = B.width * B.height;
    B.elements = (double*)malloc(size_B*sizeof(double));
    /* Allocate C */
    size_C = C.width * C.height;
    C.elements = (double*)malloc(size_C*sizeof(double));

    /* set value of A and B */
    for(i=0; i<size_A; i++)
        A.elements[i] = 1.0f;
    for(i=0; i<size_B; i++)
        B.elements[i] = 0.01f;

    /* coculate C */
    MatMul(&A, &B, &C);

    /* output reslut */
    // for(i=0; i<C.height; i++){
    //     for(j=0; j<C.width; j++)
    //         printf("%f ", C.elements[i]);
    //     printf("\n");
    // }

    free(A.elements);
    free(B.elements);
    free(C.elements);

    CRTS_athread_halt();
    
    return 0;
}