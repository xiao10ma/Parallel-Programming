#include <iostream>
#include <chrono>
#include <random>
#include "mkl.h"

using namespace std;
using namespace std::chrono;

// 使用一维数组初始化矩阵并填充随机数
#include <random>
using namespace std;

// 使用一维数组初始化矩阵并填充双精度随机浮点数
void initialize_matrix(double* matrix, int rows, int cols, double low, double high) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> distrib(low, high);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            *(matrix + i * cols + j) = distrib(gen);
        }
    }
}

int main() {
    int m = 1024, n = 1024, k = 1024;
    double alpha = 1.0, beta = 0.0;
    // 分配一维数组
    double *A, *B, *C;
    A = (double *)mkl_malloc( m*k*sizeof( double ), 64 );
    B = (double *)mkl_malloc( k*n*sizeof( double ), 64 );
    C = (double *)mkl_malloc( m*n*sizeof( double ), 64 );
    if (A == NULL || B == NULL || C == NULL) {
        printf( "\n ERROR: Can't allocate memory for matrices. Aborting... \n\n");
        mkl_free(A);
        mkl_free(B);
        mkl_free(C);
        return 1;
    }

    initialize_matrix(A, m, n, 0., 10.);
    initialize_matrix(B, n, k, 0., 10.);

    auto start = high_resolution_clock::now();

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
            m, n, k, alpha, A, k, B, n, beta, C, n);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    double seconds = duration.count() / 1e6;
    double gflops = (2.0 * m * n * k) / (seconds * 1e9);

    cout << "Matrix multiplication took " << seconds << " seconds." << endl;
    cout << "GFLOPS: " << gflops << endl;

    // 释放分配的内存
    mkl_free(A);
    mkl_free(B);
    mkl_free(C);

    return 0;
}
