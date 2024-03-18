import numpy as np

def matrix_multiply(m, n, k, low=0, high=10):
    A = np.random.randint(low, high, size=(m, n))
    B = np.random.randint(low, high, size=(n, k))

    C = np.zeros((m, k), dtype=int)  
    
    # 执行矩阵乘法
    for i in range(m):
        for j in range(k):
            for p in range(n):
                C[i, j] += A[i, p] * B[p, j]
    
    return C

m, n, k = 512, 1024, 2048  # 示例大小
C = matrix_multiply(m, n, k)
print("Resulting matrix C:", C)
