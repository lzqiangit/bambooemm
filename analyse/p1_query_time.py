import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use("TkAgg")
plt.rcParams['font.sans-serif'] = ['SimHei']  # 指定默认字体为黑体
plt.rcParams['axes.unicode_minus'] = False  # 解决保存图像是负号'-'显示为方块的问题

df_16 = pd.read_csv('analyse/csv/volumn_query_time_16_9.csv')
df_18 = pd.read_csv('analyse/csv/volumn_query_time_18_9.csv')
df_20 = pd.read_csv('analyse/csv/volumn_query_time_20_9.csv')
df_22 = pd.read_csv('analyse/csv/volumn_query_time_22_9.csv')

# 按照第一列的数值进行分组
group_16 = df_16.groupby('l')
group_18 = df_18.groupby('l')
group_20 = df_20.groupby('l')
group_22 = df_22.groupby('l')

data_16 = []
data_18 = []
data_20 = []
data_22 = []
# 输出各个分支的name和平均值
for name, group in group_16:
    data_16.append((name, group['time'].mean()))
for name, group in group_18:
    data_18.append((name, group['time'].mean()))
for name, group in group_20:
    data_20.append((name, group['time'].mean()))
for name, group in group_22:
    data_22.append((name, group['time'].mean()))
    
# 使用matplotlib绘制折线图
x_16 = [x[0] for x in data_16]
y_16 = [x[1] for x in data_16]
slope, intercept = np.polyfit(x_16, y_16, 1)
y_fit_16 =  [intercept + slope * xi for xi in x_16]

x_18 = [x[0] for x in data_18]
y_18 = [x[1] for x in data_18]
slope, intercept = np.polyfit(x_18, y_18, 1)
y_fit_18 =  [intercept + slope * xi for xi in x_18]

x_20 = [x[0] for x in data_20]
y_20 = [x[1] for x in data_20]
slope, intercept = np.polyfit(x_20, y_20, 1)
y_fit_20 =  [intercept + slope * xi for xi in x_20]

x_22 = [x[0] for x in data_22]
y_22 = [x[1] for x in data_22]
slope, intercept = np.polyfit(x_22, y_22, 1)
y_fit_22 =  [intercept + slope * xi for xi in x_22]

# 2 ^ 16
# plt.subplot(1, 2, 1)
# plt.plot(x_16, y_fit_16, color='orange', label='拟合直线')
# plt.scatter(x_16[1:], y_16[1:], marker='.')
# plt.xlabel("Volume for a label")
# plt.ylabel('Time(ms)')
# # 显示网格
# plt.grid(True)
# plt.yticks(np.arange(5, 7.1, 0.2))
# plt.title("n = 2 ^ 16")

# 2 ^ 18
plt.subplot(1, 2, 1)
plt.plot(x_18, y_fit_18, color='orange', label='拟合直线')
plt.scatter(x_18[1:], y_18[1:], marker='.')
plt.xlabel("Volume for a label")
plt.ylabel('Time(ms)')
plt.grid(True)
# y轴刻度精细化为0.2
plt.yticks(np.arange(3.4, 7.1, 0.2))
plt.title("n = $2^{18}$")

# # 2 ^ 20
plt.subplot(1, 2, 2)
plt.plot(x_20, y_fit_20, color='orange', label='拟合直线')
# 剔除y值大于7的y和y对应的x值

plt.scatter(x_20[1:], y_20[1:], marker='.')
plt.xlabel("Volume for a label")
# plt.ylabel('Time(ms)')
plt.grid(True)
plt.yticks(np.arange(3.4, 7.1, 0.2))
plt.title("n = $2^{20}$")

# # 2 ^ 22
# plt.subplot(2, 2, 4)
# plt.plot(x_22, y_fit_22, color='orange', label='拟合直线')
# plt.scatter(x_22[1:], y_22[1:], marker='.')
# plt.xlabel("Maximum Volume for a label")
# plt.ylabel('Time(ms)')
# plt.title("n = 2 ^ 22")


plt.show()
