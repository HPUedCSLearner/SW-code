#include "common_slave.h"

// 常用变量
const unsigned long  SLAVE_CLOCK_CYCLE = 2.25 * 1e9;
unsigned long t1, t2;

volatile __thread_local int reply;

__thread_local crts_rply_t dma_rply = 0;
__thread_local unsigned int D_COUNT = 0;
__thread_local unsigned int D_COUNTTTTTTT = 0;

// 常用宏变量
// CRTS_tid;
// CRTS_cid;
// CRTS_rid;

// Slave Lib-Api
unsigned long slave_timer()
{
    return CRTS_stime_cycle();
}

// Slave Fun-Api Imp
void slvae_fun_example(){
    int myid = CRTS_tid; int cid = CRTS_cid; int rid = CRTS_rid;
    int cgn  = CRTS_smng_get_cgn();
    int spcn = CRTS_smng_get_spcn();
    t1 = slave_timer();
    if (myid == 63) {
        printf("I am in [%d, %d], cgn: %d, spcn: %d\n", rid, cid, cgn, spcn);
        printf("%s:%d, %s(): ....\n", __FILE__, __LINE__, __func__);
    }
    t2 = slave_timer();
    if (myid == 63) {
        printf("t2 - t1 : %lld\n", t2 -t1);
    }
}

// __thread_local SWARGS args_buf;
// __thread_local double buf_a[para.k * 2], buf_b[para.k * 2];

