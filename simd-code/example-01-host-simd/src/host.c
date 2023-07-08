#include <simd.h> //包含 SIMD 头文件
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void array_sum(double* arr, long size, double *result);

#define N (1e8)


void test1()
{
    int arr[8] __attribute__ ((aligned (32)))= {1,2,3,4,5,6,7,8};
    int i,res[8],t=0;
    intv8 va,vb,vi;
    simd_load (va,arr);
    simd_print_intv8(va);
    for (i=16;i>=1;i>>=1) {
        vi = simd_set_intv8(i,i,i,i,i,i,i,i); //从标准类型向扩展类型赋值
        va ^= simd_vsraw(va,i); //扩展类型的变量使用运算符号
    }
    vb=simd_veqvw(va,vi); //扩展类型的变量使用扩展的内部函数接口
    simd_print_intv8(vb); //用 intv8 类型的格式打印
    simd_print_intv8(va); //用 intv8 类型的格式打印
    simd_store(va, res); //intv8 类型的存储
    for (i=0; i<8; i++) 
    {
        t =t+ res[i];
        printf("%d\n",t);
    }
}


int main()
{
	clock_t			tst_time, ted_time;
	double			result = 0, *arr;
    long			i;


	arr = (double *)malloc(N * sizeof(double));
	for (i = 0; i < N; i++) {
		arr[i] = 1.0;
	}

	tst_time = clock();

    // test1();
	array_sum(arr, (long)N, &result);




	ted_time = clock();

	printf("Sum of array is : %.2lf\n", result);

	printf("Time(time) : %.2lf ms\n", (double)(ted_time - tst_time) * 1000 / CLOCKS_PER_SEC);

    free(arr);
    return 0;
}

#define SIMD_OPT

void array_sum(double* arr, long size, double *result) {
	*result = 0;

#ifndef SIMD_OPT

	printf("ues no SIMD OPT api\n");
	long i;
	for (i = 0; i < size; i++) {  // 520ms
		*result += arr[i];
	}

#else
    doublev4 dbv4 = 0, dbv4_tmp = 0;
    // simd_print_doublev4(dbv4);
    // simd_load(dbv4_tmp, arr);
    // simd_print_doublev4(dbv4_tmp);

    long simd_strip = 4, index;
    if (size % simd_strip == 0) { // 150ms
        for (int i = 0; i < size; i += 4) {
            simd_load(dbv4_tmp, &arr[i]);
            dbv4 += dbv4_tmp;
        }
    } else { // 210ms
        for (int i = 0; i < size; i += 4) { //  i += 4 意味着：循环体只是原来的1/4
            if (i + simd_strip > size) { // 尽量不要在simd前面搞判断、很浪费
                index = i;
                break;
            }
            simd_load(dbv4_tmp, &arr[i]);
            dbv4 += dbv4_tmp;
        }
        printf("index: %ld\n", index);
        while (index < size) {
            *result += arr[index++];
        }
    }

    *result = dbv4[0] + dbv4[1] + dbv4[2] + dbv4[3];

#endif

}