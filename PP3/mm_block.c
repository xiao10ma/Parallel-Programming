#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long thread_cnt;
float* A, *B, *C;
const long N = 2048;
long block_rows = N, block_cols = N;

int counter;
pthread_mutex_t mutex;
pthread_cond_t cond_var;

double max_elapsed_time = 0.0; // 存储最大时间
pthread_mutex_t max_time_mutex = PTHREAD_MUTEX_INITIALIZER; // 保护最大时间的互斥锁

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

void *matrix_mul(void* rank) {
    long my_rank = (long) rank;
    long start_row = my_rank * block_rows;
    long end_row = (my_rank + 1) * block_rows;

    long start_col = my_rank * block_cols;
    long end_col = (my_rank + 1) * block_cols;

    struct timespec start_time, end_time;
    double elapsed_time;

    // Barrier
    pthread_mutex_lock(&mutex);
    counter++;
    if (counter == thread_cnt) {
        counter = 0;
        pthread_cond_broadcast(&cond_var);
    }
    else {
        while (pthread_cond_wait(&cond_var, &mutex) != 0);
    }
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (int i = start_row; i < end_row; ++i) {
        for (int j = start_col; j < end_col; ++j) {
            float sum = 0;
            for (int x = 0; x < N; ++x) {
                sum += *(A + i * N + x) * *(B + x * N + j);
            }
            *(C + i * N + j) = sum;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    elapsed_time = (end_time.tv_sec - start_time.tv_sec);
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    // 更新全局最大时间
    pthread_mutex_lock(&max_time_mutex); // 锁定互斥锁以安全更新
    if (elapsed_time > max_elapsed_time) {
        max_elapsed_time = elapsed_time; // 更新最大时间
    }
    pthread_mutex_unlock(&max_time_mutex); // 解锁互斥锁

    return NULL;
}

int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;
    
    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    A = (float *)malloc(sizeof(float) * N * N);
    B = (float *)malloc(sizeof(float) * N * N);
    C = (float *)malloc(sizeof(float) * N * N);

    initialize_matrix(A, N, 0., 10.);
    initialize_matrix(B, N, 0., 10.);

    thread_cnt = strtol(argv[1], NULL, 10);
    thread_handles = malloc(thread_cnt * sizeof(pthread_t));
    
    if (thread_cnt == 2) {
        block_rows = N / 2;
    }
    else if (thread_cnt == 4) {
        block_rows = N / 2;
        block_cols = N / 2;
    }
    else if (thread_cnt == 8) {
        block_rows = N / 4;
        block_cols = N/ 2;
    }
    else if (thread_cnt == 16) {
        block_rows = N / 4;
        block_cols = N / 4;
    }

    for (thread = 0; thread < thread_cnt; thread ++) {
        pthread_create(&thread_handles[thread], NULL, matrix_mul, (void *) thread);
    }

    for (thread = 0; thread < thread_cnt; thread ++) {
        pthread_join(thread_handles[thread], NULL);
    }

    // printf("Matrix A:\n");
    // for (int i = 0; i < N; i ++) {
    //     for (int j = 0; j < N; j ++) {
    //         printf("%f ", A[i * N + j]);
    //     }
    //     printf("\n");
    // }
    // printf("Matrix B:\n");
    // for (int i = 0; i < N; i ++) {
    //     for (int j = 0; j < N; j ++) {
    //         printf("%f ", B[i * N + j]);
    //     }
    //     printf("\n");
    // }
    // printf("Matrix C:\n");
    // for (int i = 0; i < N; i ++) {
    //     for (int j = 0; j < N; j ++) {
    //         printf("%f ", C[i * N + j]);
    //     }
    //     printf("\n");
    // }
    
    printf("Max elapsed time among all threads: %.6f seconds.\n", max_elapsed_time);

    // 释放资源
    free(thread_handles);
    free(A);
    free(B);
    free(C);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&max_time_mutex);

    return 0;
}