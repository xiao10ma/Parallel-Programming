import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.013769, 0.084312, 0.481108, 3.753396, 32.983432],
    [0.007233, 0.050017, 0.260021, 1.961918, 17.653807],
    [0.003861, 0.025685, 0.146547, 1.042281, 10.603551],
    [0.001570, 0.016018, 0.089492, 0.903420, 10.331040],
    [0.001096, 0.008387, 0.092121, 0.849642, 9.993411]
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
