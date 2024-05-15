# **并行程序设计与算法实验 8**

| 实验  | 最短路径并行实现       | 专业     | 计算机科学与技术 |
| ----- | ---------------------- | -------- | ---------------- |
| 学号  | 21311525               | 姓名     | 马梓培           |
| Email | mazp@mail2.sysu.edu.cn | 完成日期 | 2024/05/15       |

## 1. 实验要求

> **并行多源最短路径搜索**
>
> - 使用OpenMP/Pthreads/MPI中的任意一种
>
> - 可选用任意最短路径算法
>
> - 设置线程数量（1-16）
>
> - 根据运行时间，分析程序并行性能
>
>   •算法、并行框架、并行方式、数据（节点数量，平均度数）

## 2. 实验原理

借鉴自[oi-wiki](https://oi-wiki.org/graph/shortest-path/#floyd-算法)

### 2.1 Floyd 算法

是用来求任意两个结点之间的最短路的。

复杂度比较高，但是常数小，容易实现（只有三个 `for`）。

适用于任何图，不管有向无向，边权正负，但是最短路必须存在。（不能有个负环）

#### 2.1.1 实现

我们定义一个数组 $\mathrm{f}[\mathrm{k}][\mathrm{x}][\mathrm{y}]$, 表示只允许经过结点 1 到 $k$ （也就是说, 在子图 $V^{\prime}=1,2, \ldots, k$中的路径, 注意, $x$ 与 $y$ 不一定在这个子图中), 结点 $x$ 到结点 $y$ 的最短路长度。

很显然, $f[n][x][y]$ 就是结点 $x$ 到结点 $y$ 的最短路长度（因为 $V^{\prime}=1,2, \ldots, n$ 即为 $V$ 本身, 其表示的最短路径就是所求路径）。

接下来考虑如何求出 $f$ 数组的值。
$\mathrm{f}[0][\mathrm{x}][\mathrm{y}]: x$ 与 $y$ 的边权, 或者 0 , 或者 $+\infty(\mathrm{f}[0][\mathrm{x}][\mathrm{y}]$ 什么时候应该是 $+\infty ?$ 当 $x$ 与 $y$间有直接相连的边的时候, 为它们的边权; 当 $x=y$ 的时候为零, 因为到本身的距离为零; 当 $x$与 $y$ 没有直接相连的边的时候, 为 $+\infty)$ 。
$f[k][x][y]=\min (f[k-1][x][y], f[k-1][x][k]+f[k-1][k][y])(f[k-1][x][y]$, 为不经过 $k$点的最短路径, 而 $f[k-1][x][k]+f[k-1][k][y]$, 为经过了 $k$ 点的最短路)。

上面两行都显然是对的，所以说这个做法空间是 $O\left(N^3\right)$ ，我们需要依次增加问题规模（ $k$ 从 1 到 n) , 判断任意两点在当前问题规模下的最短路。

```c
for (k = 1; k <= n; k++) {
  for (x = 1; x <= n; x++) {
    for (y = 1; y <= n; y++) {
      f[k][x][y] = min(f[k - 1][x][y], f[k - 1][x][k] + f[k - 1][k][y]);
    }
  }
}
```

因为第一维对结果无影响，我们可以发现数组的第一维是可以省略的，于是可以直接改成`f[x][y] = min(f[x][y], f[x][k]+f[k][y])`.

```c
for (k = 1; k <= n; k++) {
  for (x = 1; x <= n; x++) {
    for (y = 1; y <= n; y++) {
      f[x][y] = min(f[x][y], f[x][k] + f[k][y]);
    }
  }
}
```

综上时间复杂度是 $O\left(N^3\right)$, 空间复杂度是 $O\left(N^2\right)$ 。

## 3. 实现代码

