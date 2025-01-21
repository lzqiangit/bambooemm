import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use("TkAgg")

data_16 = np.array([2.21456, 8.09995, 31.3099, 132.44])
data_18 = np.array([2.07028, 8.17436, 33.8054, 137.474])
data_20 = np.array([2.45631, 9.79531, 39.1621, 166.618])
data_22 = np.array([4.03117, 15.6988, 66.0963, 295.917])

x = np.array([7, 9, 11, 13])

plt.plot(x, data_16, 'o-', label='16')
plt.plot(x, data_18, 'x-', label='18')
plt.plot(x, data_20, '.-', label='20')
plt.plot(x, data_22, '*-', label='22')
plt.xlabel("Maximum Volume for a label")
plt.ylabel('Time(ms)')
# x刻度显示为128, 512, 2048, 819
plt.xticks(x, [128, 512, 2048, 8192])
# 显示注释 z^16, z^18, z^20, z^22
# 指数形式显示
plt.legend(['$2^{16}$', '$2^{18}$', '$2^{20}$', '$2^{22}$'])

plt.grid(True)

plt.show()
