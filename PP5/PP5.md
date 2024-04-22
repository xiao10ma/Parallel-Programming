# **并行程序设计与算法实验 5**

| 实验  | OpenMP实现矩阵乘法     | 专业     | 计算机科学与技术 |
| ----- | ---------------------- | -------- | ---------------- |
| 学号  | 21311525               | 姓名     | 马梓培           |
| Email | mazp@mail2.sysu.edu.cn | 完成日期 | 2024/04/22       |

## 1. 实验要求

1. OpenMP通用矩阵乘法
	- 使用OpenMP实现并行通用矩阵乘法
	- 设置线程数量（1-16）、矩阵规模（128-2048）、调度方式
		- 调度方式包括默认调度、静态调度、动态调度
	- 调度方式包括默认调度、静态调度、动态调度
2. 使用Pthreads构建并行for循环分解、分配、执行机制
	- 使用Pthreads构建并行for循环分解、分配、执行机制
	- 生成包含parallel_for函数的动态链接库（.so）文件
```cpp
parallel_for(int start, int end, int inc, 
             void *(*functor)(int,void*), void *arg, int num_threads)
```
start, end, inc分别为循环的开始、结束及索引自增量
- functor为函数指针，定义了每次循环所执行的内容
- arg为functor的参数指针，给出了functor执行所需的数据
- num_threads为期望产生的线程数量

## 2. OpenMP通用矩阵乘法

本实验代码见附件或[github](https://github.com/xiao10ma/Parallel-Programming/tree/master/PP5).

> OpenMP允许程序员只需要简单地申明一块代码应该并行执行，而由编译器和运行时系统来决定哪个线程具体执行哪个任务。

最基本的parallel指令可以以如下简单的形式来表示：

```cpp
# pragma omp parallel
```

而在本实验中，我们通过如下parallel指令来实现并行：

```cpp
# pragma omp parallel num_threads(num_thread)
```

这与基础的指令不同之处，在于多了个`num_thread`子句，从而可以允许程序员指定执行后代码块的线程数。

### 2.1 矩阵乘法

与之前几次实验类似，我们需要确定矩阵A需要运算的行，其基于线程的rank，可以通过`omp_get_thread_num()`函数来获得。

因此，矩阵的乘法如下所示：

```cpp
void omp_matrix_mul() {
    int rank = omp_get_thread_num();
    int first_row = rank * avg_rows;
    int last_row = (rank + 1) * avg_rows;
    for (int i = first_row; i < last_row; i ++) {
        for (int j = 0; j < N; j ++) {
            float temp = 0;
            for (int z = 0; z < N; z ++) {
                temp += A[i * N + z] * B[z * N + j];
            }
            C[i * N + j] = temp;
        }
    }
}
```

将矩阵乘法函数结合parallel指令，从而实现并行化：

```cpp
# pragma omp parallel num_threads(num_thread)
omp_matrix_mul();
```

### 2.2 程序计时

计时框架如下，所示，需要注意的是，可能usec不够减：

```cpp
#include <sys/time.h>
...
struct timeval start_time, end_time;

gettimeofday(&start_time, NULL);  
...
gettimeofday(&end_time, NULL);   

long seconds = end_time.tv_sec - start_time.tv_sec;
long useconds = end_time.tv_usec - start_time.tv_usec;

// 如果结束时间的微秒数小于开始时间的微秒数，需要调整
if (useconds < 0) {
    useconds += 1000000;  
    seconds -= 1;         
}

double total_seconds = seconds + useconds / 1000000.0;
printf("Time Elapsed = %.6f seconds\n", total_seconds);
```

## 3. 实验结果

N = 128:

