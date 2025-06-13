# 初始化参数对主存储结构的大小以及初始化时间的影响
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# n = 2 ^ 20, l = 2 ^ 10
init_size = [128, 256, 512, 1024, 2048, 4096]
x_indices = np.arange(1, 7)

cap = [206.824, 203.553, 203.036, 205.248, 210.744, 220.167]
time = [63140.7, 63225, 63678.9, 63171.5, 63218.3, 63321.1]

# 绘制图片,以x_indices为横坐标，cap和time为纵坐标, 纵坐标分别为容量和初始化时间
figsize = 7, 6
# 设置图片大小
fig, ax1 = plt.subplots(figsize=figsize)  # 增加画布尺寸

# 绘制时间数据（左轴）
ax1.plot(x_indices, time, 'x--', color='g', label='Time')
ax1.set_ylim(60000, 65000)  # 扩展时间轴范围
ax1.set_xlabel("Buckets per segment", fontsize=14)
ax1.set_ylabel('Time (ms)', color='g', fontsize=14)
ax1.tick_params(axis='y', labelcolor='g')
ax1.set_xticks(x_indices)
ax1.set_xticklabels(init_size, fontsize=12)
ax1.grid(True, alpha=0.3)  # 半透明网格

# 创建右轴
ax2 = ax1.twinx()
ax2.plot(x_indices, cap, 'o-', color='blue', label='Size')
ax2.set_ylim(200, 250)  # 扩展容量轴范围
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