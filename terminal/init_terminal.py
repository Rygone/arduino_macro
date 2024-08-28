import os
import serial

#region variables
com_path = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', 'com.txt'))
com = None
if os.path.exists(com_path):
    com = open(com_path, 'r').read().strip()
if com is None:
    print(f'\nFile not found or not correct: {com_path}')
    print(f'You can save the com port in this file to avoid this message')
    while com is None:
        com = input('\nEnter the com port (e.g. COM3): ').strip()
        if com.startswith('COM') and com[3:].isnumeric():
            com = f'COM{com[3:]}'
        elif com.isnumeric():
            com = f'COM{com}'
        else:
            com = None
baud = 9600
#endregion

#region setup
print(f'Port: {com}')
print(f'Baud: {baud}')

serial_ = serial.Serial(com, baud)
serial_.timeout = 1
print('Connected to serial port')

print(f'Connected: {serial_.is_open}')
if not serial_.is_open:
    print('Failed to connect to serial port')
    exit()

mp = {
    '☺' : '\x01',
    '☻' : '\x02',
    '♥' : '\x03',
    '♦' : '\x04',
    '♣' : '\x05',
    '♠' : '\x06',
    '♂' : '\x0b',
    '♀' : '\x0c',
    '♫' : '\x0e',
    '☼' : '\x0f',
    '►' : '\x10',
    '◄' : '\x11',
    '↕' : '\x12',
    '‼' : '\x13',
    '¶' : '\x14',
    '§' : '\x15',
    '▬' : '\x16',
    '↨' : '\x17',
    '↑' : '\x18',
    '↓' : '\x19',
    '→' : '\x1a',
    '∟' : '\x1c',
    '↔' : '\x1d',
    '▲' : '\x1e',
    '▼' : '\x1f',
}

def read():
    r = serial_.readline()
    # print('', r)
    try:
        r = r.decode('utf-8').rstrip()
    except UnicodeDecodeError:
        r = str(r)[2:-1]
        r = r.rstrip()
    # print('<-', r)
    return r

def write(data):
    data = data.replace('♠♦♣♥', '')
    data = list(data)
    for i, c in enumerate(data):
        if c in mp:
            data[i] = mp[c][0]
    data = ''.join(data)
    # print('->', data)
    data = f'{data}\n'.encode('utf-8')
    # print('', data)
    serial_.write(data)

path_map    = os.path.join(os.path.dirname(__file__), 'map.txt')
path_custom = os.path.join(os.path.dirname(__file__), 'custom.txt')
if os.path.isfile(path_map):
    with open(path_map, 'r', encoding='utf-8') as f:
        map_ = {
            k: v
            for line in f.readlines()
            for line in [line[:-1]]
            if len(line) > 0
            for len_ in [len(line.split(':')[0])]
            for len_ in [1 if len_ <= 0 else len_]
            for line in [(line[:len_], line[len_ + 1:])]
            if len(line) == 2
            for k, v in [line]
        }
else:
    map_ = {}
if os.path.isfile(path_custom):
    with open(path_custom, 'r', encoding='utf-8') as f:
        custom = {
            k: v
            for line in f.readlines()
            for line in [line[:-1]]
            if len(line) > 0
            for len_ in [len(line.split(':')[0])]
            for len_ in [1 if len_ <= 0 else len_]
            for line in [(line[:len_], line[len_ + 1:])]
            if len(line) == 2
            for k, v in [line]
        }
else:
    custom = {}

def convert(data):
    data_ = ''
    for c in data:
        if c in map_:
            data_ += map_[c]
        elif c in mp:
            data_ += mp[c]
        else:
            raise Exception(f'Error: {c} (0x{ord(c):2x} not set')
    return data_

exclude = [
    0, 7, 8, 9, 10, 13, 27,
]
replace = lambda i: 128 <= i <= 159 or i in exclude
#endregion

#region len
write('len')
print(read())

#region echo default char
for group in [
    "qwertyuiopasdfghjklzxcvbnm",
    "QWERTYUIOPASDFGHJKLZXCVBNM"
    " ",
    "`1234567890-=",
    "~!@#$%^&*()_+",
    "[];\'\\,./",
    "{}:\"|<>?",
]:
    string = 'echo ' + group + '¶'
    write(string)
    string_keyboard = input()
    string_serial = read()
    print(len(string_keyboard), len(string_serial))
    for k, v in zip(string_keyboard, string_serial):
        map_[k] = v
#endregion

#region echo alt
all_key = "`1234567890-=qwertyuiopasdfghjklzxcvbnm[];\'\\,./"
for key in all_key:
    write(f'echo ♣♥{key}{key}♠♦¶')
    string_keyboard = input()
    string_serial = read()
    if len(string_keyboard) > 0:
        string_keyboard = string_keyboard[0]
        if string_keyboard not in map_:
            map_[string_keyboard] = f'♣♥{key}♠♦'
#endregion

#region get custom key codes
write('keycode')
kv = read()
while len(kv) > 0:
    key, value = kv.split(':')
    value = chr(int(value, 16))
    map_[value] = value
    custom[key] = value

    kv = read()
#endregion

#region unused_char
nb_per_line = 16

print(len(map_), len(set(map_.values())))
print()
print('key')
for i in range(256):
    c = chr(i)
    v = c in map_.values()
    k = c in map_

    d = (('=' if map_[c] == c else '±') if v else '+') if k else ('-' if v else ' ')
    if replace(i):
        c = ' '
    print(f'{i:2x}: {c:<2} {d}', end='  ' if i%nb_per_line != nb_per_line-1 else '\n')
if i%nb_per_line != nb_per_line-1:
    print()
print()

print('map')
for i, (k, v) in enumerate(sorted(map_.items())):
    v = str(v) + ','
    k = '' if k == chr(7) else k
    print(f'{k:<1}: {v:<7}', end='' if i%nb_per_line != nb_per_line-1 else '\n')
if i%nb_per_line != nb_per_line-1:
    print()
print()

nb_per_line //= 2
print('custom')
for i, (k, v) in enumerate(sorted(custom.items())):
    print(f'{k:<11} : {v}', end=',    ' if i%nb_per_line != nb_per_line-1 else ',\n')
if i%nb_per_line != nb_per_line-1:
    print()
print()
#endregion

#region sort map
print(f'save map in {path_map}')
with open(path_map, 'w', encoding='utf-8') as f:
    for k, v in sorted(map_.items()):
        f.write(f'{k}:{v}\n')

print(f'save custom in {path_custom}')
with open(path_custom, 'w', encoding='utf-8') as f:
    for k, v in sorted(custom.items()):
        f.write(f'{k}:{v}\n')
#endregion

#region close
serial_.close()
print('Disconnected from serial port')
#endregion