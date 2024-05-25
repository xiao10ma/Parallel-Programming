#include <cuda_runtime.h>
#include <stdio.h>
#include "cuda_tool.h"
#define BDIMX 8
#define BDIMY 8
#define IPAD 2
//cpu transform
void transformMatrix2D_CPU(float * in,float * out,int nx,int ny) {
  for(int j=0;j<ny;j++)
  {
    for(int i=0;i<nx;i++)
    {
      out[i*nx+j]=in[j*nx+i];
    }
  }
}

__global__ void transformNaiveRow(float * in,float * out,int nx,int ny)
{
    int ix=threadIdx.x+blockDim.x*blockIdx.x;
    int iy=threadIdx.y+blockDim.y*blockIdx.y;
    int idx_row=ix+iy*nx;
    int idx_col=ix*ny+iy;
    if (ix<nx && iy<ny)
    {
      out[idx_col]=in[idx_row];
    }
}


//----------------------shared memory---------------------------
__global__ void transformSmem(float * in,float* out,int nx,int ny) {
	__shared__ float tile[BDIMY][BDIMX];
	unsigned int ix,iy,transform_in_idx,transform_out_idx;
	ix=threadIdx.x+blockDim.x*blockIdx.x;       // 全局的，没转置的ix
    iy=threadIdx.y+blockDim.y*blockIdx.y;       // 全局的，没转置的iy
	transform_in_idx=iy*nx+ix;                  // 全局的 in 数组的下标

	unsigned int bidx,irow,icol;                
	bidx=threadIdx.y*blockDim.x+threadIdx.x;    // bidx表示block idx也就是在这个块中的线程的坐标的线性位置
	irow=bidx/blockDim.y;                       // block转置后，的irow
	icol=bidx%blockDim.y;                       // block转置后，的icol


	ix=blockIdx.y*blockDim.y+icol;
	iy=blockIdx.x*blockDim.x+irow;


	transform_out_idx=iy*ny+ix;

	if(ix<nx&& iy<ny)
	{
		tile[threadIdx.y][threadIdx.x]=in[transform_in_idx];
		__syncthreads();
		out[transform_out_idx]=tile[icol][irow];

	}

}

__global__ void transformSmemPad(float * in,float* out,int nx,int ny) {
	__shared__ float tile[BDIMY][BDIMX+IPAD];
	unsigned int ix,iy,transform_in_idx,transform_out_idx;
	ix=threadIdx.x+blockDim.x*blockIdx.x;
    iy=threadIdx.y+blockDim.y*blockIdx.y;
	transform_in_idx=iy*nx+ix;

	unsigned int bidx,irow,icol;
	bidx=threadIdx.y*blockDim.x+threadIdx.x;
	irow=bidx/blockDim.y;
	icol=bidx%blockDim.y;


	ix=blockIdx.y*blockDim.y+icol;
	iy=blockIdx.x*blockDim.x+irow;


	transform_out_idx=iy*ny+ix;

	if(ix<nx&& iy<ny)
	{
		tile[threadIdx.y][threadIdx.x]=in[transform_in_idx];
		__syncthreads();
		out[transform_out_idx]=tile[icol][irow];

	}

}

int main(int argc,char** argv)
{
    initDevice(0);
    int nx=1<<12;
    int ny=1<<12;
    int dimx=BDIMX;
    int dimy=BDIMY;
    int nxy=nx*ny;
    int nBytes=nxy*sizeof(float);
    int transform_kernel=0;
    if(argc==2)
        transform_kernel=atoi(argv[1]);
    if(argc>=4)
    {
        transform_kernel=atoi(argv[1]);
        dimx=atoi(argv[2]);
        dimy=atoi(argv[3]);
    }

    //Malloc
    float* A_host=(float*)malloc(nBytes);
    float* B_host_cpu=(float*)malloc(nBytes);
    float* B_host=(float*)malloc(nBytes);
    initialData(A_host,nxy);

    //cudaMalloc
    float *A_dev=NULL;
    float *B_dev=NULL;
    CHECK(cudaMalloc((void**)&A_dev,nBytes));
    CHECK(cudaMalloc((void**)&B_dev,nBytes));

    CHECK(cudaMemcpy(A_dev,A_host,nBytes,cudaMemcpyHostToDevice));
    CHECK(cudaMemset(B_dev,0,nBytes));

    // cpu compute
    double iStart=cpuSecond();
    transformMatrix2D_CPU(A_host,B_host_cpu,nx,ny);
    double iElaps=cpuSecond()-iStart;
    printf("CPU Execution Time elapsed %f sec\n",iElaps);

    // 2d block and 2d grid
    dim3 block(dimx,dimy);
    dim3 grid((nx-1)/block.x+1,(ny-1)/block.y+1);
    dim3 block_1(dimx,dimy);
    dim3 grid_1((nx-1)/(block_1.x*2)+1,(ny-1)/block_1.y+1);

    CHECK(cudaDeviceSynchronize());
    iStart=cpuSecond();
    switch(transform_kernel)
    {
    case 0:
            transformNaiveRow<<<grid,block>>>(A_dev,B_dev,nx,ny);
            printf("transformNaiveRow ");
            break;
    case 1:
            transformSmem<<<grid,block>>>(A_dev,B_dev,nx,ny);
            printf("transformSmem ");
            break;
    case 2:
            transformSmemPad<<<grid,block>>>(A_dev,B_dev,nx,ny);
            printf("transformSmemPad ");
            break;
    default:
        break;
    }
    CHECK(cudaDeviceSynchronize());
    iElaps=cpuSecond()-iStart;
    printf(" Time elapsed %f sec\n",iElaps);
    CHECK(cudaMemcpy(B_host,B_dev,nBytes,cudaMemcpyDeviceToHost));
    checkResult(B_host,B_host_cpu,nxy);

    cudaFree(A_dev);
    cudaFree(B_dev);
    free(A_host);
    free(B_host);
    free(B_host_cpu);
    cudaDeviceReset();
    return 0;
}