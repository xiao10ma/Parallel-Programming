
# 并行程序设计与算法实验0

| 实验  | 串行矩阵算法           | 专业     | 计算机科学与技术 |
| ----- | ---------------------- | -------- | ---------------- |
| 学号  | 21311525               | 姓名     | 马梓培           |
| Email | mazp@mail2.sysu.edu.cn | 完成日期 | 2024/03/23       |

## 1. 实验目的

根据数学定义用C/C++语言实现一个串行矩阵乘法，并通过对比实验分析其性能。
通用矩阵乘法: $C=A \cdot B$, 其中 $A$ 为 $m \times n$ 的矩阵, $B$ 为 $n \times k$ 的矩阵,则其乘积 $C$ 为 $m \times k$ 的矩阵, $C$ 中第 $i$ 行第 $j$ 列元素可由矩阵 $A$ 中第 $i$ 行向量与矩阵 $B$ 中第 $j$ 列向量的内积给出, 即:

$$
C_{\mathrm{i}, \mathrm{j}}=\sum_{p=1}^n A_{i, p} B_{p, j}
$$

**输入：** $m, n, k$ 三个整数, 每个整数的取值范围均为 $[512,2048]$
**问题描述：**随机生成 $m \times n$ 的矩阵 $A$ 及 $n \times k$ 的矩阵 $B$, 并对这两个矩阵进行矩阵乘法运算, 得到矩阵 $C$.
**输出：** $A, B, C$ 三个矩阵, 及矩阵计算所消耗的时间 $t$ 。
**要求：** 实现多个版本的串行矩阵乘法（可考虑多种语言/编译选项/实现方式/算法/库），填写表格，并对比分析不同因素对最终性能的影响。

## 2. 环境配置

### 2.1 安装Ubuntu
1. 安装 VirtualBox: https://www.virtualbox.org/wiki/Downloads
2. 下载 Ubuntu 18.04 镜像文件: http://releases.ubuntu.com/18.04/
3. 在 VirtualBox 中创建虚拟机实例, 并调整其资源, 如 CPU 核、内存、硬盘容量等。
4. 在虚拟机实例中的虚拟光驱中加载 Ubuntu 镜像文件(iso 文件),并安装 Ubuntu 操作系统。

### 2.2 OpenMPI 命令行安装：
```bash
sudo apt-get update
sudo apt-get install libopenmpi-dev -y
sudo apt-get install vim -y
```
### 2.3 Intel oneAPI Math Kernel Library(MKL)命令行安装：
```bash
wget https://registrationcenter-download.intel.com/akdlm/IRC_NAS/86d6a4c1-c998-4c6b-9fff-ca004e9f7455/l_onemkl_p_2024.0.0.49673_offline.sh

sudo sh ./l_onemkl_p_2024.0.0.49673_offline.sh
```
修改环境变量：
```bash
source /opt/intel/oneapi/setvars.sh
```
可以输入env | grep SETVARS_COMPLETED来检查一下，输出结果是SETVARS_COMPLETED=1，就证明配置成功了！
```bash
env | grep SETVARS_COMPLETED
```
## 3. 串行矩阵算法实现

### 3.1 Python
通用矩阵乘法实现：
```python
for i in range(m):
    for j in range(k):
        for p in range(n):
            C[i, j] += A[i, p] * B[p, j]
```
记录运行时间和GFLOPS：
```python
# 计算总的浮点运算次数（这里只计算乘法）
total_flops = m * n * k
# 计算GFLOPS
gflops = total_flops / (elapsed_time * 1e9)
```
### 3.2 Naive
c++ naive 版本就是3个for循环实现通用矩阵乘法
```cpp
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
```
### 3.3 Change Order
在简单的代码实现中，由于矩阵以行主序方式存储，对于计算C矩阵中的元素值，采取的是先列后行的顺序。这样虽然能保证对A矩阵元素的连续访问，但对于B和C矩阵而言，访问模式就显得不那么高效了。特别是在最内层的循环中，对于k维度，访问B[j+k*m]时会产生较大的跨步访问。为了改善这种情况，可以考虑交换i和j维度的循环顺序，以利用数据的空间局部性，提高数据复用率。
```cpp
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
```
### 3.4 编译优化
在针对简单版本的代码，可以首先采取一种直接的方法，即应用一系列编译优化选项，如`-O3 -fomit-frame-pointer -march=armv8-a -ffast-math`，来促使编译器最大限度地实施自动向量化，以提高代码效率。
终端命令：
```bash
g++ matrix_mul.cpp -O3 -fomit-frame-pointer -march=native -ffast-math -o matrix_mul.out
```
编译选项解释：
- `-O3`：这是一个优化级别设置。它告诉编译器使用尽可能多的优化技术来提高程序的执行速度。它比-O2级别做得更多，可能会让编译时间变长，但是目的是为了让最终的程序运行得更快。

- `-fomit-frame-pointer`：这个选项会让编译器尝试不使用帧指针(frame pointer)，这样可以节省一些寄存器资源，可能会让程序运行得更快一些，特别是在寄存器非常宝贵的情况下。

- `-march=armv8-a`：这个选项指定了目标CPU的架构类型，在这个例子中是ARMv8-A架构。这意味着编译器会生成专门为这种类型的CPU优化过的代码，利用该架构的特定功能来提高性能。

- `-ffast-math`：这个选项放宽了一些数学计算的精度和标准规则，让编译器可以采取一些额外的数学相关优化措施来加速计算过程，但可能会牺牲一定的精确度。

### 3.5 循环展开
