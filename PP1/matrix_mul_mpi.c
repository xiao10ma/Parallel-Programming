#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(){
    int M = 2048, N = 2048, K = 2048;
    double  MPI_Start, MPI_End;
    int comm_sz, my_rank;
    MPI_Comm comm = MPI_COMM_WORLD;// MPI通信子
    MPI_Init(NULL, NULL);          // 初始化MPI
    MPI_Comm_size(comm, &comm_sz); // 获取通信子大小（进程数量）
    MPI_Comm_rank(comm, &my_rank); // 获取进程编号
    MPI_Status status;

    // 本进程开始的行，结束的行，平均前n-2进程计算的行的数量
    // 最后一个进程计算剩下的大于等于avg_rows小于2*avg_rows行
    int begin_Arow, end_Arow, avg_rows;
    if (comm_sz > 1) avg_rows = K / (comm_sz - 1);

    if(my_rank == 0) {
        for (i = 0; i < comm_sz - 1; i++) {
            // 判断是不是最后一个进程，是的话把剩的全发过去
            begin_Arow = avg_rows * i, end_Arow = i + 1 == comm_sz - 1 ? MAX(K, avg_rows * (i + 1)) : avg_rows * (i + 1);
            // start row 可以计算得出，而最后一个进程的 end row 可能有些不一样，所以额外发送。
            MPI_Send(&end_Arow, 1, MPI_INT, i + 1, 10, MPI_COMM_WORLD); 
            MPI_Send(&A[begin_Arow * N], (end_Arow - begin_Arow) * N, MPI_FLOAT, i + 1, 1, MPI_COMM_WORLD); // some rows of matrix A
            MPI_Send(B, N * K, MPI_FLOAT, i + 1, 2, MPI_COMM_WORLD); // whole matrix B
        }
        for (i = 0; i < comm_sz - 1; i++) {
            begin_Arow = avg_rows * i, end_Arow = i + 1 == comm_sz - 1 ? MAX(K, avg_rows * (i + 1)) : avg_rows * (i + 1);
            MPI_Recv(&C[begin_Arow * N], (end_Arow - begin_Arow) * K, MPI_FLOAT, i + 1, 3, MPI_COMM_WORLD, &status);
        }
    }   
    else {
        MPI_Recv(&end_Arow, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
        begin_Arow = avg_rows * (my_rank - 1); // end_Arow = MIN(K, avg_rows * (my_rank + 1));

        printf("rank%d:\nfrom %d to %d\n", my_rank, begin_Arow, end_Arow);

        localA = (float *)malloc(sizeof(float) * (end_Arow - begin_Arow) * N);
        localB = (float *)malloc(sizeof(float) * N * K);
        localC = (float *)malloc(sizeof(float) * (end_Arow - begin_Arow) * K);

        MPI_Recv(localA, (end_Arow - begin_Arow) * N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(localB, N * K, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);

        matrix_multiply(end_Arow - begin_Arow, N, K, localA, localB, localC);  // 计算

        MPI_Send(localC, (end_Arow - begin_Arow) * K, MPI_FLOAT, 0, 3, MPI_COMM_WORLD); // 发送结果

        free(localA);
        // free(localB);
        free(localC);
    }
    MPI_Finalize();
}