# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
data = pd.read_excel('analyse/csv/data_p9_p10.xlsx')
bamboo_row = np.array([2, 11, 20])
twoch_row = np.array([5, 14, 23])

# 绘图
l = np.array([512, 1024, 2048, 4096, 8192])
x_indices = np.arange(1, 6) 
n = np.array([16, 18, 20])
color = ['r', 'g', 'b']

figsize = 7, 6
fig, ax = plt.subplots(figsize=figsize)
ax.set_yscale('log')

for i in range(3):
    y = data.iloc[bamboo_row[i], 2:7]
    ax.plot(x_indices, y, 'o-', label='$BamVH(our)(n=2^{'+ str(n[i]) +'}$)', color=color[i])


for i in range(3):
    y = data.iloc[twoch_row[i], 2:7]
    ax.plot(x_indices, y, 'x--', label='$2ch_{FB}(n=2^{'+str(n[i])+'}$)', color=color[i])


plt.xlabel("Maximum volume of all keywords(${\ell}$)", fontsize=14)
ax.set_ylabel('Time(ms, log scale)', fontsize=14)
# x刻度显示为l,并均匀刻度
# 图例显示为两列三行
# 同时显示ax1和ax2的图例

# 合并两个轴的图例句柄和标签

# 合并后统一创建图例
plt.legend(ncol=2, fontsize=12)  # 可调整位置

plt.grid(True)
ax.set_xlabel("Maximum volume of all keywords(${\ell}$)", fontsize=14)
ax.set_xticks(x_indices)
ax.set_xticklabels(l, fontsize=12)
# 标题放在下方
# y刻度显示为l,并均匀刻度


# 确保显示最小和最大刻度
ymin, ymax = ax.get_ylim()
log_min = np.floor(np.log10(ymin))
log_max = np.ceil(np.log10(ymax))
yticks = np.logspace(log_min, log_max, num=int(log_max - log_min) + 1)
ax.set_yticks(yticks)

# 禁用科学计数法显示
from matplotlib.ticker import ScalarFormatter
ax.yaxis.set_major_formatter(ScalarFormatter())

# ... [之后的代码] ...
ax.set_xticklabels(l, fontsize=12)
plt.show()