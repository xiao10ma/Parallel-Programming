#include <stdio.h>
#include <stdlib.h>
#include "cuda_tool.h"
#include <cudnn.h>

#define USE_CPU 1
const int N = 4096;  // matrix size
const int channel = 3;  // 通道数
const int K = 3;    // kernel size
const int convOutSize = N - K + 1;

__device__ float kernel_dev[K * K * channel];
// __constant__ float kernel_dev[K * K * channel];

void CPU_Conv(float *input, float *output, float *kernel) {
    for (int row = 0; row < convOutSize; row ++) {
        for (int col = 0; col < convOutSize; col ++) {
            float sum = 0.f;
            for (int i = 0; i < K; i ++) {
                for (int j = 0; j < K; j ++) {
                    int curCol = col + j;
                    int curRow = row + i;
                    // 3通道
                    sum += kernel[i * K + j] * input[curRow * N + curCol] + kernel[i * K + j + K * K] * input[curRow * N + curCol + N * N] + kernel[i * K + j + 2 * K * K] * input[curRow * N + curCol + 2 * N * N];
                }
            }
            output[row * convOutSize + col] = sum;
        }
    }
}

__global__ void Conv2DKernel(float *input, float *output) {
    int col = threadIdx.x + blockDim.x * blockIdx.x;
    int row = threadIdx.y + blockDim.y * blockIdx.y;

    if (col >= convOutSize || row >= convOutSize) {
        return;
    }
    int curCol = 0, curRow = 0;
    float sum = 0.f;
    for (int i = 0; i < K; i ++) {
        for (int j = 0; j < K; j ++) {
            curCol = col + j;
            curRow = row + i;
            // 3 通道
            sum += kernel_dev[i * K + j] * input[curRow * N + curCol] + kernel_dev[i * K + j + K * K] * input[curRow * N + curCol + N * N] + kernel_dev[i * K + j + 2 * K * K] * input[curRow * N + curCol + 2 * N * N];
        }
    }
    output[row * convOutSize + col] = sum;
}

__device__ void matrix_mul(float *col_vec, float *res_dev) {
    const int nRow = blockIdx.y * blockDim.y + threadIdx.y;
    const int nCol = blockIdx.x * blockDim.x + threadIdx.x;
    if (nRow >= convOutSize || nCol >= convOutSize) return;

    const int col_vec_row = convOutSize * convOutSize;
    float c_sum = 0.f;
    for (int i = 0; i < K * K; i ++) {
        c_sum += kernel_dev[i] * col_vec[i * col_vec_row + nRow * convOutSize + nCol];
    }

    res_dev[nRow * convOutSize + nCol] = c_sum;
}

__global__ void im2col(float *input, float *col_vec, float *res_dev) {
    int col = threadIdx.x + blockDim.x * blockIdx.x;
    int row = threadIdx.y + blockDim.y * blockIdx.y;

    if (col >= convOutSize || row >= convOutSize) {
        return;
    }
    int curCol = 0, curRow = 0;

    const int col_vec_row = convOutSize * convOutSize;
    // const int col_vec_col = K * K * 3;

    for (int i = 0; i < K; i ++) {
        for (int j = 0; j < K; j ++) {
            curRow = row + i;
            curCol = col + j;
            // 1. input
            // 2. col_vec: i * K + j 行
            //             row * convOutSize + col 列 
            col_vec[(i * K + j) * col_vec_row + row * convOutSize + col] = input[curRow * N + curCol];
            col_vec[(i * K + j) * col_vec_row + row * convOutSize + col + col_vec_row * K * K] = input[curRow * N + curCol + N * N];
            col_vec[(i * K + j) * col_vec_row + row * convOutSize + col + 2 * col_vec_row * K * K] = input[curRow * N + curCol + N * N * 2];
        }
    }
    __syncthreads();
    matrix_mul(col_vec, res_dev);
}

