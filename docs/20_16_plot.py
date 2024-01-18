import matplotlib.pyplot as plt
import numpy as np

irange = 1 << 20
orange = 1 << 16

def convert(output):
    output_sample = output/((4095*255 >> 7)*3*15*2/orange)
    return np.clip(output_sample, -32768, 32768)

def shift(output): return output >> 4

x = np.arange(-int(irange / 2), int(irange /2 ), 16)
y = convert(x)
z = shift(x)

fig, ax = plt.subplots()

ax.set(xlim = (-irange/2, irange/2), ylim = (-32768, 32767), yticks=range(-32768, 32768, 4096))

plt.plot(x, y, label='resid')
plt.plot(x, z, label='>>4')
plt.grid(True)
plt.legend(loc='best')
plt.show()
