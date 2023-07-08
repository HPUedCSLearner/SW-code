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


