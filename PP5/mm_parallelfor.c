#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>

const int N = 2048;
int num_threads;

// 需要extern声明以便链接到动态库
extern void parallel_for(int start, int end, int inc, void *(*func)(int, void*), void *arg, int num_threads);

typedef struct {
    double *A, *B, *C;
    int n;
} MatrixData;

void *matrix_multiply_functor(int i, void *arg) {
    MatrixData *data = (MatrixData *)arg;
    int n = data->n;
    double *A = data->A;
    double *B = data->B;
    double *C = data->C;

    for (int j = 0; j < n; j++) {
        double sum = 0.0;
        for (int k = 0; k < n; k++) {
            sum += A[i * n + k] * B[k * n + j];
        }
        C[i * n + j] = sum;
    }
    return NULL;
}

void generate_matrix(double *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
}

int main(int argc, char **argv) {
    num_threads = atoi(argv[1]);

    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)malloc(N * N * sizeof(double));

    srand((unsigned int)time(NULL));
    generate_matrix(A, N);
    generate_matrix(B, N);

    double startTime = omp_get_wtime();

    MatrixData data = {A, B, C, N};
    parallel_for(0, N, 1, matrix_multiply_functor, &data, num_threads);

    double endTime = omp_get_wtime();

    printf("Total time: %f seconds\n", endTime - startTime);

    free(A);
    free(B);
    free(C);

    return 0;
}
