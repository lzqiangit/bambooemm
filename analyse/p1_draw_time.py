# 定义numpy二维数组
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd

# 读取excel
data = pd.read_excel('analyse/csv/data_p1_p2.xlsx')
bamboo_time_row = np.array([2, 9, 16, 23])
twoch_time_row = np.array([4, 11, 18, 25])

# 绘图
l = np.array([128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768])
x_indices = np.arange(1, 10) 
n = np.array([16, 18, 20, 22])
color = ['r', 'g', 'b', 'y']

plt.figure(figsize=(10, 6))
for i in range(4):
    # print(data.iloc[bamboo_time_row[i], 2:])
    plt.plot(x_indices, data.iloc[bamboo_time_row[i], 2:], 'o-', label='bamboo($n=2^{{{}}}$)'.format(n[i]), color=color[i])
    # print(data.iloc[twoch_size_row[i], 2:])
    plt.plot(x_indices, data.iloc[twoch_time_row[i], 2:], 'x-', label='twoch($n=2^{{{}}}$)'.format(n[i]), color=color[i])
plt.xlabel("Maximum Volume for a label")
plt.ylabel('Time(s)')
# x刻度显示为l,并均匀刻度

print(x_indices)
plt.xticks(x_indices, l)
plt.legend()
plt.grid(True)
plt.title('Bamboo and Twoch Query Time')
# y刻度显示为l,并均匀刻度



plt.show()