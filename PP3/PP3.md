# 并行程序设计与算法实验3

| 实验  | MPI点对点通信矩阵乘法  | 专业     | 计算机科学与技术 |
| ----- | ---------------------- | -------- | ---------------- |
| 学号  | 21311525               | 姓名     | 马梓培           |
| Email | mazp@mail2.sysu.edu.cn | 完成日期 | 2024/04/10       |

## 1. 实验目的

> 实验1：矩阵乘法
>
> - 使用Pthreads多线程实现并行矩阵乘法
> - 设置线程数量（1-16）及矩阵规模（128-2048）
> - 分析程序并行性能
> - 选做：可分析不同数据及任务划分方式的影响
>
> 实验2：数组求和
>
> - 使用Pthreads创建多线程，实现并行数组求和
> - 设置线程数量（1-16）及数组规模（1M-128M）
> - 分析程序并行性能及扩展性
> - 选做：可分析不同聚合方式的影响

## 2. 配置安装Pthreads

**Linux 下的 GCC/G++**:

pthread 库通常是 Linux 系统的默认库，所以通常不需要单独安装。但如果需要，可以使用包管理器安装。

```bash
# 对于基于 Debian 的系统（如 Ubuntu）
sudo apt-get install libpthread-stubs0-dev
 
# 对于基于 Red Hat 的系统（如 Fedora, CentOS）
sudo yum install pthread-stubs
```

在编译时，需要链接 pthread 库：

```bash
gcc -o myprogram myprogram.c -lpthread
```

## 3. Pthreads实现矩阵乘法