代码见附件或[github](https://github.com/xiao10ma/Parallel-Programming/tree/master/PP8).

观察到Floyd算法还是比较容易实现的，3个for循环即可。而openmp对于for循环又是非常友好，只需要通过一行**预处理指令**即可实现for循环的并行。

### 3.1 为什么不能并行`k`循环

但是，需要注意的是，我们不能简单的并行k。如果并行k，那么不同的线程将会同时计算不同的`k`值。然而，由于这些`k`值之间存在数据依赖关系，一个线程更新的结果可能会被另一个线程使用，而此时更新尚未完成，这将导致竞争条件和错误的计算结果。

### 3.2 为什么可以并行化第二层`i`或第三层`j`循环

并行化`i`或`j`循环时，同一个`k`值下，不同的`i`或`j`之间是相互独立的。换句话说，更新`distance_matrix[i][j]`的值时，不依赖于其他`i`或`j`的更新结果。因此，`i`或`j`循环中的每个迭代可以独立并行执行，不会引发数据竞争。

因此，openmp并行floyd算法：

```c
for (k = 0; k < N; k++) {
    #pragma omp parallel for private(i, j) shared(distance_matrix, k)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            distance_matrix[i][j] = min(distance_matrix[i][j], distance_matrix[i][k] + distance_matrix[k][j]);
        }
    }
}
```

## 4. 实验结果

1. updated_mouse.csv

```bash
xiaoma@xiaoma-virtual-machine:~/Parallel-Programming/PP8$ ./floyd_omp.out 
Total time for sequential (in sec):3.08
Total time for thread 1 (in sec):2.95
Total time for thread 2 (in sec):1.52
Total time for thread 4 (in sec):0.84
Total time for thread 8 (in sec):0.78
Total time for thread 16 (in sec):0.86
```

可以看到，随着进程数的2倍增大，时间在减半。（由于我的电脑是8线程，因此thread8和thread16几乎一致）

<img src="https://files.oaiusercontent.com/file-ZbClmNOWUtsf1PyMGkpkmrJc?se=2024-05-15T02%3A16%3A45Z&sp=r&sv=2023-11-03&sr=b&rscc=max-age%3D299%2C%20immutable&rscd=attachment%3B%20filename%3D42d004f6-defc-48ad-a7ed-0183e4fe282d&sig=ZLPN5VLBCyAkKvadyqTSeR5IBWgJreaLGZeuBwAivZI%3D" style="zoom: 30%;" />

处理后的邻接矩阵见附件（mat_mouse.txt），下面是部分截图：

![](https://github.com/xiao10ma/Parallel-Programming/blob/master/PP8/截屏2024-05-15%2010.07.35.png?raw=true)

2. updated_flower.csv

```bash
xiaoma@xiaoma-virtual-machine:~/Parallel-Programming/PP8$ ./floyd_omp.out 
Total time for sequential (in sec):2.24
Total time for thread 1 (in sec):2.14
Total time for thread 2 (in sec):1.11
Total time for thread 4 (in sec):0.61
Total time for thread 8 (in sec):0.51
Total time for thread 16 (in sec):0.70
```

同样的，随着进程数的2倍增大，时间在减半。（由于我的电脑是8线程，因此thread8和thread16几乎一致）

<img src="https://files.oaiusercontent.com/file-jGjB89gryvm6D2AGPVzklxUb?se=2024-05-15T02%3A17%3A38Z&sp=r&sv=2023-11-03&sr=b&rscc=max-age%3D299%2C%20immutable&rscd=attachment%3B%20filename%3D39e951ac-8744-430d-8ff3-003a462aafaa&sig=cEUEZmP/OasRQ7Xco8ujvJHUKS8Vd4EQenx%2BK%2BOP8YI%3D" style="zoom:33%;" />

处理后的邻接矩阵见附件（mat_flower.txt），下面是部分截图：

![](https://github.com/xiao10ma/Parallel-Programming/blob/master/PP8/截屏2024-05-15%2010.08.51.png?raw=true)

## 实验感想

在本次实验中，我使用OpenMP并行框架实现了Floyd算法来解决多源最短路径问题。通过将i或j循环并行化，显著提高了算法的执行效率。实验结果显示，随着线程数的增加，程序运行时间大幅减少，当线程数达到8时，性能提升最为明显。

通过本次实验，我深入理解了并行计算的原理和实际应用。在解决数据竞争和线程同步问题时，学到了如何优化代码结构以避免常见的并行编程问题。这次实验不仅提升了我的编程技能，也为未来处理大规模数据提供了有益的经验。
