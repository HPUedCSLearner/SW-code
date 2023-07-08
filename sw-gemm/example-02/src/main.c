#include "common_master.h"

int main(){
    // 1. ahtread env init
    // CRTS_init();
    crts_env_init();

    // 2. master-slave-api
    // test_master_api();
    // test_spawn_join();
    int dim_begin = 320, dim_end = 4000;
    printf( "M\t K\t N\t SLEF_FLOPS\t REF_FLOPS\t SPEED_UP\n");
    for (int dim = dim_begin; dim <= dim_end; dim+= 80)
    {
        test_gemm(dim, dim, dim);
    }
    // test_gemm(400, 400, 400);


    // 3. atherad env halt
    // CRTS_athread_halt();
    crts_env_halt();
    return 0;
}