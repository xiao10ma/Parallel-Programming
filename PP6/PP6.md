# **并行程序设计与算法实验 6**

| 实验  | Pthreads并行应用    | 专业     | 计算机科学与技术 |
| ----- | ---------------------- | -------- | ---------------- |
| 学号  | 21311525               | 姓名     | 马梓培           |
| Email | mazp@mail2.sysu.edu.cn | 完成日期 | 2024/05/01      |

## 1. 实验要求

> - 使用自定义的parallel_for替代heated_plate_openmp中的并行构造
> - 该应用模拟规则网格上的热传导，每次循环中对邻域内热量平均
> - $w_{i,j}^{t+1} = \frac{1}{4}(w_{i-1,j-1}^t + w_{i-1, j+1}^t + w_{i+1,j-1}^t + w_{i+1,j+1}^t)$​
> - 测试不同线程、调度方式下的程序并行性能
> - 与原始heated_plate_openmp.c实现进行对比

## 2. 基于 Pthreads 的 parallel_for 函数替换omp parallel for

代码见附件或[github](https://github.com/xiao10ma/Parallel-Programming/tree/master/PP6).

### 2.1 parallel_for.h 头文件

我在头文件中引入一些库，例如C语言标准库，以及`<omp.h>`和`<pthread.h>`。

同时，定义了两个结构体，分别为`Arg`与`parg`，其中：

- `Arg` 结构体：包含用于某些任务的各种参数，例如线程的起始和结束位置、线程数、排名以及指向两个二维数组 `W` 和 `U` 的指针，还有一个 `double` 类型的 `target`。
- `parg` 结构体：封装了线程应该执行的范围（`start` 到 `end`），以及函数指针 `functor`，该函数指针指向每次迭代要执行的函数。`increment` 表示循环的步长。

最后，我定义了parallel_for函数与toDo()：

- `void *toDo(void *arg)`：这是一个线程函数，接受一个 `parg` 类型的指针。该函数遍历从 `start` 到 `end`，步长为 `increment` 的范围，并在每步调用 `functor` 指向的函数。
- `void *paraller_for(int start, int end, int increment, void *(*functor)(void *), void *arg, int num_threads);`：这个函数原型是用来创建并管理多线程的，功能类似于 OpenMP 的 `#pragma omp parallel for`。此函数的实现应负责分配 `start` 到 `end` 范围内的迭代到不同线程，并确保这些线程可以并行执行。

```cpp
#ifndef PARALLEL_FOR_H
#define PARALLEL_FOR_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <pthread.h>
typedef struct {
    int start;
    int end;
    int pos;
    int threadNums;
    int rank;
    double **W;
    double **U;
    double target;
} Arg;
typedef struct {
    int start;
    int end;
    int increment;
    void *(*functor)(void *);
    void *arg;
} parg;
void *toDo(void *arg) {
    parg myArg = *(parg *)arg;
    for (int i = myArg.start; i < myArg.end; i += myArg.increment) {
        (*myArg.functor)(myArg.arg);
    }
}
void *paraller_for(int start, int end, int increment, void *(*functor)(void *), void *arg, int num_threads);
#endif // PARALLEL_FOR_H
```



### 2.2 parallel_for 函数

parallel_for函数则在parallel_for.c中实现，它也正是我们实验中要实现的.so文件：

`paraller_for`函数主要功能是将一个迭代范围（由 `start` 到 `end`），按照指定的步长 (`increment`) 和线程数量 (`num_threads`) 分配到多个线程上执行。我利用头文件中 `Arg` 类型的数组来存储每个线程的参数。这些参数是从主调用函数传入的 `arg` 复制而来，并为每个线程设置了一个唯一的 `rank`。同时，我准备 `parg` 结构体数组，用于传递给每个线程必要的信息，包括每个线程的起始和结束迭代位置、步长、要执行的函数（`functor`）以及传递给 `functor` 的参数。

然后就是普普通通的pthread_create与pthread_join了，这里比较麻烦的就是它前期的准备了。

```cpp
void *paraller_for(int start, int end, int increment, void *(*functor)(void *), void *arg, int num_threads) {
    if ((end - start) / (increment * num_threads) < 1)
        num_threads = (end - start) / increment;
    pthread_t thread_handles[num_threads];
    Arg args[num_threads];
    for (int i = 0; i < num_threads; i++) {
        args[i] = *(Arg *)arg;
        args[i].rank = i;
    }
    parg tmp[num_threads];
    int local_size = (end - start) / num_threads;
    for (int i = 0; i < num_threads - 1; i++) {
        tmp[i].start = i * local_size;
        tmp[i].end = tmp[i].start + local_size;
        tmp[i].increment = increment;
        tmp[i].arg = (void *)&args[i];
        tmp[i].functor = functor;
        pthread_create(&thread_handles[i], NULL, toDo, (void *)&tmp[i]);
    }
    tmp[num_threads - 1].start = (num_threads - 1) * local_size;
    tmp[num_threads - 1].end = end;
    tmp[num_threads - 1].increment = increment;
    tmp[num_threads - 1].arg = (void *)&args[num_threads - 1];
    tmp[num_threads - 1].functor = functor;
    pthread_create(&thread_handles[num_threads - 1], NULL, toDo, (void *)&tmp[num_threads - 1]);
    for (int i = 0; i < num_threads; i++)
        pthread_join(thread_handles[i], NULL);
}
```

### 2.3 