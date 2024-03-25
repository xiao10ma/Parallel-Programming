#include <iostream>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

// 使用一维数组初始化矩阵并填充随机数
#include <random>
using namespace std;

// 使用一维数组初始化矩阵并填充随机浮点数
void initialize_matrix(float* matrix, int rows, int cols, float low, float high) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> distrib(low, high);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            *(matrix + i * cols + j) = distrib(gen);
        }
    }
}

// naive
void matrix_multiply(float* A, float* B, float* C, int m, int n, int k) {
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < k; ++j) {
            float sum = 0;
            for (int x = 0; x < n; ++x) {
                sum += *(A + i * n + x) * *(B + x * k + j);
            }
            *(C + i * k + j) = sum;
        }
    }
}

// change order
void matrixMultiply_change_order(float *A, float *B, float *C, int m, int n, int k) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            float a = A[i * n + j];
            for (int x = 0; x < k; x++) {
                C[i * k + x] += a * B[j * k + x];
            }
        }
    }
}

// change order + unrolled
void matrixMultiply_change_order_unrolled4(float *A, float *B, float *C, int m, int n, int k) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            float a = A[i * n + j];
            for (int x = 0; x < (k / 4) * 4; x += 4) { 
                // 主循环展开4次
                C[i * k + x] += a * B[j * k + x];
                C[i * k + x + 1] += a * B[j * k + x + 1];
                C[i * k + x + 2] += a * B[j * k + x + 2];
                C[i * k + x + 3] += a * B[j * k + x + 3];
            }
            // 处理剩余元素
            for (int x = (k / 4) * 4; x < k; x++) {
                C[i * k + x] += a * B[j * k + x];
            }
        }
    }
}

int main() {
    int m = 512, n = 512, k = 512;

    // 分配一维数组
    float* A = new float[m * n];
    float* B = new float[n * k];
    float* C = new float[m * k];

    initialize_matrix(A, m, n, 0., 10.);
    initialize_matrix(B, n, k, 0., 10.);

    auto start = high_resolution_clock::now();

    matrix_multiply(A, B, C, m, n, k);
    // matrixMultiply_change_order(A, B, C, m, n, k);
    //matrixMultiply_change_order_unrolled4(A, B, C, m, n, k);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    double seconds = duration.count() / 1e6;
    double gflops = (2.0 * m * n * k) / (seconds * 1e9);

    cout << "Matrix multiplication took " << seconds << " seconds." << endl;
    cout << "GFLOPS: " << gflops << endl;

    // 释放分配的内存
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}