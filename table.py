import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.002609, 0.003239, 0.005645, 0.008607, 0.017711, 0.034818, 0.070977, 0.143842],
    [0.001175, 0.001655, 0.002686, 0.004442, 0.008411, 0.017545, 0.038256, 0.067061],
    [0.000794, 0.001380, 0.001656, 0.002432, 0.005225, 0.009814, 0.020395, 0.039102],
    [0.000637, 0.000956, 0.001593, 0.002071, 0.004062, 0.008950, 0.016129, 0.030899],
    [0.000507, 0.000874, 0.001135, 0.001857, 0.004248, 0.007547, 0.015611, 0.029182]
])

# 进程数
process_counts = np.array([1, 2, 4, 8, 16])

# 矩阵规模
# matrix_sizes = ['128', '256', '512', '1024', '2048']
array_len = ['1M', '2M', '4M', '8M', '16M', '32M', '64M', '128M']

# 遍历每一列来绘制曲线图
for i in range(table.shape[1]):
    plt.plot(process_counts, table[:, i], label=f'Matrix {array_len[i]}', marker='o')  # 添加了标记以便更好地区分数据点

plt.title("Performance by Matrix Size and Process Count")  # 设置图表标题
plt.xlabel("Process Count")  # 设置x轴标签为进程数
plt.ylabel("Time (s)")  # 设置y轴标签为时间（秒）
plt.xticks(process_counts)  # 设置x轴刻度以匹配进程数
plt.yscale('log')  # 设置y轴为对数尺度
plt.legend()  # 显示图例，每条曲线对应一个矩阵规模
plt.grid(True, which="both", ls="--")  # 显示网格，包括主要和次要刻度线，以便更容易读取数值
plt.show()  # 显示图表
