import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
# matplotlib.use("TkAgg")
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 指定默认字体为黑体
# plt.rcParams['axes.unicode_minus'] = False  # 解决保存图像是负号'-'显示为方块的问题

size = [0] * 5
time = [0] * 5
for i in range(7, 12):
    data = pd.read_csv('analyse/csv/setup_init_size_{}.csv'.format(i))
    s = data.iloc[:, 0]
    t = data.iloc[:, 1]
    size_mean = s.mean()
    time_mean = t.mean()
    size[i - 7] = size_mean
    time[i - 7] = time_mean

size = np.array(size) / 1024  # 转换为MB
x = np.arange(7, 12)
x_indices = [ pow(2, i) for i in x ]  # 生成x_indices列表，表示x中每个元素是否为2的幂次方


figsize = 7, 6
# 设置图片大小
fig, ax1 = plt.subplots(figsize=figsize)  # 增加画布尺寸
# 绘制时间数据（左轴）
ax1.plot(x, time, 'x--', color='g', label='Time')
ax1.set_ylim(0.8, 1.2)  # 扩展时间轴范围
ax1.set_xlabel("Initial capacity", fontsize=14)
ax1.set_ylabel('Time (ms)', color='g', fontsize=14)
ax1.tick_params(axis='y', labelcolor='g')
ax1.set_xticks(x, x_indices, fontsize=12)
ax1.grid(True, alpha=0.3)  # 半透明网格

# 创建右轴
ax2 = ax1.twinx()
ax2.plot(x, size, 'o-', color='blue', label='Size')
ax2.set_ylim(30, 480)  # 扩展容量轴范围
ax2.set_ylabel('Size (KB)', color='blue', fontsize=14)
ax2.tick_params(axis='y', labelcolor='blue')


print("time:", time)
print("size:", size)


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