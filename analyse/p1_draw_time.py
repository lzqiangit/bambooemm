# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
data = pd.read_excel('analyse/csv/data_p1_p2.xlsx')
bamboo_time_row = np.array([2, 9, 16])
twoch_time_row = np.array([4, 11, 18])

figsize = 7, 6
# 设置图片大小
plt.figure(figsize=figsize)
# 绘图
l = np.array([512, 1024, 2048, 4096, 8192])
x_indices = np.arange(1, 6) 
n = np.array([16, 18, 20])
color = ['r', 'g', 'b']

for i in range(3):
    plt.plot(x_indices, data.iloc[bamboo_time_row[i], 4:9], 'o-', label='$BamVH(our)(n=2^{'+ str(n[i]) +'}$)', color=color[i])

for i in range(3):
    plt.plot(x_indices, data.iloc[twoch_time_row[i], 4:9], 'x--', label='$2ch_{FB}(n=2^{'+str(n[i])+'}$)', color=color[i])

plt.xlabel("Maximum volume of all keywords(${\ell}$)", fontsize=14)
plt.ylabel('Time(ms)', fontsize=14)
# x刻度显示为l,并均匀刻度

plt.xticks(x_indices, l, fontsize=12)
plt.yticks(fontsize=12)
# 图例显示为两列三行
plt.legend(ncol=2, fontsize=12)
plt.grid(True)

# y刻度显示为l,并均匀刻度



plt.show()