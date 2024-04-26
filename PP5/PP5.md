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

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.005874 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.003123 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.001781 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.003686 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.001638 seconds
```

N = 256:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.047781 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.024062 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.012173 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.016812 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.009599 seconds
```

N = 512:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.385720 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.196501 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.102242 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.063299 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.066805 seconds
```

N = 1024:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 3.464608 seconds
PP5 ) ./mm.out 2
Time Elapsed = 1.738003 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.874890 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.533318 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.521655 seconds
```

N = 2048:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 34.086398 seconds
PP5 ) ./mm.out 2
Time Elapsed = 17.859157 seconds
PP5 ) ./mm.out 4
Time Elapsed = 9.262058 seconds
PP5 ) ./mm.out 8
Time Elapsed = 4.804137 seconds
PP5 ) ./mm.out 16
Time Elapsed = 5.713462 seconds
```

表格：

| P\N  | 128      | 256      | 512      | 1024     | 2048      |
| ---- | -------- | -------- | -------- | -------- | --------- |
| 1    | 0.005874 | 0.047781 | 0.385720 | 3.464608 | 34.086398 |
| 2    | 0.003123 | 0.024062 | 0.196501 | 1.738003 | 17.859157 |
| 4    | 0.001781 | 0.012173 | 0.102242 | 0.874890 | 9.262058  |
| 8    | 0.003686 | 0.016812 | 0.063299 | 0.533318 | 4.804137  |
| 16   | 0.001638 | 0.009599 | 0.066805 | 0.521655 | 5.713462  |

可视化：



## 4. 不同调度方式实现

不同调度方式，如静态和动态调度，可以通过预处理指令来实现：

```bash
1. #pragma omp parallel for schedule(static, 1) num_threads(num_thread)
2. #pragma omp parallel for schedule (dynamic, 1) num_threads(num_thread)
```



### 4.1 静态调度

N = 128:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.005855 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.003015 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.001751 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.014824 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.001821 seconds
```

N = 256:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.048124 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.024265 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.012359 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.015369 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.008438 seconds
```

N = 512:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.404326 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.201948 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.100092 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.072592 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.064959 seconds
```

N = 1024:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 3.726951 seconds
PP5 ) ./mm.out 2
Time Elapsed = 1.821126 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.931003 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.620243 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.576337 seconds
```

N = 2048:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 43.360471 seconds
PP5 ) ./mm.out 2
Time Elapsed = 20.762046 seconds
PP5 ) ./mm.out 4
Time Elapsed = 11.449756 seconds
PP5 ) ./mm.out 8
Time Elapsed = 6.752237 seconds
PP5 ) ./mm.out 16
Time Elapsed = 6.712368 seconds
```

表格：

| P\N  | 128      | 256      | 512      | 1024     | 2048      |
| ---- | -------- | -------- | -------- | -------- | --------- |
| 1    | 0.005855 | 0.048124 | 0.404326 | 3.726951 | 43.360471 |
| 2    | 0.003015 | 0.024265 | 0.201948 | 1.821126 | 20.762046 |
| 4    | 0.001751 | 0.012359 | 0.100092 | 0.931003 | 11.449756 |
| 8    | 0.014824 | 0.015369 | 0.072592 | 0.620243 | 6.752237  |
| 16   | 0.001821 | 0.008438 | 0.064959 | 0.576337 | 6.712368  |

可视化：

### 4.2 动态调度
N = 128:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.005983 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.003115 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.001711 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.001057 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.001329 seconds
```

N = 256:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.048604 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.024398 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.012336 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.012760 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.012713 seconds
```

N = 512:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 0.398889 seconds
PP5 ) ./mm.out 2
Time Elapsed = 0.200865 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.100701 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.057415 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.055641 seconds
```

N = 1024:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 3.642851 seconds
PP5 ) ./mm.out 2
Time Elapsed = 1.848243 seconds
PP5 ) ./mm.out 4
Time Elapsed = 0.944562 seconds
PP5 ) ./mm.out 8
Time Elapsed = 0.547553 seconds
PP5 ) ./mm.out 16
Time Elapsed = 0.548499 seconds
```

N = 2048:

```bash
PP5 ) ./mm.out 1
Time Elapsed = 47.677945 seconds
PP5 ) ./mm.out 2
Time Elapsed = 22.641008 seconds
PP5 ) ./mm.out 4
Time Elapsed = 11.833128 seconds
PP5 ) ./mm.out 8
Time Elapsed = 6.701250 seconds
PP5 ) ./mm.out 16
Time Elapsed = 6.779896 seconds
```

表格：

| P\N  | 128      | 256      | 512      | 1024     | 2048      |
| ---- | -------- | -------- | -------- | -------- | --------- |
| 1    | 0.005983 | 0.048604 | 0.398889 | 3.642851 | 47.677945 |
| 2    | 0.003115 | 0.024398 | 0.200865 | 1.848243 | 22.641008 |
| 4    | 0.001711 | 0.012336 | 0.100701 | 0.944562 | 11.833128 |
| 8    | 0.001057 | 0.012760 | 0.057415 | 0.547553 | 6.701250  |
| 16   | 0.001329 | 0.012713 | 0.055641 | 0.548499 | 6.779896  |

可视化：

