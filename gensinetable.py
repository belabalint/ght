import math
import matplotlib.pyplot as plt
import numpy as np
def swapper(tmp):
    x = np.uint16(tmp << 8)
    x |= tmp >> 8
    return x

table = [swapper(round(970 * (math.sin(math.pi * t/500)+1))) for t in range(1000)]
print(table)
plt.plot([swapper(x) for x in table])
plt.show()


'''statedefault = ['0' for i in range(80)]
statedefault[57] = '1'
statedefault[0] = '1'
statedefault[1] = '1'
statedefault[2] = '1'
statedefault[3] = '1'
statedefault[6] = '1'
statedefault[7] = '1'
statedefault[8] = '1'
statedefault[63] = '1'
statedefault[69] = '1'

print([f'{i}' for i in range(80)])'''
