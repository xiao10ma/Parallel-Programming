import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.005625, 0.046530, 0.371795, 3.403670, 43.593964],
    [0.002802, 0.023810, 0.187439, 1.798121, 20.225294],
    [0.001473, 0.014615, 0.096316, 1.016863, 11.056081],
    [0.000899, 0.005903, 0.059355, 0.611415, 6.097683],
    [0.000458, 0.003792, 0.081119, 0.628513, 6.506788]
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
