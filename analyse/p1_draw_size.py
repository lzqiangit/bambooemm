# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
data = pd.read_excel('analyse/csv/data_p1_p2.xlsx')
bamboo_time_row = np.array([1, 8, 15])
twoch_size_row = np.array([3, 10, 17])

# 绘图
l = np.array([512, 1024, 2048, 4096, 8192])
x_indices = np.arange(1, 6) 
n = np.array([16, 18, 20])
color = ['r', 'g', 'b']

figsize = 7, 6
# 设置图片大小
plt.figure(figsize=figsize)

for i in range(3):
    y = data.iloc[bamboo_time_row[i], 4:9]
    y = y / 1024
    print(y)
    plt.plot(x_indices, y, 'o-', label='$BamVH(our)(n=2^{'+ str(n[i]) +'}$)', color=color[i])
for i in range(3):
    y = data.iloc[twoch_size_row[i], 4:9]
    y = y / 1024
    print(y)
    plt.plot(x_indices, y, 'x--', label='$2ch_{FB}(n=2^{'+str(n[i])+'}$)', color=color[i])

plt.xlabel("Maximum volume of all keywords(${\ell}$)", fontsize=14)
plt.ylabel('Size(kb)', fontsize=14)
# x刻度显示为l,并均匀刻度

plt.xticks(x_indices, l, fontsize=12)
plt.yticks(fontsize=12)
# 图例显示为两列三行
plt.legend(ncol=2, fontsize=12)
plt.grid(True)

# 标题放在下方
# y刻度显示为l,并均匀刻度



plt.show()