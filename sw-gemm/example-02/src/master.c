#include "common_master.h"
// #include "common_slave.h"

const unsigned long MASTER_CLOCK_CYCLE = 2.1  * 1e9;

// 从核接口声明
extern void SLAVE_FUN(slvae_fun_example());
extern void SLAVE_FUN(slvae_pusu_gemm(SWARGS* args));


// master ge-api imp
void test_master_api()
{
    printf("hello master api\n");
}

void test_gemm(int m, int n, int k)
{
    double *A, *B, *C, *C_ref;
    int lda, ldb, ldc, ldc_ref;
    double gflops, gflops_self, gflops_ref;
    unsigned long time_begin, dgemm_self_time, dgemm_ref_time;

    lda = k;
    ldb = n;
    ldc = n;
    ldc_ref = n;

    gflops = ( m * n / ( 1000.0 * 1000.0 * 1000.0 ) ) * ( 2 * k );

    alloc_matrix_mem(&A, m, k);
    alloc_matrix_mem(&B, k, n);
    alloc_matrix_mem(&C, m, n);
    alloc_matrix_mem(&C_ref, m, n);

    init_matrix_with_rand_value(A, m, k, lda);
    init_matrix_with_rand_value(B, k, n, ldb);
    init_matrix_with_value(C, m, n, ldc, 0);
    init_matrix_with_value(C_ref, m, n, ldc_ref, 0);

    // print_matrix(A, m, k, lda);
    // print_matrix(B, k, n, ldb);
    // print_matrix(C, m, n, ldc);
    // print_matrix(C_ref, m, n, ldc_ref);
    // print_matrix(&A[2*lda + 2], 2, 2, lda); // print sub-matrix

    time_begin = master_timer();
    // dgemm_self(m, n, k, A, lda, B, ldb, C, ldc);
    sw_pusu_gemm(m, n, k, A, lda, B, ldb, C, ldc);
    dgemm_self_time = master_timer() - time_begin;
    gflops_self = gflops / ((double)dgemm_self_time / MASTER_CLOCK_CYCLE);

    time_begin = master_timer();
    dgemm_ref(m, n, k, A, lda, B, ldb, C_ref, ldc_ref);
    // print_matrix(C_ref, m, n, ldc_ref);
    dgemm_ref_time = master_timer() - time_begin;
    gflops_ref = gflops / ((double)dgemm_ref_time / MASTER_CLOCK_CYCLE);

    computeError(ldc, ldc_ref, m, n, C, C_ref );

    // printf( "M\t K\t N\t SLEF_FLOPS\t REF_FLOPS\t SPEED_UP\n");
    printf( "%5d\t %5d\t %5d\t %5.10lf\t %5.10lf\t %5.10lf\n", 
            m, n, k, gflops_self, gflops_ref, gflops_self / gflops_ref);
    // printf( "%5d\t %5d\t %5d\t %5.10lf\t\n", m, n, k, gflops_self);

    free_matrix_mem(A);
    free_matrix_mem(B);
    free_matrix_mem(C);
    free_matrix_mem(C_ref);
}


// master spawn-fun-join-api imp
void test_spawn_join()
{
    CRTS_athread_spawn(slvae_fun_example, 0);
    CRTS_athread_join();
}
void sw_pusu_gemm(int m, int n, int k, double *A, int lda, double *B, int ldb, double *C, int ldc)
{
    SWARGS args;
    args.m = m;
    args.n = n;
    args.k = k;
    args.A = A;
    args.B = B;
    args.C = C;
    args.lda = lda;
    args.ldb = ldb;
    args.ldc = ldc;
    CRTS_athread_spawn(slvae_pusu_gemm, &args);
    CRTS_athread_join();
}


// master lib-api imp
void crts_env_init()
{
    CRTS_init();
}
void crts_env_halt()
{
    CRTS_athread_halt();
}

unsigned long master_timer()
{
    return CRTS_time_cycle();
}

void dgemm_ref(
        int    m,
        int    n,
        int    k,
        double *A,
        int    lda,
        double *B,
        int    ldb,
        double *C,
        int    ldc
        )
{
    // Local variables.
    int    i, j, p;

    // Sanity check for early return.
    if ( m == 0 || n == 0 || k == 0 ) return;

    // Reference GEMM implementation.

    // printf("m=%d, n=%d, k=%d, lda=%d, ldb=%d, ldc=%d\n", m, n, k, lda, ldb, ldc);
    
    for ( i = 0; i < m; i ++ ) {
        for ( j = 0; j < n; j ++ ) {
            for ( p = 0; p < k; p ++ ) {
                C[ i * ldc + j ] += A[ i * lda + p ] * B[ p * ldb + j ];
            }
        }
    }
}

