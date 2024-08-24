# This script shows the algorithm of the cipher in the arduino code.

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

def shuffle(x, back=False):
    range_ = range(0, len_)
    if back:
        range_ = reversed(range_)
    for i in range_:
        a = (x[i] & 0xF) ^ ((x[i] >> 4) & 0xF)
        j = (i + 1) & len_mask
        k = (j + a) & len_mask
        print(i, j, k)
        x[k], x[j] = x[j], x[k]
    return x

def shuffle_back(x):
    return shuffle(x, True)

# set text
txt = 'Hello, World!'
print(txt)

# encode
txt = [ord(c) for c in txt]
txt += [0, 0]
txt += (np.random.rand(len_) * 256).astype(int).tolist()

# encrypt
enum = list(enumerate(key))
for i, k in reversed(enum):
    # print(i)
    txt = cipher(txt, k)
    txt = shuffle_back(txt)

# print cipher text
for i in txt:
    print(i, end=' ')
print()

# add an error
# txt[-1] += 1

# decrypt
for i, k in enum:
    # print(i)
    txt = shuffle(txt)
    txt = cipher(txt, k)

# decode
idx = [
    i
    for i in range(len(txt) - 1)
    if txt[i] == 0 and txt[i + 1] == 0
]
if len(idx) > 0:
    idx = idx[0]
    txt = txt[:idx]

txt = ''.join([chr(i) for i in txt])

# print output
print(txt)