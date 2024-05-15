#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

// 图规模
#define N 1000

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// 邻接矩阵 存图
double distance_matrix[N][N] = {};
double original_distance_matrix[N][N] = {};

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
    char buffer[32];    
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

    memcpy(original_distance_matrix, distance_matrix, sizeof(distance_matrix));

    int nthreads;
    int src, dst, middle;
    int i, j, k;

    double start_time = omp_get_wtime();

    // Serial Floyd
    for (k = 0; k < N; k++) {
        for (i = 0; i < N; i ++) {
            for (j = 0; j < N; j ++) {
                distance_matrix[i][j] = min(distance_matrix[i][j], distance_matrix[i][k] + distance_matrix[k][j]);
            }
        }
    }

    double time = omp_get_wtime() - start_time;
    printf("Total time for sequential (in sec):%.2f\n", time);

    for (nthreads = 1; nthreads <= 16; nthreads*=2)
    {
        // 恢复初始矩阵
        memcpy(distance_matrix, original_distance_matrix, sizeof(distance_matrix));

        // Define different number of threads
        omp_set_num_threads(nthreads);

        // Define iterator to iterate over distance matrix
        // Define time variable to record start time for execution of program
        double start_time = omp_get_wtime();


        for (k = 0; k < N; k++) {
            // 内层for循环并行
            #pragma omp parallel for private(i, j) shared(distance_matrix, k)
            for (i = 0; i < N; i++) {
                for (j = 0; j < N; j++) {
                    distance_matrix[i][j] = min(distance_matrix[i][j], distance_matrix[i][k] + distance_matrix[k][j]);
                }
            }
        }
        

        double time = omp_get_wtime() - start_time;
        printf("Total time for thread %d (in sec):%.2f\n", nthreads, time);
    }

    FILE *file2 = fopen("mat_mouse.txt", "w");
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
