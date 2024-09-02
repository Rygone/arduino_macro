# This script shows the algorithm of indexing the memory in the arduino code.

import numpy as np

# define constants
len_     = 32       # power of 2 > 16
len_mask = len_ - 1 # len_ - 1
val_max  = 0xFF + 1 # 256

# set seed
seed = 1
np.random.seed(seed)

# generate key
key = [
    (np.random.rand(len_) * val_max).astype(int).tolist()
    for _ in range(4)
]

# define functions
cipher = lambda x, k:[int(x) ^ (k) for x, k in zip(x, k)]

def shuffle(x, key=None, back=False):
    if key is not None:
        key = x
    range_ = range(0, len_)
    if back:
        range_ = reversed(range_)
    for i in range_:
        a = (key[i] & 0xF) ^ ((key[i] >> 4) & 0xF)
        j = (i + 1) & len_mask
        k = (j + a) & len_mask
        # print(i, j, k)
        x[k], x[j] = x[j], x[k]
    return x

def shuffle_back(x, key=None):
    return shuffle(x, key, True)

# set index
len_ = 32
idxs = [i for i in range(len_)]

# shuffle
for i in range(4):
    idxs = shuffle(idxs, key[i])

print("idxs :")
for i, idx in enumerate(idxs):
    print(f'{idx:2d}', end=' ' if i%8!=7 else '\n')