void dgemm_self(
        int    m,
        int    n,
        int    k,
        double *A,
        int    lda,
        double *B,
        int    ldb,
        double *C,
        int    ldc
        )
{
    // Local variables.
    int    i, j, p;

    // Sanity check for early return.
    if ( m == 0 || n == 0 || k == 0 ) return;

    // Reference GEMM implementation.

    // printf("m=%d, n=%d, k=%d, lda=%d, ldb=%d, ldc=%d\n", m, n, k, lda, ldb, ldc);
    
    double tmpC_rusult[16];
    double tmpA[4];
    double tmpB[4];

    // 分成4*4的块计算C
    for ( i = 0; i < m; i+= 4 ) {
        for ( j = 0; j < n; j += 4 ) {

            memset(tmpC_rusult, 0, sizeof(tmpC_rusult));

            for ( p = 0; p < k; p++ ) {
                // 目前的性能瓶颈是在儿，下面的取数操作，大矩阵都是cache-miss
                tmpA[0] = A[ (i + 0) * lda + p ];  // 这边访存了
                tmpA[1] = A[ (i + 1) * lda + p ];  // 这边也访存了，没有利用上一步访存开销，局部性没有发挥
                tmpA[2] = A[ (i + 2) * lda + p ];  // 这里也是，上一步的局部性浪费了
                tmpA[3] = A[ (i + 3) * lda + p ];

                tmpB[0] = B[ p * ldb + j + 0]; // 取B的数据，是由局部性的
                tmpB[1] = B[ p * ldb + j + 1];
                tmpB[2] = B[ p * ldb + j + 2];
                tmpB[3] = B[ p * ldb + j + 3];
                
                tmpC_rusult[0]  += tmpA[0] * tmpB[0];
                tmpC_rusult[1]  += tmpA[0] * tmpB[1];
                tmpC_rusult[2]  += tmpA[0] * tmpB[2];
                tmpC_rusult[3]  += tmpA[0] * tmpB[3];

                tmpC_rusult[4]  += tmpA[1] * tmpB[0];
                tmpC_rusult[5]  += tmpA[1] * tmpB[1];
                tmpC_rusult[6]  += tmpA[1] * tmpB[2];
                tmpC_rusult[7]  += tmpA[1] * tmpB[3];

                tmpC_rusult[8]  += tmpA[2] * tmpB[0];
                tmpC_rusult[9]  += tmpA[2] * tmpB[1];
                tmpC_rusult[10] += tmpA[2] * tmpB[2];
                tmpC_rusult[11] += tmpA[2] * tmpB[3];

                tmpC_rusult[12] += tmpA[3] * tmpB[0];
                tmpC_rusult[13] += tmpA[3] * tmpB[1];
                tmpC_rusult[14] += tmpA[3] * tmpB[2];
                tmpC_rusult[15] += tmpA[3] * tmpB[3];
            }

            C[ (i + 0) * ldc + j + 0] += tmpC_rusult[0];
            C[ (i + 0) * ldc + j + 1] += tmpC_rusult[1];
            C[ (i + 0) * ldc + j + 2] += tmpC_rusult[2];
            C[ (i + 0) * ldc + j + 3] += tmpC_rusult[3];

            C[ (i + 1) * ldc + j + 0] += tmpC_rusult[4];
            C[ (i + 1) * ldc + j + 1] += tmpC_rusult[5];
            C[ (i + 1) * ldc + j + 2] += tmpC_rusult[6];
            C[ (i + 1) * ldc + j + 3] += tmpC_rusult[7];

            C[ (i + 2) * ldc + j + 0] += tmpC_rusult[8];
            C[ (i + 2) * ldc + j + 1] += tmpC_rusult[9];
            C[ (i + 2) * ldc + j + 2] += tmpC_rusult[10];
            C[ (i + 2) * ldc + j + 3] += tmpC_rusult[11];

            C[ (i + 3) * ldc + j + 0] += tmpC_rusult[12];
            C[ (i + 3) * ldc + j + 1] += tmpC_rusult[13];
            C[ (i + 3) * ldc + j + 2] += tmpC_rusult[14];
            C[ (i + 3) * ldc + j + 3] += tmpC_rusult[15];
        }
    }
}


#define TOLERANCE 1E-10
void computeError(
        int    ldc,
        int    ldc_ref,
        int    m,
        int    n,
        double *C,
        double *C_ref
        )

{
    int    i, j;
    for ( i = 0; i < m; i ++ ) {
        for ( j = 0; j < n; j ++ ) {
            if ( fabs( C[i*ldc + j] - C_ref[i*ldc + j] ) > TOLERANCE ) {
                // printf( "C[ %d ][ %d ] != C_ref, %E, %E\n", i, j, C[i*ldc + j] , C_ref[i*ldc + j]);
                printf( "C[ %d ][ %d ] != C_ref, %.20lf, %.20lf\n", i, j, C[i*ldc + j] , C_ref[i*ldc + j]);
                break;
            }
        }
        break;
    }
}


void alloc_matrix_mem(double** matrix, int m, int n)
{
    // printf("come to alloc_matrix_mem 1\n");
    *matrix = (double*)malloc( sizeof(double) * m * n );
    // printf("come to alloc_matrix_mem 2\n");
}

void init_matrix_with_value(double* matrix, int m, int n, int ld, double value)
{
    for ( int i = 0; i < m; i++ ) {
        for ( int j = 0; j < n; j++ ) {
            matrix[i*ld +j] = value;	
        }
    }
}

void init_matrix_with_rand_value(double* matrix, int m, int n, int ld)
{
    // printf("come to init_matrix_with_rand_value 1\n");
    for ( int i = 0; i < m; i++ ) {
        for ( int j = 0; j < n; j++ ) {
            // printf("come here i= %d, j = %d\n", i, j);
            matrix[i*ld +j] = (double)(drand48());	
        }
    }
    // printf("come to init_matrix_with_rand_value 2\n");
}

void free_matrix_mem(double* matrix)
{
    if(matrix != NULL) {
        free(matrix);
    }
}

void print_matrix(const double * matirx, int m, int n, int ld)
{
    printf("***************************************************\n");
    int i ,j;
    for(i = 0; i < m; ++i) {
        for (j = 0; j < n; ++j) {
            printf("%4.lf", matirx[i*ld + j]);
        }
        printf("\n");
    }
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}