void slvae_pusu_gemm(SWARGS* args)
{
    // printf("sizeof(SWARGS) %d\n",sizeof(SWARGS));
    // SWARGS* args_buf = (SWARGS*)CRTS_pldm_malloc(8 * sizeof(SWARGS));
    // CRTS_dma_get(&args_buf, args, sizeof(SWARGS));
    // CRTS_dma_iget(&args_buf, args, sizeof(SWARGS), &dma_rply);
    // D_COUNT++;
    // CRTS_dma_wait_value(dma_rply, D_COUNT);
    // if (CRTS_tid == 0) {
    //     // printf("sizeof(SWARGS) %d\n",sizeof(SWARGS));
    //     // SWARGS* args_buf = (SWARGS*)CRTS_pldm_malloc(sizeof(SWARGS));
    //     // CRTS_dma_get(&args_buf, args, sizeof(SWARGS));

    //     // SWARGS para;
    //     // CRTS_dma_iget(&para, args, sizeof(SWARGS), &dma_rply);
    //     // D_COUNT++;
    //     // CRTS_dma_wait_value(&dma_rply, D_COUNT);

    //     SWARGS para;
    //     CRTS_dma_get(&para, args, sizeof(SWARGS));

    //     // printf("args:%d %d %d\n", args->m, args->n, args->k);
    //     printf("args:%d %d %d\n", para.m, para.n, para.k);
    //     // printf("args:%d %d %d\n", args_buf->m, args_buf->n, args_buf->k);

        
    // }

    SWARGS para;
    CRTS_dma_get(&para, args, sizeof(SWARGS));
    // printf("args:%d %d %d\n", para.m, para.n, para.k);

    // 每个从核负责计算C的（row_start，col_start） 块大小为block_m * block_n的块
    int block_m = para.m / 8;
    int block_n = para.n / 8;

    int row_start = CRTS_rid * block_m;
    int col_start = CRTS_cid * block_n;
    // printf("block_m:%d block_n:%d row_start:%d col_start:%d\n", block_m, block_n, row_start, col_start);

    // 最朴素的三重循环
    // for (int i = 0; i < block_m; ++i) {
    //     int row = row_start + i;
    //     for (int j = 0; j < block_m; ++j) {
    //         int col = col_start + j;
    //         for (int k = 0; k < para.k; ++k) {
    //             para.C[row * para.ldc + col] += 
    //                 para.A[row * para.lda + k] * para.B[k * para.ldb + col];
    //         }
    //     }
    // }


    // int ldm_size = get_allocatable_size();
    // if (CRTS_tid == 0) {
    //     printf("get_allocatable_size=%d byte\n", ldm_size);
    // }
    // // 一次算一个C，暂存A的一行和B的一列
    // double buf_a[para.k * 2], buf_b[para.k * 2];
    // for (int i = 0; i < block_m; ++i) {
    //     int row = row_start + i;
    //     CRTS_dma_get(buf_a, &para.A[row * para.lda + 0], para.k * sizeof(double));
    //     for (int j = 0; j < block_m; ++j) {
    //         int col = col_start + j;
    //         // CRTS_dma_get(buf_b, &para.B[0 * para.ldb + col], para.k * sizeof(double));
    //         CRTS_dma_get_stride(buf_b, &para.B[0 * para.ldb + col], para.k * sizeof(double), 1 * sizeof(double), (para.ldb - 1) * sizeof(double));
    //         for (int k = 0; k < para.k; ++k) {
    //             // para.C[row * para.ldc + col] += 
    //             //     para.A[row * para.lda + k] * para.B[k * para.ldb + col];
    //             para.C[row * para.ldc + col] += buf_a[k] * buf_b[k];
    //         }
    //     }
    // }


    // // 一个从核负责计算C的block_m * block_n
    // // 一次算mm * nn 的 block_m * block_n
    // // 并且暂存A的mm * k和B的 k * nn
    // // 计算结果放到mm * nn的buf_c中
    // // 最后把buf中的计算结果放到c中
    // // 问题：K的维度打的时候，ldm空间不够，需要对K方向分块
    // int mm = 10, nn = 10;
    // double buf_a[mm * para.k], buf_b[para.k * nn], buf_c[mm * nn];
    // // memset(buf_c, 0, mm*nn*sizeof(double));
    // // memset(buf_c, 0, sizeof(buf_c));
    // for (int i = 0; i < block_m; i += mm) {
    //     int row = row_start + i;
    //     CRTS_dma_get_stride(buf_a, &para.A[row * para.lda + 0], mm * para.k * sizeof(double), para.k * sizeof(double), (para.lda - para.k) * sizeof(double));
    //     for (int j = 0; j < block_m; j += nn) {
    //         int col = col_start + j;
    //         CRTS_dma_get_stride(buf_b, &para.B[0 * para.ldb + col], nn * para.k * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double));
    //         // 计算小块mm * nn 到buf_c中
    //         memset(buf_c, 0, sizeof(buf_c));
    //         for (int ii = 0; ii < mm; ++ii) {
    //             for (int jj = 0; jj < nn; ++jj) {
    //                 for (int k = 0; k < para.k; ++k) {
    //                     // 此次非常耗时的debug操作：
    //                     // 1:buf_b[k * nn + jj]的跨度变量写错 2: 循环变量ii,jj使用和i 与 j混淆（以后不要命名如此沙雕的循环变量）
    //                     buf_c[ii * nn + jj] += buf_a[ii * para.k + k] * buf_b[k * nn + jj];
    //                 }
    //             }
    //         }
    //         // if (CRTS_tid == 0) {
    //         //     printf("come here\n");
    //         // }
    //         CRTS_dma_put_stride(&para.C[row * para.ldc + col], buf_c, mm * nn * sizeof(double), nn * sizeof(double), (para.ldc - nn) * sizeof(double));
    //     }
    // }


    // // 问题：K的维度打的时候，ldm空间不够，需要对K方向分块
    // // 对上面的方法，对K方向分块优化
    // // 方法非常慢：多重循环，最内层的循环变量是使用最频繁的；所以耗时的变量应该放到外层循环
    // int mm = 10, nn = 10;
    // int block_k = 1024; // 8 * 1KB
    // int dma_get_times = para.k / block_k;
    // int dma_last_num  = para.k % block_k;

    // double buf_a[mm * block_k], buf_b[block_k * nn], buf_c[mm * nn]; // 80KB + 80KB + 0.8KB
    // for (int i = 0; i < block_m; i += mm) {
    //     int row = row_start + i;
    //     for (int j = 0; j < block_m; j += nn) {
    //         int col = col_start + j;
    //         // 计算小块mm * nn 到buf_c中
    //         memset(buf_c, 0, sizeof(buf_c));
    //         for (int ii = 0; ii < mm; ++ii) {
    //             for (int jj = 0; jj < nn; ++jj) {
    //                 for (int dma_time = 0; dma_time < dma_get_times; ++dma_time) {
    //                 // 一次够block_k
    //                     CRTS_dma_get_stride(buf_a, &para.A[row * para.lda + (0 + dma_time * block_k)],
    //                         mm * block_k * sizeof(double), block_k * sizeof(double), (para.lda - block_k) * sizeof(double));
    //                     CRTS_dma_get_stride(buf_b, &para.B[(0 + dma_time * block_k) * para.ldb + col],
    //                         block_k * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double));
    //                     for (int k = 0; k < block_k; ++k) {
    //                         buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                     }
    //                 }
    //                 // 最后一次dma的数量
    //                 CRTS_dma_get_stride(buf_a, &para.A[row * para.lda + (0 + dma_get_times * block_k)],
    //                     mm * dma_last_num * sizeof(double), dma_last_num * sizeof(double), (para.lda - dma_last_num) * sizeof(double));
    //                 CRTS_dma_get_stride(buf_b, &para.B[(0 + dma_get_times * block_k) * para.ldb + col],
    //                     dma_last_num * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double));
    //                 for (int k = 0; k < block_k; ++k) {
    //                     buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                 }
    //             }
    //         }
    //         CRTS_dma_put_stride(&para.C[row * para.ldc + col], buf_c, mm * nn * sizeof(double), nn * sizeof(double), (para.ldc - nn) * sizeof(double));
    //     }
    // }

    // // 问题：K的维度打的时候，ldm空间不够，需要对K方向分块
    // // 对上面的方法，对K方向分块优化
    // // 优化循环变量：----->>>>>> 方法非常慢：多重循环，最内层的循环变量是使用最频繁的；所以耗时的变量应该放到外层循环
    // int mm = 10, nn = 10;
    // int block_k = 1024; // 8 * 1KB
    // int dma_get_times = para.k / block_k;
    // int dma_last_num  = para.k % block_k;

    // double buf_a[mm * block_k], buf_b[block_k * nn], buf_c[mm * nn]; // 80KB + 80KB + 0.8KB
    // for (int i = 0; i < block_m; i += mm) {
    //     int row = row_start + i;
    //     for (int j = 0; j < block_m; j += nn) {
    //         int col = col_start + j;
    //         // 计算小块mm * nn 到buf_c中
    //         memset(buf_c, 0, sizeof(buf_c));
    //         for (int dma_time = 0; dma_time < dma_get_times; ++dma_time) { // 优化循环变量：----->>>>>> dma放到内层
    //             // 一次够block_k
    //             CRTS_dma_get_stride(buf_a, &para.A[row * para.lda + (0 + dma_time * block_k)],
    //                 mm * block_k * sizeof(double), block_k * sizeof(double), (para.lda - block_k) * sizeof(double));
    //             CRTS_dma_get_stride(buf_b, &para.B[(0 + dma_time * block_k) * para.ldb + col],
    //                 block_k * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double));
    //             for (int ii = 0; ii < mm; ++ii) {
    //                 for (int jj = 0; jj < nn; ++jj) {
    //                     for (int k = 0; k < block_k; ++k) {
    //                         buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                     }
    //                 }
    //             }
    //         }
    //         // 最后一次dma的数量
    //         CRTS_dma_get_stride(buf_a, &para.A[row * para.lda + (0 + dma_get_times * block_k)],
    //             mm * dma_last_num * sizeof(double), dma_last_num * sizeof(double), (para.lda - dma_last_num) * sizeof(double));
    //         CRTS_dma_get_stride(buf_b, &para.B[(0 + dma_get_times * block_k) * para.ldb + col],
    //             dma_last_num * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double));
    //         for (int ii = 0; ii < mm; ++ii) {
    //             for (int jj = 0; jj < nn; ++jj) {
    //                 for (int k = 0; k < block_k; ++k) {
    //                     buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                 }
    //             }
    //         }
    //         CRTS_dma_put_stride(&para.C[row * para.ldc + col], buf_c, mm * nn * sizeof(double), nn * sizeof(double), (para.ldc - nn) * sizeof(double));
    //     }
    // }



    // // 优化方向：----->>>>>> DMA 行广播、列广播
    // // 维度为   960      960     960 的时候，就开始算错了
    // int mm = 10, nn = 10;
    // int block_k = 1024; // 8 * 1KB
    // int dma_get_times = para.k / block_k;
    // int dma_last_num  = para.k % block_k;

    // // printf("[row %d, col %d, num %d]: before ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);
    // double buf_a[mm * block_k], buf_b[block_k * nn], buf_c[mm * nn]; // 80KB + 80KB + 0.8KB
    // // printf("[row %d, col %d, num %d]: after ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);

    // for (int i = 0; i < block_m; i += mm) {
    //     int row = row_start + i;
    //     for (int j = 0; j < block_m; j += nn) {
    //         int col = col_start + j;
    //         // 计算小块mm * nn 到buf_c中
    //         memset(buf_c, 0, sizeof(buf_c));
    //         for (int dma_time = 0; dma_time < dma_get_times; ++dma_time) { // 优化循环变量：----->>>>>> dma放到内层
    //             // 一次够block_k
    //             dma_rply = 0;
    //             CRTS_ssync_array ();
    //             if (!(_COL ^ _ROW)) {               // 主对角线上的从核，取矩阵A的K维度的block广播到同行
    //                 CRTS_dma_row_ibcast_stride(buf_a, &para.A[row * para.lda + (0 + dma_time * block_k)],
    //                     mm * block_k * sizeof(double), block_k * sizeof(double), (para.lda - block_k) * sizeof(double), &dma_rply);
    //             } else if (!(_COL ^ _ROW ^ 7)) {    // 副对角线上的从核，取矩阵B的K维度的block广播到同列
    //                 CRTS_dma_col_ibcast_stride(buf_b, &para.B[(0 + dma_time * block_k) * para.ldb + col],
    //                     block_k * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double), &dma_rply);
    //             }
    //             // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
    //             CRTS_dma_wait_value (&dma_rply, 2); // //保证列上其他从核看见最新数据
    //             // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
    //             CRTS_ssync_array ();
    //             for (int ii = 0; ii < mm; ++ii) {
    //                 for (int jj = 0; jj < nn; ++jj) {
    //                     for (int k = 0; k < block_k; ++k) {
    //                         buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                     }
    //                 }
    //             }
    //         }

            
    //         // __thread_local crts_rply_t dma_rply = 0;    
    //         // 最后一次dma的数量
    //         dma_rply = 0;
    //         // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
    //         CRTS_ssync_array ();
    //         if (!(_COL ^ _ROW)) {               // 主对角线上的从核，取矩阵A的K维度的block广播到同行
    //             CRTS_dma_row_ibcast_stride(buf_a, &para.A[row * para.lda + (0 + dma_get_times * block_k)],
    //                 mm * dma_last_num * sizeof(double), dma_last_num * sizeof(double), (para.lda - dma_last_num) * sizeof(double), &dma_rply); 
    //         } else if (!(_COL ^ _ROW ^ 7)) {    // 副对角线上的从核，取矩阵B的K维度的block广播到同列
    //             CRTS_dma_col_ibcast_stride(buf_b, &para.B[(0 + dma_get_times * block_k) * para.ldb + col],
    //                 dma_last_num * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double), &dma_rply);
    //         }
    //         // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
    //         CRTS_dma_wait_value (&dma_rply, 2); // //保证列上其他从核看见最新数据
    //         // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
    //         CRTS_ssync_array ();

    //         for (int ii = 0; ii < mm; ++ii) {
    //             for (int jj = 0; jj < nn; ++jj) {
    //                 for (int k = 0; k < block_k; ++k) {
    //                     buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
    //                 }
    //             }
    //         }

    //         CRTS_dma_put_stride(&para.C[row * para.ldc + col], buf_c, mm * nn * sizeof(double), nn * sizeof(double), (para.ldc - nn) * sizeof(double));
    //     }
    // }

    // 优化方向：----->>>>>> DMA 行广播、列广播 + 循环展开
    int mm = 10, nn = 10;
    int block_k = 1024; // 8 * 1KB
    int dma_get_times = para.k / block_k;
    int dma_last_num  = para.k % block_k;

    // printf("[row %d, col %d, num %d]: before ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);
    double buf_a[mm * block_k], buf_b[block_k * nn], buf_c[mm * nn]; // 80KB + 80KB + 0.8KB
    // printf("[row %d, col %d, num %d]: after ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);

    for (int i = 0; i < block_m; i += mm) {
        int row = row_start + i;
        for (int j = 0; j < block_m; j += nn) {
            int col = col_start + j;
            // 计算小块mm * nn 到buf_c中
            memset(buf_c, 0, sizeof(buf_c));
            for (int dma_time = 0; dma_time < dma_get_times; ++dma_time) { // 优化循环变量：----->>>>>> dma放到内层
                // 一次够block_k
                dma_rply = 0;
                CRTS_ssync_array ();
                if (!(_COL ^ _ROW)) {               // 主对角线上的从核，取矩阵A的K维度的block广播到同行
                    CRTS_dma_row_ibcast_stride(buf_a, &para.A[row * para.lda + (0 + dma_time * block_k)],
                        mm * block_k * sizeof(double), block_k * sizeof(double), (para.lda - block_k) * sizeof(double), &dma_rply);
                } else if (!(_COL ^ _ROW ^ 7)) {    // 副对角线上的从核，取矩阵B的K维度的block广播到同列
                    CRTS_dma_col_ibcast_stride(buf_b, &para.B[(0 + dma_time * block_k) * para.ldb + col],
                        block_k * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double), &dma_rply);
                }
                // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
                CRTS_dma_wait_value (&dma_rply, 2); // //保证列上其他从核看见最新数据
                // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
                CRTS_ssync_array ();
                for (int ii = 0; ii < mm; ++ii) {
                    for (int jj = 0; jj < nn; ++jj) {
                        for (int k = 0; k < block_k; ++k) {
                            buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
                        }
                    }
                }
            }

            
            // __thread_local crts_rply_t dma_rply = 0;    
            // 最后一次dma的数量
            dma_rply = 0;
            // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
            CRTS_ssync_array ();
            if (!(_COL ^ _ROW)) {               // 主对角线上的从核，取矩阵A的K维度的block广播到同行
                CRTS_dma_row_ibcast_stride(buf_a, &para.A[row * para.lda + (0 + dma_get_times * block_k)],
                    mm * dma_last_num * sizeof(double), dma_last_num * sizeof(double), (para.lda - dma_last_num) * sizeof(double), &dma_rply); 
            } else if (!(_COL ^ _ROW ^ 7)) {    // 副对角线上的从核，取矩阵B的K维度的block广播到同列
                CRTS_dma_col_ibcast_stride(buf_b, &para.B[(0 + dma_get_times * block_k) * para.ldb + col],
                    dma_last_num * nn * sizeof(double), nn * sizeof(double), (para.ldb - nn) * sizeof(double), &dma_rply);
            }
            // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
            CRTS_dma_wait_value (&dma_rply, 2); // //保证列上其他从核看见最新数据
            // printf("[row %d, col %d, num %d]: dma_rply:%d\n", _ROW, _COL, _PEN, dma_rply);
            CRTS_ssync_array ();

            for (int ii = 0; ii < mm; ++ii) {
                for (int jj = 0; jj < nn; ++jj) {
                    for (int k = 0; k < block_k; ++k) {
                        buf_c[ii * nn + jj] += buf_a[ii * block_k + k] * buf_b[k * nn + jj];
                    }
                }
            }

            CRTS_dma_put_stride(&para.C[row * para.ldc + col], buf_c, mm * nn * sizeof(double), nn * sizeof(double), (para.ldc - nn) * sizeof(double));
        }
    }

}


