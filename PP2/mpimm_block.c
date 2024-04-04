#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>   
#include <string.h>

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


void matrix_multiply(float* A, float* B, float* C, int block_rows, int block_cols, int N) {
    for (int i = 0; i < block_rows; ++i) {
        for (int j = 0; j < block_cols; ++j) {
            float sum = 0;
            for (int x = 0; x < N; ++x) {
                sum += *(A + i * N + x) * *(B + x * N + j);
            }
            *(C + i * N + j) = sum;
        }
    }
}

int main(){
    srand(time(NULL));
    int N = 256;   // 方阵大小

    float *A, *B, *C, *localA, *localB, *localC;
    double elapsed = 0.;

    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;// MPI通信子
    MPI_Init(NULL, NULL);          // 初始化MPI
    MPI_Comm_size(comm, &comm_sz); // 获取通信子大小（进程数量）
    MPI_Comm_rank(comm, &my_rank); // 获取进程编号

    A = (float *)malloc(sizeof(float) * N * N);
    B = (float *)malloc(sizeof(float) * N * N);
    C = (float *)malloc(sizeof(float) * N * N);

    int block_rows = N, block_cols = N;

    if (comm_sz == 2) {
        block_rows = N / 2;
    }
    else if (comm_sz == 4) {
        block_rows = N / 2;
        block_cols = N / 2;
    }
    else if (comm_sz == 8) {
        block_rows = N / 4;
        block_cols = N/ 2;
    }
    else if (comm_sz == 16) {
        block_rows = N / 4;
        block_cols = N / 4;
    }

    localA = (float *)malloc(sizeof(float) * block_rows * N);
    localB = (float *)malloc(sizeof(float) * block_cols * N);
    localC = (float *)malloc(sizeof(float) * block_rows * block_cols);

    // send matrix
    if (my_rank == 0) {
        // init mat
        initialize_matrix(A, N, 0., 10.);
        initialize_matrix(B, N, 0., 10.);
        memset(C, 0, sizeof(float) * N * N);
    }
    // scatter A
    MPI_Scatter(A, N * block_rows, MPI_FLOAT, localA, N * block_rows, MPI_FLOAT, 0, MPI_COMM_WORLD);
    // scatter B
    MPI_Scatter(B, N * block_cols, MPI_FLOAT, localB, N * block_cols, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // matrix multiply and recv
    if (my_rank == 0) {
        double local_start, local_end, local_elapsed;
        MPI_Barrier(MPI_COMM_WORLD);
        local_start = MPI_Wtime();
        // Core 0 own mat mul
        matrix_multiply(localA, localB, localC, block_rows, block_cols, N);
        local_end = MPI_Wtime();
        local_elapsed = local_end - local_start;

        MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        // gather c
        MPI_Gather(localC, block_rows * block_cols, MPI_FLOAT, C, block_rows * block_cols, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // for (int i = 0; i < N; i ++) {
        //     for (int j = 0; j < N; j ++) {
        //         printf("%f ", A[i * N + j]);
        //     }
        //     printf("\n");
        // }

        // for (int i = 0; i < N; i ++) {
        //     for (int j = 0; j < N; j ++) {
        //         printf("%f ", B[i * N + j]);
        //     }
        //     printf("\n");
        // }

        // for (int i = 0; i < N; i ++) {
        //     for (int j = 0; j < N; j ++) {
        //         printf("%f ", C[i * N + j]);
        //     }
        //     printf("\n");
        // }
        printf("Cost time: %f\n", elapsed);
    }
    else {
        double local_start, local_end, local_elapsed;
        MPI_Barrier(MPI_COMM_WORLD);
        local_start = MPI_Wtime();
        // Core 0 own mat mul
        matrix_multiply(localA, localB, localC, block_rows, block_cols, N);
        local_end = MPI_Wtime();
        local_elapsed = local_end - local_start;

        MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        // gather c
        MPI_Gather(localC, block_rows * block_cols, MPI_FLOAT, C, block_rows * block_cols, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    free(A);
    free(B);
    free(C);
    free(localA);
    free(localC);
    return 0;
}