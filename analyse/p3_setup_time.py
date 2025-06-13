# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
bamboo_time = [[2440.3, 2272.71, 2437.43, 2412.67, 2469.92],
               [3777.67, 3544.93, 3546.17, 3522.9, 3703.58],
               [9683.56, 9753.27, 9576.6, 9566, 9408.24]]
twoch_time = [[8231.51, 8232.38, 8232.05, 8235.17, 8244.64],
              [32917.1, 32813.9, 32934.4, 32943.3, 32962.5],
              [132571, 131555, 131554, 131633, 131659]]
bamboo_time_avg = np.mean(bamboo_time, axis=1) / 1000
twoch_time_avg = np.mean(twoch_time, axis=1) / 1000
print("Bamboo average time:", bamboo_time_avg)
print("Two-channel average time:", twoch_time_avg)
figsize = 7, 6
# 设置图片大小
plt.figure(figsize=figsize)
# 绘图
l = np.array(['$2^{18}$', '$2^{20}$', '$2^{22}$'])
x_indices = np.arange(1, 4) 
n = np.array([18, 20, 22])
color = ['r', 'g', 'b']


# for i in range(3):
plt.plot(x_indices, bamboo_time_avg, 'o-', label='$BamVH(our)$', color='b')
# for i in range(3):
plt.plot(x_indices, twoch_time_avg, 'x--', label='$2ch_{FB}$', color='g')

plt.xlabel("Data set(n)", fontsize=14)
plt.ylabel('Average time(s)', fontsize=14)
# x刻度显示为l,并均匀刻度

plt.xticks(x_indices, l, fontsize=12)
plt.yticks(fontsize=12)
# 图例显示为两列三行
plt.legend(ncol=2, fontsize=12, loc='upper left')
plt.grid(True)

# 标题放在下方
# y刻度显示为l,并均匀刻度



plt.show()