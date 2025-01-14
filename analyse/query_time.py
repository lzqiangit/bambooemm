import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('analyse/csv/query_time.csv')

print(df)

# 按照第一列的数值进行分组
group = df.groupby('l')

data = []
# 输出各个分支的name和平均值
for name, group in group:
    data.append((name, group['time'].mean()))
    
# 使用matplotlib绘制折线图
x = [x[0] for x in data]
y = [x[1] for x in data]
plt.plot(x[1:], y[1:], marker='o')
plt.xlabel('l')
plt.ylabel('time')
plt.title('query_time')
plt.show()
