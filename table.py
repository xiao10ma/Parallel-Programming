import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.005921, 0.048979, 0.391363, 3.582811, 45.888383],
    [0.002949, 0.025063, 0.197304, 1.892759, 21.289783],
    [0.001551, 0.015384, 0.101385, 1.070382, 11.637980],
    [0.000946, 0.006214, 0.062479, 0.643595, 6.418614],
    [0.000482, 0.003992, 0.085388, 0.661593, 6.849250]
])

# 进程数
process_counts = np.array([1, 2, 4, 8, 16])

# 矩阵规模
matrix_sizes = ['256', '512', '1024', '2048', '4096']

# 遍历每一列来绘制曲线图
for i in range(table.shape[1]):
    plt.plot(process_counts, table[:, i], label=f'Matrix {matrix_sizes[i]}', marker='o')  # 添加了标记以便更好地区分数据点

plt.title("Performance by Matrix Size and Process Count")  # 设置图表标题
plt.xlabel("Process Count")  # 设置x轴标签为进程数
plt.ylabel("Time (s)")  # 设置y轴标签为时间（秒）
plt.xticks(process_counts)  # 设置x轴刻度以匹配进程数
plt.yscale('log')  # 设置y轴为对数尺度
plt.legend()  # 显示图例，每条曲线对应一个矩阵规模
plt.grid(True, which="both", ls="--")  # 显示网格，包括主要和次要刻度线，以便更容易读取数值
plt.show()  # 显示图表
