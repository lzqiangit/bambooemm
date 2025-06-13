# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
bamboo_mem = [[13.296, 13.221, 13.286, 13.416, 13.381],
               [31.856, 31.755, 31.497, 31.813, 31.771],
               [110.184, 109.084, 110.065, 110.51, 109.108]]
twoch_mem = [[8.302, 8.301, 8.304, 8.295, 8.314],
              [30.447, 30.322, 30.39, 30.461, 30.39],
              [112.649, 112.67, 112.812, 112.683, 112.681]]
bamboo_mem_avg = np.mean(bamboo_mem, axis=1)
twoch_mem_avg = np.mean(twoch_mem, axis=1)
print("Bamboo average mem:", bamboo_mem_avg)
print("Two-channel average mem:", twoch_mem_avg)

figsize = 7, 6
# 设置图片大小
plt.figure(figsize=figsize)
# 绘图
l = np.array(['$2^{18}$', '$2^{20}$', '$2^{22}$'])
x_indices = np.arange(1, 4) 
n = np.array([18, 20, 22])
color = ['r', 'g', 'b']

# for i in range(3):
plt.plot(x_indices, bamboo_mem_avg, 'o-', label='$BamVH(our)$', color='b')
# for i in range(3):
plt.plot(x_indices, twoch_mem_avg, 'x--', label='$2ch_{FB}$' , color='g')

plt.xlabel("Data set(n)", fontsize=14)
plt.ylabel('Average size(MB)', fontsize=14)
# x刻度显示为l,并均匀刻度

plt.xticks(x_indices, l, fontsize=12)
plt.yticks(fontsize=12)
# 图例显示为两列三行
plt.legend(ncol=2, fontsize=12, loc='upper left')
plt.grid(True)

# 标题放在下方
# y刻度显示为l,并均匀刻度



plt.show()