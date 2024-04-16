import numpy as np
import matplotlib.pyplot as plt

# 你提供的数据
table = np.array([[0.001771, 0.003424, 0.005634, 0.008436, 0.017006, 0.035475, 0.074379, 0.145537],
 [0.000794, 0.001792, 0.002740, 0.004492, 0.008234, 0.017141, 0.033816, 0.075687],
 [0.000377, 0.000795, 0.001475, 0.002281, 0.004939, 0.009986, 0.018382, 0.036094],
 [0.000599, 0.000985, 0.001373, 0.001943, 0.004415, 0.007589, 0.016177, 0.034524],
 [0.000664, 0.000816, 0.001206, 0.001930, 0.003793, 0.007616, 0.015637, 0.030697]]
)

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