void big_kernal(SWARGS* args)
{
    SWARGS para;
    CRTS_dma_get(&para, args, sizeof(SWARGS));
    // printf("args:%d %d %d\n", para.m, para.n, para.k);

    // 每个从核负责计算C的（row_start，col_start） 块大小为block_m * block_n的块
    int block_m_size = 8;
    int block_n_size = 8;
    int block_k_size = 8;

    int block_m = para.m / block_m_size;
    int block_n = para.n / block_n_size;
    int block_k = para.k / block_k_size;

    int row_a_c_start = CRTS_rid * block_m;
    int col_b_c_start = CRTS_cid * block_n;

    int col_a_start = CRTS_cid * block_k;
    int row_b_start = CRTS_rid * block_k;

    // 优化方向：----->>>>>> DMA + RMA

    int A_buffer_size = 36 * 1024 / 8;
    int B_buffer_size = 36 * 1024 / 8;
    double A_dma_buffer[A_buffer_size]; // 36KB
    double A_rma_buffer[A_buffer_size]; // 36KB
    double B_dma_buffer[B_buffer_size]; // 36KB
    double B_rma_buffer[B_buffer_size]; // 36KB
    double C_buf[block_m * block_n];
    double C_rma_buf[block_m * block_n];
    double C_result_buf[block_m * block_n];


    // printf("[row %d, col %d, num %d]: before ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);

    // DMA加载A的块到LDM
    // DMA加载B的块到LDM
    // RMA转置B的块
    // 计算 一次A*B
    // 规约一次（寄存器或RMA），得对角线的结果
    // 结果写回
    // Do i 1，7
    //          RMA上移B的块
    //          计算 A*B
    //          规约（寄存器或RMA），得对角线 + 1 的结果
    //          结果写回

    dma_rply = 0;
    // 1、DMA加载A的块到LDM
    CRTS_dma_iget_stride(A_dma_buffer, &para.A[row_a_c_start * para.lda + col_a_start],
        block_m * block_k * sizeof(double), block_m * sizeof(double), (para.lda - block_m) * sizeof(double), &dma_rply);
    // 2、DMA加载B的块到LDM
    CRTS_dma_iget_stride(B_dma_buffer, &para.B[row_b_start * para.ldb + col_b_c_start],
        block_k * block_n * sizeof(double), block_n * sizeof(double), (para.ldb - block_n) * sizeof(double), &dma_rply);
    CRTS_dma_wait_value (&dma_rply, 2);
    // printf("[row %d, col %d, num %d]: after ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);

    dma_rply = 0;
    athread_ssync_array();

    // 3、RMA转置B的块
    if ((_COL * 8 + _ROW) == _PEN) {
        // athread_rma_put(A_dma_buffer, block_m * block_k * sizeof(double),
        CRTS_rma_put(A_dma_buffer, block_m * block_k * sizeof(double),
                        _COL * 8 + _ROW, A_rma_buffer, &dma_rply);
    }
    // 4、计算 一次A*B
    for(int i = 0; i < block_m; ++i) {
        for(int j = 0; j < block_n; ++j) {
            for ( int k = 0; k < block_k; ++k)  {
                C_buf[i*block_n + j] = A_rma_buffer[i * block_k + k] +  B_rma_buffer[k * block_m + j];
            }
        }
    }
    // 5、规约一次（寄存器或RMA），得对角线的结果
    // dma_rply = 0;
    // athread_ssync_array();
    // if(_ROW == 0) {
    //     if (_COL % 2 == 0) { // 0 2 4 6
    //         CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
    //                     _ROW * 8 + _ROW + 1, C_buf, &dma_rply);
    //     }
    // }

    dma_rply = 0;
    athread_ssync_array();
    if (_COL % 2 == 0) { // 0 2 4 6
        CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                    _ROW * 8 + _ROW + 1, C_buf, &dma_rply);
        for(int i = 0; i < block_m; ++i) {
            for(int j = 0; j < block_n; ++j) {
                    C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
            }
        }
    }
    if (_COL % 4 == 0) { // 0  4 
        CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                    _ROW * 8 + _ROW + 2, C_buf, &dma_rply);
        for(int i = 0; i < block_m; ++i) {
            for(int j = 0; j < block_n; ++j) {
                    C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
            }
        }
    }
    if (_COL % 8 == 0) { // 0
        CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                    _ROW * 8 + _ROW + 4, C_buf, &dma_rply);
        for(int i = 0; i < block_m; ++i) {
            for(int j = 0; j < block_n; ++j) {
                    C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
            }
        }
    }

    // 6、结果存到对应从核
    dma_rply = 0;
    athread_ssync_array();
    if(!(_ROW ^ _COL)) {
        CRTS_rma_get(C_result_buf, block_m * block_k * sizeof(double),
                    _ROW * 8 + 0, C_buf, &dma_rply);
    }



    //  7、做循环
    // Do i 1，7
    //          RMA上移B的块
    //          计算 A*B
    //          规约（寄存器或RMA），得对角线 + 1 的结果
    //          结果写回
    dma_rply = 0;
    athread_ssync_array();
    for(int i = 1; i <= 7; ++i) {
        if(i % 2 == 1) {
            //  RMA上移B的块
            CRTS_rma_get(C_buf, block_m * block_k * sizeof(double),
                         (_ROW - 1 + 8) % 8 * 8 + _COL, C_rma_buf, &dma_rply); // [(_ROW - 1 + 8) % 8, _COL] 上排从核号坐标
            // 计算 一次A*B
            for(int i = 0; i < block_m; ++i) {
                for(int j = 0; j < block_n; ++j) {
                    for ( int k = 0; k < block_k; ++k)  {
                        C_buf[i*block_n + j] = A_rma_buffer[i * block_k + k] +  B_rma_buffer[k * block_m + j];
                    }
                }
            }
            // 规约（寄存器或RMA），得对角线 + 1 的结果
            if (_COL % 2 == 0) { // 0 2 4 6
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 1, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if (_COL % 4 == 0) { // 0  4 
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 2, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if (_COL % 8 == 0) { // 0
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 4, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if(!(_ROW ^ (_COL - i + 8) % 8)) { // 右移对角线上的从核
                CRTS_rma_get(C_result_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + 0, C_buf, &dma_rply);
            }
            
        } else {
            //  RMA上移B的块
            CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                         (_ROW - 1 + 8) % 8 * 8 + _COL, C_buf, &dma_rply); // [(_ROW - 1 + 8) % 8, _COL] 上排从核号坐标
            // 计算 一次A*B
            for(int i = 0; i < block_m; ++i) {
                for(int j = 0; j < block_n; ++j) {
                    for ( int k = 0; k < block_k; ++k)  {
                        C_buf[i*block_n + j] = A_rma_buffer[i * block_k + k] +  B_rma_buffer[k * block_m + j];
                    }
                }
            }
            // mul_add_AB_to_C(A_rma_buffer, B_rma_buffer, C_buf, block_m, block_n, block_k);

            // 规约（寄存器或RMA），得对角线 + 1 的结果
            if (_COL % 2 == 0) { // 0 2 4 6
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 1, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if (_COL % 4 == 0) { // 0  4 
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 2, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if (_COL % 8 == 0) { // 0
                CRTS_rma_get(C_rma_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + _ROW + 4, C_buf, &dma_rply);
                for(int i = 0; i < block_m; ++i) {
                    for(int j = 0; j < block_n; ++j) {
                            C_buf[i*block_n + j] += C_rma_buf[i * block_n + j];
                    }
                }
                // sum_C(C_buf, C_rma_buf, block_m, block_n);
            }
            if(!(_ROW ^ (_COL - i + 8) % 8)) { // 右移对角线上的从核
                CRTS_rma_get(C_result_buf, block_m * block_k * sizeof(double),
                            _ROW * 8 + 0, C_buf, &dma_rply);
            }
        }
    }

    // Last, 一次DMA结果写回
    // printf("[row %d, col %d, num %d]: after ldm space: %dKB \n", _ROW, _COL, _PEN, CRTS_pldm_get_free_size()/1024);
    // dma_rply = 0;
    // CRTS_dma_iput_stride(C_result_buf, &para.C[row_a_c_start * para.ldc + col_b_c_start],
    //     block_m * block_n * sizeof(double), block_n * sizeof(double), (para.ldc - block_n) * sizeof(double), &dma_rply);
    // CRTS_dma_wait_value (&dma_rply, 1);

    // CRTS_dma_put_stride(C_result_buf, &para.C[row_a_c_start * para.ldc + col_b_c_start],
    //     block_m * block_n * sizeof(double), block_n * sizeof(double), (para.ldc - block_n) * sizeof(double));

}

void sum_C(double* dest, double* src, int m, int n) {
    for(int i = 0; i < m; ++i) {
        for(int j = 0; j < n; ++j) {
                dest[i * n + j] += src[i * n + j];
        }
    }
}

void mul_add_AB_to_C(double* A, double* B, double* C, int M, int N, int K)
{
    for(int i = 0; i < M; ++i) {
            for(int j = 0; j < N; ++j) {
                for ( int k = 0; k < K; ++k)  {
                    C[i * N + j] = A[i * K + k] +  B[k * N + j];
                }
            }
    }
}


    // //阻塞 RMA 读
    // reply=0;
    // if(_PEN==0)
    //     a=1;
    // athread_ssync_array();
    // athread_rma_get(&a,4,0,&a,&reply); // 可以想象，每个从核都从0号从核LDM空间读取数据
    // if(a!=1)
    //     flag+=1;
    
    // //非阻塞 RMA 读
    // athread_ssync_array();
    // reply=0;
    // if(_PEN==0)
    //     a=2;
    // athread_ssync_array();
    // athread_rma_iget(&a,&reply,4,0,&a,&reply);
    // athread_rma_wait_value(&reply,1);
    // if(a!=2)
    //     flag+=1;

    // //阻塞 RMA 写
    // // 0号从核 和 1号从核通信示例
    // athread_ssync_array();
    // reply=0;
    // athread_ssync_array();
    // if(_PEN==0){
    //     a=3;
    //     athread_rma_put(&a,4,1,&a,&reply);
    // }else if(_PEN==1){
    //     athread_rma_wait_value(&reply,1);
    //     if(a!=3)
    //     flag+=1;
    // }