import os
import numpy as np

#region user parameters
nb_key = 4
len_key = 32
#endregion

#region load setup.h
path = os.path.join(os.path.dirname(__file__), 'arduino_macro', 'setup.h')
print(f'Loading setup.h : {os.path.normpath(path)}')

lines = []
if os.path.exists(path):
    with open(path, 'r') as file:
        lines = [line.rstrip() for line in file.readlines()]
    idx_start = [i for i, line in enumerate(lines) if '#pragma region Keys' in line]
    if len(idx_start) > 0:
        idx_start = idx_start[0]
        idx_end = [i for i, line in enumerate(lines) if '#pragma endregion Keys' in line][0]
        lines = lines[:idx_start] + lines[idx_end+1:]
    if len(lines) > 0 and lines[-1] != '':
        lines.append('')
    while lines[0] == '':
        lines.pop(0)
#endregion

#region generate random keys
print('Generating keys')
len_key >>= 1
val_max = 0xFFFF + 1
lines += [
    '#pragma region Keys',
    f'const unsigned int cipher_keys[] = {{',
]
for i in range(nb_key * 2):
    key = [
        f'0x{i:04X}'
        for i in (np.random.rand(len_key//2) * val_max).astype(int).tolist()
    ]
    lines += ['  ' + ','.join(key) + ',']
    if i % 2 == 1:
        lines += ['']
lines = lines[:-1]
lines += [
    '};',
    '#pragma endregion Keys',
]
#endregion

#region save keys setup.h
with open(path, 'w') as file:
    file.write('\n'.join(lines))
print('Keys saved')
#endregion