代码见附件或[github](https://github.com/xiao10ma/Parallel-Programming/tree/master/PP3).

首先，先进行一些初始化：

```cpp
long thread;
pthread_t* thread_handles;

// 初始化互斥锁和条件变量
pthread_mutex_init(&mutex, NULL);
pthread_cond_init(&cond_var, NULL);

A = (float *)malloc(sizeof(float) * N * N);
B = (float *)malloc(sizeof(float) * N * N);
C = (float *)malloc(sizeof(float) * N * N);

initialize_matrix(A, N, 0., 10.);
initialize_matrix(B, N, 0., 10.);

thread_cnt = strtol(argv[1], NULL, 10);
thread_handles = malloc(thread_cnt * sizeof(pthread_t));
avg_rows = N / thread_cnt;
```



我通过将矩阵A按行分块，矩阵B共享来计算矩阵C。

我们知道Pthreads采用的是共享内存的方式来实现。上述的矩阵乘法，并不会有临界区的情况。每个fork的线程都各自处理各自的数据，没有同时访问一块内存的情况。因此，可以直接fork出`thread_cnt`个线程，每个线程执行各自对应的矩阵乘法。

### 3.1 pthread_create

```cpp
for (thread = 0; thread < thread_cnt; thread ++) {
    pthread_create(&thread_handles[thread], NULL, matrix_mul, (void *) thread);
}
```

结合`pthread_create()`定义解释：

```cpp
int pthread_create(
		pthread_t* 								thread_p									/* out */,
  	const pthread_attr_t*			attr_p										/* in  */,
  	void*											(*start_routine)(void*)		/* in  */,
  	void* 										arg_p											/* in  */);
```

首先，写一个for循环，从0遍历到thread_cnt-1，总共fork出thread_cnt个线程。pthread_create()第一个参数是一个指针，指向我们初始化已经分配好了的pthread_t对象，通过for循环的thread来索引。第二个参数不用，用NULL表示。接下来，`*start_routine`函数指针，我们传入matrix_mul，表示要并行的函数。而最后一个参数，为每一个线程赋予了唯一的int型参数rank，表示线程的编号。

### 3.2 pthread_joint

同样，既然有fork线程，我们就需要合并线程。

![](https://github.com/xiao10ma/Parallel-Programming/blob/master/PP3/WechatIMG224.jpg?raw=true)

```cpp
for (thread = 0; thread < thread_cnt; thread ++) {
    pthread_join(thread_handles[thread], NULL);
}
```

结合`pthread_join()`定义解释：

```cpp
int pthread_join(
		pthread_t				thread				/* in  */,
  	void**					ret_val_p			/* out */);
```

首先，写一个for循环，从0遍历到thread_cnt - 1，总共fork出thread_cnt个线程。pthread_create()第一个参数表明，我们要合并哪个线程。第二个参数表示接受的返回值，我们不需要返回值，写上NULL。

### 3.3 运行计时

首先，我们需要设置一个barrier让所有进程在同一起跑线，copy自书上：

```cpp
// Barrier
pthread_mutex_lock(&mutex);
counter++;
if (counter == thread_cnt) {
    counter = 0;
    pthread_cond_broadcast(&cond_var);
}
else {
    while (pthread_cond_wait(&cond_var, &mutex) != 0);
}
pthread_mutex_unlock(&mutex);
```

计时，start_time 和 end_time 夹住矩阵相乘：

```cpp
clock_gettime(CLOCK_MONOTONIC, &start_time);
for (int i = start_rows; i < end_rows; ++i) {
    for (int j = 0; j < N; ++j) {
        float sum = 0;
        for (int x = 0; x < N; ++x) {
            sum += *(A + i * N + x) * *(B + x * N + j);
        }
        *(C + i * N + j) = sum;
    }
}
clock_gettime(CLOCK_MONOTONIC, &end_time);
```

在同步时间时，需要有互斥锁，因为访问的是同一个共享变量：

```cpp
// 更新全局最大时间
pthread_mutex_lock(&max_time_mutex); // 锁定互斥锁以安全更新
if (elapsed_time > max_elapsed_time) {
    max_elapsed_time = elapsed_time; // 更新最大时间
}
pthread_mutex_unlock(&max_time_mutex); // 解锁互斥锁
```

## 4. 通用矩阵乘法实验结果

N = 128

```bash
❯ ./mm.out 1
Max elapsed time among all threads: 0.013769 seconds.
❯ ./mm.out 2
Max elapsed time among all threads: 0.007233 seconds.
❯ ./mm.out 4
Max elapsed time among all threads: 0.003861 seconds.
❯ ./mm.out 8
Max elapsed time among all threads: 0.001570 seconds.
❯ ./mm.out 16
Max elapsed time among all threads: 0.001096 seconds.
```

N = 256

```bash
❯ ./mm.out 1
Max elapsed time among all threads: 0.084312 seconds.
❯ ./mm.out 2
Max elapsed time among all threads: 0.050017 seconds.
❯ ./mm.out 4
Max elapsed time among all threads: 0.025685 seconds.
❯ ./mm.out 8
Max elapsed time among all threads: 0.016018 seconds.
❯ ./mm.out 16
Max elapsed time among all threads: 0.008387 seconds.
```

N = 512

```bash
❯ ./mm.out 1
Max elapsed time among all threads: 0.481108 seconds.
❯ ./mm.out 2
Max elapsed time among all threads: 0.260021 seconds.
❯ ./mm.out 4
Max elapsed time among all threads: 0.146547 seconds.
❯ ./mm.out 8
Max elapsed time among all threads: 0.089492 seconds.
❯ ./mm.out 16
Max elapsed time among all threads: 0.092121 seconds.
```

N = 1024

```bash
❯ ./mm.out 1
Max elapsed time among all threads: 3.753396 seconds.
❯ ./mm.out 2
Max elapsed time among all threads: 1.961918 seconds.
❯ ./mm.out 4
Max elapsed time among all threads: 1.042281 seconds.
❯ ./mm.out 8
Max elapsed time among all threads: 0.903420 seconds.
❯ ./mm.out 16
Max elapsed time among all threads: 0.849642 seconds.
```

N = 2048

```bash
❯ ./mm.out 1
Max elapsed time among all threads: 32.983432 seconds.
❯ ./mm.out 2
Max elapsed time among all threads: 17.653807 seconds.
❯ ./mm.out 4
Max elapsed time among all threads: 10.603551 seconds.
❯ ./mm.out 8
Max elapsed time among all threads: 10.331040 seconds.
❯ ./mm.out 16
Max elapsed time among all threads: 9.993411 seconds.
```

绘制成表格：

| P\N  | 128      | 256      | 512      | 1024     | 2048      |
| ---- | -------- | -------- | -------- | -------- | --------- |
| 1    | 0.013769 | 0.084312 | 0.481108 | 3.753396 | 32.983432 |
| 2    | 0.007233 | 0.050017 | 0.260021 | 1.961918 | 17.653807 |
| 4    | 0.003861 | 0.025685 | 0.146547 | 1.042281 | 10.603551 |
| 8    | 0.001570 | 0.016018 | 0.089492 | 0.903420 | 10.331040 |
| 16   | 0.001096 | 0.008387 | 0.092121 | 0.849642 | 9.993411  |

可视化：

![](https://github.com/xiao10ma/Parallel-Programming/blob/master/PP3/Figure_1.png?raw=true)

## 5. 分块矩阵乘法实现

算法原理与上次MPI作业实现分块矩阵乘法基本类似。简短概括，我们可以将A矩阵拆成a个子矩阵，对应于(block_row = N / a)行N列的一个矩阵；将B矩阵拆成b个子矩阵，对应于(block_col = N / b)列N行的一个矩阵，即：
$$
A B=\left[\begin{array}{c}
A_0 \\
A_1 \\
\vdots \\
A_{a-1}
\end{array}\right]\left[\begin{array}{llll}
B_0 & B_1 & \cdots & B_{b-1}
\end{array}\right]=\left[\begin{array}{cccc}
A_0 B_0 & A_0 B_1 & \cdots & A_0 B_{b-1} \\
A_1 B_0 & A_1 B_1 & \cdots & A_1 B_{b-1} \\
\vdots & & \ddots & A_0 B_{b-1} \\
A_{a-1} B_0 & A_{a-1} B_1 & \cdots & A_{a-1} B_{b-1}
\end{array}\right]
$$


实现方式：

将可用于计算的进程数（thread_cnt）分解为a*b，然后将矩阵A全体行划分为a个部分，矩阵B全体列划分为b个部分，从而将整个矩阵划分为size相同的（thread_cnt）个块。每个子进程负责计算最终结果的一块。由于通用矩阵乘法的实现中，整个矩阵B都参与了运算，因此考虑到cache的空间局部性原理，分块矩阵乘法应该是要比通用矩阵乘法快上一些的。

### 5.1 代码实现

首先，根据thread_cnt确认每个线程需要处理的矩阵A和矩阵B大小：

```cpp
if (thread_cnt == 2) {
    block_rows = N / 2;
}
else if (thread_cnt == 4) {
    block_rows = N / 2;
    block_cols = N / 2;
}
else if (thread_cnt == 8) {
    block_rows = N / 4;
    block_cols = N/ 2;
}
else if (thread_cnt == 16) {
    block_rows = N / 4;
    block_cols = N / 4;
}
```

根据分块的大小，得到每个thread需要处理的矩阵范围：

```cpp
long start_row = my_rank * block_rows;
long end_row = (my_rank + 1) * block_rows;

long start_col = my_rank * block_cols;
long end_col = (my_rank + 1) * block_cols;
```

根据已经确定的矩阵范围，执行乘法：

```cpp
for (int i = start_row; i < end_row; ++i) {
    for (int j = start_col; j < end_col; ++j) {
        float sum = 0;
        for (int x = 0; x < N; ++x) {
            sum += *(A + i * N + x) * *(B + x * N + j);
        }
        *(C + i * N + j) = sum;
    }
}
```

## 6. 分块矩阵乘法实验结果

N = 128

```bash
❯ ./mm_block.out 1
Max elapsed time among all threads: 0.014830 seconds.
❯ ./mm_block.out 2
Max elapsed time among all threads: 0.008628 seconds.
❯ ./mm_block.out 4
Max elapsed time among all threads: 0.005661 seconds.
❯ ./mm_block.out 8
Max elapsed time among all threads: 0.001791 seconds.
❯ ./mm_block.out 16
Max elapsed time among all threads: 0.001080 seconds.
```

N = 256

```bash
❯ ./mm_block.out 1
Max elapsed time among all threads: 0.088521 seconds.
❯ ./mm_block.out 2
Max elapsed time among all threads: 0.051135 seconds.
❯ ./mm_block.out 4
Max elapsed time among all threads: 0.031025 seconds.
❯ ./mm_block.out 8
Max elapsed time among all threads: 0.018194 seconds.
❯ ./mm_block.out 16
Max elapsed time among all threads: 0.008349 seconds.
```

N = 512

```bash
❯ ./mm_block.out 1
Max elapsed time among all threads: 0.486692 seconds.
❯ ./mm_block.out 2
Max elapsed time among all threads: 0.249893 seconds.
❯ ./mm_block.out 4
Max elapsed time among all threads: 0.133712 seconds.
❯ ./mm_block.out 8
Max elapsed time among all threads: 0.093087 seconds.
❯ ./mm_block.out 16
Max elapsed time among all threads: 0.083458 seconds.
```

N = 1024

```bash
❯ ./mm_block.out 1
Max elapsed time among all threads: 3.700232 seconds.
❯ ./mm_block.out 2
Max elapsed time among all threads: 1.947231 seconds.
❯ ./mm_block.out 4
Max elapsed time among all threads: 1.020938 seconds.
❯ ./mm_block.out 8
Max elapsed time among all threads: 0.864890 seconds.
❯ ./mm_block.out 16
Max elapsed time among all threads: 0.838746 seconds.
```

N = 2048

```bash
❯ ./mm_block.out 1
Max elapsed time among all threads: 32.724529 seconds.
❯ ./mm_block.out 2
Max elapsed time among all threads: 17.917027 seconds.
❯ ./mm_block.out 4
Max elapsed time among all threads: 10.302597 seconds.
❯ ./mm_block.out 8
Max elapsed time among all threads: 10.231490 seconds.
❯ ./mm_block.out 16
Max elapsed time among all threads: 9.894151 seconds.
```

绘制成表格：

| P\N  | 128      | 256      | 512      | 1024     | 2048      |
| ---- | -------- | -------- | -------- | -------- | --------- |
| 1    | 0.014830 | 0.088521 | 0.486692 | 3.700232 | 32.724529 |
| 2    | 0.008628 | 0.051135 | 0.249893 | 1.947231 | 17.917027 |
| 4    | 0.005661 | 0.031025 | 0.133712 | 1.020938 | 10.302597 |
| 8    | 0.001791 | 0.018194 | 0.093087 | 0.864890 | 10.231490 |
| 16   | 0.001080 | 0.008349 | 0.083458 | 0.838746 | 9.894151  |

可视化：

![](https://github.com/xiao10ma/Parallel-Programming/blob/master/PP3/Figure_2.png?raw=true)

## 7. Pthread实现数组求和

### 7.1 pthread_create和pthread_join

```cpp
for (thread = 0; thread < thread_cnt; thread ++) {
    pthread_create(&thread_handles[thread], NULL, array_sum, (void *) thread);
}

for (thread = 0; thread < thread_cnt; thread ++) {
    pthread_join(thread_handles[thread], NULL);
}
```

与矩阵乘法极为相似，不过多赘述。

### 7.2 array sum

```cpp
void *array_sum(void* rank) {
    long my_rank = (long) rank;
    long start_ind = my_rank * avg_len;
    long end_ind = (my_rank + 1) * avg_len;

    float local_sum = 0;
    for (int i = start_ind; i < end_ind; i ++) {
        local_sum += A[i];
    }

    pthread_mutex_lock(&mutex);
    sum += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}
```

首先，先将自己进程中的A[i]相加求和，得到局部和local_sum。然后降其加到共享变量中。然而，由于共享的缘故，我们必须互斥地访问改变量。因此，需要有一个互斥锁来保证互斥访问。

## 8. 实验结果

N = 1M

```bash
❯ ./as.out 1
Cost time: 0.000017
❯ ./as.out 2
Cost time: 0.000020
❯ ./as.out 4
Cost time: 0.000017
❯ ./as.out 8
Cost time: 0.000024
❯ ./as.out 16
Cost time: 0.000024
```

N = 2M

```bash
❯ ./as.out 1
Cost time: 0.000025
❯ ./as.out 2
Cost time: 0.000020
❯ ./as.out 4
Cost time: 0.000025
❯ ./as.out 8
Cost time: 0.000021
❯ ./as.out 16
Cost time: 0.000020
```

N = 4M

```bash
❯ ./as.out 1
Cost time: 0.000041
❯ ./as.out 2
Cost time: 0.000025
❯ ./as.out 4
Cost time: 0.000021
❯ ./as.out 8
Cost time: 0.000023
❯ ./as.out 16
Cost time: 0.000016
```

N = 8M

```bash
❯ ./as.out 1
Cost time: 0.000036
❯ ./as.out 2
Cost time: 0.000020
❯ ./as.out 4
Cost time: 0.000021
❯ ./as.out 8
Cost time: 0.000020
❯ ./as.out 16
Cost time: 0.000020
```

N = 16M

```bash
❯ ./as.out 1
Cost time: 0.000042
❯ ./as.out 2
Cost time: 0.000020
❯ ./as.out 4
Cost time: 0.000025
❯ ./as.out 8
Cost time: 0.000024
❯ ./as.out 16
Cost time: 0.000019
```

N = 32M

```bash
❯ ./as.out 1
Cost time: 0.000026
❯ ./as.out 2
Cost time: 0.000019
❯ ./as.out 4
Cost time: 0.000021
❯ ./as.out 8
Cost time: 0.000025
❯ ./as.out 16
Cost time: 0.000024
```

N = 64M

```bash
❯ ./as.out 1
Cost time: 0.000022
❯ ./as.out 2
Cost time: 0.000021
❯ ./as.out 4
Cost time: 0.000015
❯ ./as.out 8
Cost time: 0.000022
❯ ./as.out 16
Cost time: 0.000025
```

N = 128M

```bash
❯ ./as.out 1
Cost time: 0.000032
❯ ./as.out 2
Cost time: 0.000030
❯ ./as.out 4
Cost time: 0.000027
❯ ./as.out 8
Cost time: 0.000029
❯ ./as.out 16
Cost time: 0.000035
```

| P\N  | 1M   | 2M   | 4M   | 8M   | 16M  | 32M  | 64M  | 128M |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 1    | 17   | 25   | 41   | 36   | 42   | 26   | 22   | 32   |
| 2    | 20   | 20   | 25   | 20   | 20   | 19   | 21   | 30   |
| 4    | 17   | 25   | 21   | 21   | 25   | 21   | 15   | 27   |
| 8    | 24   | 21   | 23   | 20   | 24   | 25   | 22   | 29   |
| 16   | 24   | 20   | 16   | 20   | 19   | 24   | 25   | 35   |

