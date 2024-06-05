#include <cuda_runtime.h>
#include <stdio.h>
#include "cuda_tool.h"

#define USE_CPU 1
const int N = 1024;
const int block_size = 16;

void mm_cpu(float *A_host, float *B_host, float *C_host) {
    for (int i = 0; i < N; i ++) {
        for (int k = 0; k < N; k ++) {
            for (int j = 0; j < N; j ++) {
                C_host[i * N + j] += A_host[i * N + k] * B_host[k * N + j];
            }
        }
    }
}

__global__ void mm_cuda(float *A_dev, float *B_dev, float *C_dev) {
    int nRow = blockIdx.y * blockDim.y + threadIdx.y;
    int nCol = blockIdx.x * blockDim.x + threadIdx.x;
    float c_sum = 0.f;

    for (int i = 0; i < N; i ++) {
        c_sum += A_dev[nRow * N + i] * B_dev[i * N + nCol];
    }
    C_dev[nRow * N + nCol] = c_sum;
}

__global__ void mm_shared_mem(float *A_dev, float *B_dev, float *C_dev) {
    int nRow = blockIdx.y * blockDim.y + threadIdx.y;
    int nCol = blockIdx.x * blockDim.x + threadIdx.x;

    float c_sum = 0.f;

    __shared__ float A_tile[block_size][block_size];
    __shared__ float B_tile[block_size][block_size];

    int nIter = (N + block_size - 1) / block_size;      // 将矩阵一个维度拆成 nIter 块
    for (int i = 0; i < nIter; i ++) {
        A_tile[threadIdx.y][threadIdx.x] = A_dev[nRow * N + i * block_size + threadIdx.x];
        B_tile[threadIdx.y][threadIdx.x] = B_dev[(i * block_size + threadIdx.y) * N + nCol];
        // 同步block中不同warp
        __syncthreads();

        for (int iter = 0; iter < block_size; iter ++) {
            c_sum += A_tile[threadIdx.y][iter] * B_tile[iter][threadIdx.x];
        }
        // 同步block中不同warp
        __syncthreads();
    }
    C_dev[nRow * N + nCol] = c_sum;
}

int main() 
{
    float *A_host, *B_host, *C_host, *C_dev2host, *C_dev2host_shared;
    float *A_dev, *B_dev, *C_dev, *C_dev_shared;
    int nxy = N * N;    // matrix size
    int nBytes = sizeof(float) * N * N;

    // 分配host(CPU)上的内存
    cudaMallocHost((void **) &A_host, nBytes);
    cudaMallocHost((void **) &B_host, nBytes);
    cudaMallocHost((void **) &C_host, nBytes);
    cudaMallocHost((void **) &C_dev2host, nBytes);
    cudaMallocHost((void **) &C_dev2host_shared, nBytes);

    initialData(A_host, nxy);
    initialData(B_host, nxy);

    if (USE_CPU) {
        // cpu compute
        double iStart_cpu=cpuSecond();
        mm_cpu(A_host, B_host, C_host);
        double iElaps_cpu=cpuSecond()-iStart_cpu;
        printf("CPU \t\t\tExecution Time elapsed %f sec\n", iElaps_cpu);
    }

    // 分配device(GPU)上的内存
    cudaMalloc((void **) &A_dev, nBytes);
    cudaMalloc((void **) &B_dev, nBytes);
    cudaMalloc((void **) &C_dev, nBytes);
    cudaMalloc((void **) &C_dev_shared, nBytes);

    cudaMemcpy(A_dev, A_host, nBytes, cudaMemcpyHostToDevice);
    cudaMemcpy(B_dev, B_host, nBytes, cudaMemcpyHostToDevice);

    dim3 block(block_size, block_size);
    dim3 grid((N + block_size - 1) / block_size, (N + block_size - 1) / block_size);

    double iStart_cuda=cpuSecond();
    mm_cuda<<<grid, block>>>(A_dev, B_dev, C_dev);
    cudaDeviceSynchronize();
    double iElaps_cuda=cpuSecond()-iStart_cuda;
    printf("CUDA \t\t\tExecution Time elapsed %f sec\n", iElaps_cuda);

    double iStart_shared=cpuSecond();
    mm_shared_mem<<<grid, block>>>(A_dev, B_dev, C_dev_shared);
    cudaDeviceSynchronize();
    double iElaps_shared=cpuSecond()-iStart_shared;
    printf("CUDA(shared mem) \tExecution Time elapsed %f sec\n", iElaps_shared);
    printf("---------------------------------------------\n");
    cudaDeviceSynchronize();
    cudaMemcpy(C_dev2host, C_dev, nBytes, cudaMemcpyDeviceToHost);
    cudaMemcpy(C_dev2host_shared, C_dev_shared, nBytes, cudaMemcpyDeviceToHost);
    printf("CUDA: \t\t\t");
    checkResult(C_host, C_dev2host, nxy);
    printf("CUDA(shared mem): \t");
    checkResult(C_host, C_dev2host_shared, nxy);

    cudaFreeHost(A_host);
    cudaFreeHost(B_host);
    cudaFreeHost(C_host);
    cudaFreeHost(C_dev2host);
    cudaFreeHost(C_dev2host_shared);
    cudaFree(A_dev);
    cudaFree(B_dev);
    cudaFree(C_dev);
    cudaFree(C_dev_shared);
}
