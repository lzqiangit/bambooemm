import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use("TkAgg")
plt.rcParams['font.sans-serif'] = ['SimHei']  # 指定默认字体为黑体
plt.rcParams['axes.unicode_minus'] = False  # 解决保存图像是负号'-'显示为方块的问题

bamboo = pd.read_csv('analyse/csv/query_bamboo_n22_l15.csv') # TODO
twoch = pd.read_csv('analyse/csv/query_twoch_n22_l15.csv')   # TODO

# 计算bamboo第二列元素的平均值
size = bamboo.iloc[:, 0]
bamboo_size = size.mean()
print("bamboo size: ", bamboo_size)

times = bamboo.iloc[:, 1]
bamboo_mean = times.mean()
print("bamboo mean: ", bamboo_mean)
# 计算twoch第二列元素的平均值   
size = twoch.iloc[:, 0]
twoch_size = size.mean()
print("twoch size: ", twoch_size)

times = twoch.iloc[:, 1]
twoch_mean = times.mean()
print("twoch mean: ", twoch_mean)

# 画散点图
plt.subplot(1, 2, 1)
