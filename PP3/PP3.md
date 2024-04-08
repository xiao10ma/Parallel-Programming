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

代码见附件或github