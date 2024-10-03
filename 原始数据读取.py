import struct
import matplotlib.pyplot as plt
import numpy as np

f = open("CAP00009.raw", "rb")


def getTempereture(x, y, f):
    f.seek((160 * y + x) * 4)
    data = f.read(4)
    temperature = struct.unpack("<f", data)
    return temperature[0]
data = np.zeros((160, 120), dtype=np.float32)
for x in range(160):
    for y in range(120):
        data[x, y] = getTempereture(x, y, f)
        
# 转置温度数组
data = data.transpose()

plt.imshow(data, cmap="hot", interpolation="nearest")
plt.colorbar()
plt.show()