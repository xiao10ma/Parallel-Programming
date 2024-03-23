import numpy as np
import time  

def matrix_multiply(m, n, k, low=0, high=10):
    A = np.random.randint(low, high, size=(m, n))
    B = np.random.randint(low, high, size=(n, k))

    C = np.zeros((m, k), dtype=int)  
    
    start_time = time.time()

    for i in range(m):
        for j in range(k):
            for p in range(n):
                C[i, j] += A[i, p] * B[p, j]

    end_time = time.time()
    elapsed_time = end_time - start_time

    # 计算总的浮点运算次数（这里只计算乘法）
    total_flops = m * n * k
    # 计算GFLOPS
    gflops = total_flops / (elapsed_time * 1e9)

    print(f"Matrix multiplication took {elapsed_time:.3f} seconds.")
    print(f"GFLOPS: {gflops:.3f}")

    return C

m, n, k = 512, 512, 512 # 示例大小
C = matrix_multiply(m, n, k)