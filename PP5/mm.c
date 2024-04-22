#include <stdio.h>
#include <stdlib.h>
#include<sys/time.h>
#include <omp.h>

const int N = 2; 

float *A, *B, *C;

int avg_rows;

void initialize_matrix(float* matrix, int size, float low, float high) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            // 生成[0, 1)之间的随机浮点数
            float randomValue = (float)rand() / (float)RAND_MAX;
            
            // 调整随机数的范围到[low, high)
            *(matrix + i * size + j) = low + randomValue * (high - low);
        }
    }
}


void omp_matrix_mul() {
    int rank = omp_get_thread_num();
    int first_row = rank * avg_rows;
    int last_row = (rank + 1) * avg_rows;
    for (int i = first_row; i < last_row; i ++) {
        for (int j = 0; j < N; j ++) {
            float temp = 0;
            for (int z = 0; z < N; z ++) {
                temp += A[i * N + z] * B[z * N + j];
            }
            C[i * N + j] = temp;
        }
    }
}

int main(int argc, char* argv[]) {
    A = (float *)malloc(sizeof(float) * N * N);
    B = (float *)malloc(sizeof(float) * N * N);
    C = (float *)malloc(sizeof(float) * N * N);

    initialize_matrix(A, N, 0., 10.);
    initialize_matrix(B, N, 0., 10.);

    int num_thread = strtol(argv[1], NULL, 10);
    avg_rows = N / num_thread;

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);
    // 执行并行矩阵乘法
    # pragma omp parallel num_threads(num_thread)
    omp_matrix_mul();
    gettimeofday(&end_time, NULL);

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long useconds = end_time.tv_usec - start_time.tv_usec;

    if (useconds < 0) {
        useconds += 1000000;  // 微秒数加上1秒的微秒数
        seconds -= 1;         // 秒数减1
    }

    printf("Matrix A:\n");
    for (int i = 0; i < N; i ++) {
        for (int j = 0; j < N; j ++) {
            printf("%f ", A[i * N + j]);
        }
        printf("\n");
    }
    printf("Matrix B:\n");
    for (int i = 0; i < N; i ++) {
        for (int j = 0; j < N; j ++) {
            printf("%f ", B[i * N + j]);
        }
        printf("\n");
    }
    printf("Matrix C:\n");
    for (int i = 0; i < N; i ++) {
        for (int j = 0; j < N; j ++) {
            printf("%f ", C[i * N + j]);
        }
        printf("\n");
    }

    // 将时间转换为以秒为单位的浮点数
    double total_seconds = seconds + useconds / 1000000.0;

    printf("Time Elapsed = %.6f seconds\n", total_seconds);

    return 0;
}