int main() {
    int imgSize = N * N * channel;

    float *matrix_host, *conved_m_host, *kernel_host, *cpu_conved_res;
    float *matrix_dev, *conved_m_dev, *col_vec_dev;

    int matrix_bytes = imgSize * sizeof(float);
    int conved_matrix_bytes = convOutSize * convOutSize * sizeof(float);
    int kernel_bytes = K * K * channel * sizeof(float);

    // 分配并初始化 host(CPU) 内存
    cudaMallocHost((void **) &matrix_host, matrix_bytes);
    cudaMallocHost((void **) &conved_m_host, conved_matrix_bytes);
    cudaMallocHost((void **) &cpu_conved_res, conved_matrix_bytes);
    cudaMallocHost((void **) &kernel_host, kernel_bytes);

    initialData(matrix_host, imgSize);
    initialData(kernel_host, K * K * channel);

    if (USE_CPU) {
        // cpu compute
        double iStart_cpu=cpuSecond();
        CPU_Conv(matrix_host, cpu_conved_res, kernel_host);
        double iElaps_cpu=cpuSecond()-iStart_cpu;
        printf("CPU \t\t\tExecution Time elapsed %f sec\n", iElaps_cpu);
    }

    // 分配device(GPU)上的内存
    cudaMalloc((void **) &matrix_dev, matrix_bytes);
    cudaMalloc((void **) &conved_m_dev, conved_matrix_bytes);
    // 分配拼接列向量矩阵的内存
    cudaMalloc((void **) &col_vec_dev, K * K * 3 * convOutSize * convOutSize);

    cudaMemcpy(matrix_dev, matrix_host, matrix_bytes, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(kernel_dev, kernel_host, kernel_bytes);

    dim3 block(K, K);
    dim3 grid((N + K - 1) / K, (N + K - 1) / K);

    double iStart_cuda = cpuSecond();
    Conv2DKernel<<<grid, block>>>(matrix_dev, conved_m_dev);
    double iElaps_cuda=cpuSecond()-iStart_cuda;
    printf("CUDA \t\t\tExecution Time elapsed %f sec\n", iElaps_cuda);
    cudaMemcpy(conved_m_host, conved_m_dev, conved_matrix_bytes, cudaMemcpyDeviceToHost);
    checkResult(cpu_conved_res, conved_m_host, convOutSize);

    double iStart_cuda_im2col = cpuSecond();
    im2col<<<grid, block>>>(matrix_dev, col_vec_dev, conved_m_dev);
    double iElaps_cuda_im2col=cpuSecond()-iStart_cuda_im2col;
    printf("CUDA_im2col: \t\tExecution Time elapsed %f sec\n", iElaps_cuda_im2col);

    cudaMemcpy(conved_m_host, conved_m_dev, conved_matrix_bytes, cudaMemcpyDeviceToHost);
    checkResult(cpu_conved_res, conved_m_host, convOutSize);

    // cudnn
    cudnnHandle_t cudnn;
    cudnnCreate(&cudnn);

    cudnnTensorDescriptor_t in_desc;
    cudnnCreateTensorDescriptor(&in_desc);
    cudnnSetTensor4dDescriptor(in_desc, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT, 1, channel, N, N);

    cudnnFilterDescriptor_t filt_desc;
    cudnnCreateFilterDescriptor(&filt_desc);
    cudnnSetFilter4dDescriptor(filt_desc, CUDNN_DATA_FLOAT, CUDNN_TENSOR_NCHW, 1, channel, K, K);

    cudnnConvolutionDescriptor_t conv_desc;
    cudnnCreateConvolutionDescriptor(&conv_desc);
    cudnnSetConvolution2dDescriptor(conv_desc, 1, 1, 1, 1, 1, 1, CUDNN_CONVOLUTION, CUDNN_DATA_FLOAT);

    int out_n, out_c, out_h, out_w;
    cudnnGetConvolution2dForwardOutputDim(conv_desc, in_desc, filt_desc, &out_n, &out_c, &out_h, &out_w);

    float* out_data;
    cudaMalloc(&out_data, out_n * out_c * out_h * out_w * sizeof(float));

    cudnnTensorDescriptor_t out_desc;
    cudnnCreateTensorDescriptor(&out_desc);
    cudnnSetTensor4dDescriptor(out_desc, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT, out_n, out_c, out_h, out_w);

    size_t ws_size;
    cudnnGetConvolutionForwardWorkspaceSize(cudnn, in_desc, filt_desc, conv_desc, out_desc, CUDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_GEMM, &ws_size);

    float* ws_data;
    cudaMalloc(&ws_data, ws_size);

    float alpha = 1.0f;
    float beta = 0.0f;

    double iStart_cudnn = cpuSecond();
    cudnnConvolutionForward(
        cudnn,
        &alpha,
        in_desc, matrix_dev,
        filt_desc, kernel_host, // 修改这里，将kernel_host传给cuDNN
        conv_desc,
        CUDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_GEMM,
        ws_data, ws_size,
        &beta,
        out_desc, out_data
    );
    double iElaps_cudnn = cpuSecond() - iStart_cudnn;
    printf("cuDNN \t\t\tExecution Time elapsed %f sec\n", iElaps_cudnn);

    float* host_out_data = new float[out_n * out_c * out_h * out_w];
    cudaMemcpy(host_out_data, out_data, out_n * out_c * out_h * out_w * sizeof(float), cudaMemcpyDeviceToHost);
    checkResult(cpu_conved_res, conved_m_host, convOutSize);

    cudaFree(ws_data);
    cudaFree(out_data);
    cudnnDestroyTensorDescriptor(out_desc);
    cudnnDestroyConvolutionDescriptor(conv_desc);
    cudnnDestroyFilterDescriptor(filt_desc);
    cudnnDestroyTensorDescriptor(in_desc);
    cudnnDestroy(cudnn);

    delete[] host_out_data;


    cudaFreeHost(matrix_host);
    cudaFreeHost(conved_m_host);
    cudaFreeHost(cpu_conved_res);
    cudaFreeHost(kernel_host);
    cudaFree(matrix_dev);
    cudaFree(conved_m_host);

    return 0;
}
