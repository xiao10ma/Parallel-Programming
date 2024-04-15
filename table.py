import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.014830, 0.088521, 0.486692, 3.700232, 32.724529],
    [0.008628, 0.051135, 0.249893, 1.947231, 17.917027],
    [0.005661, 0.031025, 0.133712, 1.020938, 10.302597],
    [0.001791, 0.018194, 0.093087, 0.864890, 10.231490],
    [0.001080, 0.008349, 0.083458, 0.838746, 9.894151]
])

# 进程数
process_counts = np.array([1, 2, 4, 8, 16])

# 矩阵规模
matrix_sizes = ['128', '256', '512', '1024', '2048']

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
