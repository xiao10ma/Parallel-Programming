import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([
    [0.005983, 0.048604, 0.398889, 3.642851, 47.677945],
    [0.003115, 0.024398, 0.200865, 1.848243, 22.641008],
    [0.001711, 0.012336, 0.100701, 0.944562, 11.833128],
    [0.001057, 0.012760, 0.057415, 0.547553, 6.701250],
    [0.001329, 0.012713, 0.055641, 0.548499, 6.779896]
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
