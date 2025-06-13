# 初始化参数对主存储结构的大小以及初始化时间的影响
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# n = 2 ^ 20, l = 2 ^ 10
times = [2, 3, 4, 5, 6, 7, 8]


time = [18797, 18640.4, 18574.2, 18673.9, 18532.8, 18632.8, 18769.7]
cap = [94.737, 90.761, 87.998, 87.555, 86.95, 86.558, 86.264]

# 绘制图片,以x_indices为横坐标，cap和time为纵坐标, 纵坐标分别为容量和初始化时间
figsize = 7, 6
# 设置图片大小
fig, ax1 = plt.subplots(figsize=figsize)  # 增加画布尺寸

# 绘制时间数据（左轴）
ax1.plot(times, time, 'x--', color='g', label='Time')
ax1.set_ylim(18000, 20000)  # 扩展时间轴范围
ax1.set_xlabel("Max kick times", fontsize=14)
ax1.set_ylabel('Time (ms)', color='g', fontsize=14)
ax1.tick_params(axis='y', labelcolor='g')
ax1.set_xticks(times)
ax1.set_xticklabels(times, fontsize=12)
ax1.grid(True, alpha=0.3)  # 半透明网格

# 创建右轴
ax2 = ax1.twinx()
ax2.plot(times, cap, 'o-', color='blue', label='Size')
ax2.set_ylim(70, 100)  # 扩展容量轴范围
ax2.set_ylabel('Size (MB)', color='blue', fontsize=14)
ax2.tick_params(axis='y', labelcolor='blue')


# 合并图例
lines, labels = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()

# 合并图例（调整位置到右下角）
ax1.legend(lines + lines2, labels + labels2, 
          ncol=2, fontsize=12, 
          loc='upper left',  # 调整图例位置
          #bbox_to_anchor=(0.9, 0.15)
          )  # 精确定位

plt.tight_layout()  # 自动调整布局
plt.show()