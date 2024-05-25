#include <stdio.h>

__global__ void hello_world(void)
{
    printf("Hello World from Thread(%d, %d) in Block %d!\n", threadIdx.x, threadIdx.y, blockIdx.x);
}

int main(int argc, char **argv)
{
    const int N = 15, M = 5, K = 5;

    dim3 block(M, K);
    dim3 grid(N);

    printf("Hello World from the host!\n");
    hello_world<<<grid, block>>>();
    cudaDeviceReset(); // if no this line ,it can not output hello world from gpu
    return 0;
}
