#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

// Define the number of nodes in the graph
#define N 1000

// Define minimum function that will be used later on to calcualte minimum values between two numbers
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// Define matrix of size N * N to store distances between nodes
// Initialize all distances to zero
double distance_matrix[N][N] = {};

int main(int argc, char *argv[])
{
    // 并行初始化矩阵
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) distance_matrix[i][j] = 0;
            else distance_matrix[i][j] = INFINITY;
        }
    }

    FILE *file1 = fopen("updated_mouse.csv", "r");
    char buffer[32];    // 4 + 4 + 4
    // 读取并忽略第一行
    fgets(buffer, 32, file1);

    int a, b;
    float c;

    int max_index = -1, min_index = 0x3f3f3f3f;
    while (fgets(buffer, 32, file1)) {
        sscanf(buffer, "%d,%d,%f", &a, &b, &c);
        distance_matrix[a][b] = c;
        distance_matrix[b][a] = c;
        max_index = max(max_index, a); 
        max_index = max(max_index, b);
        min_index = min(min_index, a);
        min_index = min(min_index, b);
    }

    int nthreads;
    int src, dst, middle;

    // Define time variable to record start time for execution of program
    double start_time = omp_get_wtime();

    for (middle = 0; middle < N; middle++)
    {
        double *dm = distance_matrix[middle];
        for (src = 0; src < N; src++)
        {
            double *ds = distance_matrix[src];
            for (dst = 0; dst < N; dst++)
            {
                ds[dst] = min(ds[dst], ds[middle] + dm[dst]);
            }
        }
    }

    double time = omp_get_wtime() - start_time;
    printf("Total time for sequential (in sec):%.2f\n", time);

    for (nthreads = 1; nthreads <= 16; nthreads*=2)
    {
        // Define different number of threads
        omp_set_num_threads(nthreads);

        // Define iterator to iterate over distance matrix
        // Define time variable to record start time for execution of program
        double start_time = omp_get_wtime();

/* Taking a node as mediator
check if indirect distance between source and distance via mediator
is less than direct distance between them */
#pragma omp parallel shared(distance_matrix)
        for (middle = 0; middle < N; middle++)
        {
            double *dm = distance_matrix[middle];
#pragma omp parallel for private(src, dst) schedule(dynamic)
            for (src = 0; src < N; src++)
            {
                double *ds = distance_matrix[src];
                for (dst = 0; dst < N; dst++)
                {
                    ds[dst] = min(ds[dst], ds[middle] + dm[dst]);
                }
            }
        }

        double time = omp_get_wtime() - start_time;
        printf("Total time for thread %d (in sec):%.2f\n", nthreads, time);
    }

    FILE *file2 = fopen("mat.txt", "w");
    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            fprintf(file2, "%f ", distance_matrix[i][j]);
        }
        fprintf(file2, "\n");
    }


    fclose(file1); // 关闭文件
    fclose(file2); // 关闭文件


    return 0;
}