#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

int num_thread;
const int N = 128;
float *A, *B, *C;

extern void parallel_for(int start, int end, int step, void (*func)(int, void*), void* arg, int num_threads);

struct for_index {
	int start; 
	int end;
	int increment;
};

// function impl
void *functor(void *args) {
    struct for_index *idx = (struct for_index *)args;
    int first = idx->start;
    int last = idx->end;
    int increment = idx->increment;
    for (int i = first; i <= last; i += increment) {
        for (int j = 0; j < N; j++) {
            float temp = 0;
            for (int z = 0; z < N; z++) {
                temp += A[i * N + z] * B[z * N + j];
            }
            C[i * N + j] = temp;
        }
    }
}

void initialize_matrix(float *matrix, int size, float low, float high)
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            // 生成[0, 1)之间的随机浮点数
            float randomValue = (float)rand() / (float)RAND_MAX;

            // 调整随机数的范围到[low, high)
            *(matrix + i * size + j) = low + randomValue * (high - low);
        }
    }
}

int main(int argc, char *argv[])
{
    A = (float *)malloc(sizeof(float) * N * N);
    B = (float *)malloc(sizeof(float) * N * N);
    C = (float *)malloc(sizeof(float) * N * N);
    initialize_matrix(A, N, 0., 10.);
    initialize_matrix(B, N, 0., 10.);
    float times = 0;
    struct timeval start;
    struct timeval end;

    num_thread = strtol(argv[1], NULL, 10);

    gettimeofday(&start, NULL);
    // # pragma omp parallel num_threads(num_thread)
    // omp_matrix_mul();
    parallel_for(0, N, 1, functor, NULL, num_thread);
    gettimeofday(&end, NULL);

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
    times = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Cost time: %f\n", times / 1000000);

    free(A);
    free(B);
    free(C);
    return 0;
}