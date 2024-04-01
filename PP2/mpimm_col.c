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


void matrix_multiply(float* A, float* B, float* C, int size, int avg_rows) {
    for (int i = 0; i < avg_rows; ++i) {
        for (int j = 0; j < size; ++j) {
            float sum = 0;
            for (int x = 0; x < size; ++x) {
                sum += *(A + i * size + x) * *(B + x * size + j);
            }
            *(C + i * size + j) = sum;
        }
    }
}

int main(){
    srand(time(NULL));
    int N = 128;   // 方阵大小

    float *A, *B, *C, *localA, *localC;
    double elapsed = 0.;

    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;// MPI通信子
    MPI_Init(NULL, NULL);          // 初始化MPI
    MPI_Comm_size(comm, &comm_sz); // 获取通信子大小（进程数量）
    MPI_Comm_rank(comm, &my_rank); // 获取进程编号

    A = (float *)malloc(sizeof(float) * N * N);
    B = (float *)malloc(sizeof(float) * N * N);
    C = (float *)malloc(sizeof(float) * N * N);

    int avg_rows = N;
    if (comm_sz > 1) 
        avg_rows = N / comm_sz;

    localA = (float *)malloc(sizeof(float) * avg_rows * N);
    localC = (float *)malloc(sizeof(float) * avg_rows * N);

    // send matrix
    if (my_rank == 0) {
        // init mat
        initialize_matrix(A, N, 0., 10.);
        initialize_matrix(B, N, 0., 10.);
        memset(C, 0, sizeof(float) * N * N);

        // scatter A
        MPI_Scatter(A, N * avg_rows, MPI_FLOAT, localA, N * avg_rows, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    // broadcast B
    MPI_Bcast(B, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // matrix multiply and recv
    if (my_rank == 0) {
        double local_start, local_end, local_elapsed;
        MPI_Barrier(MPI_COMM_WORLD);
        local_start = MPI_Wtime();
        // Core 0 own mat mul
        matrix_multiply(A, B, localC, N, avg_rows);
        local_end = MPI_Wtime();
        local_elapsed = local_end - local_start;

        MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        // gather c
        MPI_Gather(localC, avg_rows * N, MPI_FLOAT, C, avg_rows * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

        for (int i = 0; i < N; i ++) {
            for (int j = 0; j < N; j ++) {
                printf("%f ", A[i * N + j]);
            }
            printf("\n");
        }

        for (int i = 0; i < N; i ++) {
            for (int j = 0; j < N; j ++) {
                printf("%f ", B[i * N + j]);
            }
            printf("\n");
        }

        for (int i = 0; i < N; i ++) {
            for (int j = 0; j < N; j ++) {
                printf("%f ", C[i * N + j]);
            }
            printf("\n");
        }
        printf("Cost time: %f\n", elapsed);
    }
    else {
        // scatter A
        MPI_Scatter(A, N * avg_rows, MPI_FLOAT, localA, N * avg_rows, MPI_FLOAT, 0, MPI_COMM_WORLD);

        double local_start, local_end, local_elapsed;
        MPI_Barrier(MPI_COMM_WORLD);
        local_start = MPI_Wtime();
        // Core 0 own mat mul
        matrix_multiply(localA, B, localC, N, avg_rows);
        local_end = MPI_Wtime();
        local_elapsed = local_end - local_start;

        MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        // gather c
        MPI_Gather(localC, avg_rows * N, MPI_FLOAT, C, avg_rows * N, MPI_FLOAT, 0, MPI_COMM_WORLD);


    }

    MPI_Finalize();
    free(A);
    free(B);
    free(C);
    free(localA);
    free(localC);
    return 0;
}