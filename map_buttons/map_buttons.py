import os
import serial

#region variables
com = os.path.join(os.path.dirname(__file__), '..', 'com.txt')
if not os.path.exists(com):
    com = os.path.normpath(com)
    raise FileNotFoundError(f'File not found: {com} you have to save the com port in this file')
com = open(com, 'r').read().strip()
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

buttons = []
#endregion

#region get buttons
try:
    while True:
        print(f'press button {len(buttons)} or press ctrl+c to exit')
        button = ''
        while button == '':
            button = serial_.readline().decode('utf-8').strip()
        print(f'button: {button}')
        buttons.append(button)
except KeyboardInterrupt:
    print('Exiting...')
    serial_.close()
#endregion

#region load setup.h
path = os.path.join(os.path.dirname(__file__), '..', 'arduino_macro', 'setup.h')
print(f'Loading setup.h : {os.path.normpath(path)}')

lines = []
if os.path.exists(path):
    with open(path, 'r') as file:
        lines = [line.rstrip() for line in file.readlines()]
    idx_start = [i for i, line in enumerate(lines) if '#pragma region Buttons' in line]
    if len(idx_start) > 0:
        idx_start = idx_start[0]
        idx_end = [i for i, line in enumerate(lines) if '#pragma endregion Buttons' in line][0]
        lines = lines[:idx_start] + lines[idx_end+1:]
    if len(lines) > 0 and lines[-1] != '':
        lines.append('')
#endregion

#region add buttons
print('Adding buttons')
lines += [
    '#pragma region Buttons',
    'const int buttons[] = {',
    ', '.join(buttons),
    '};',
    'const int buttons_size = sizeof(buttons) / sizeof(buttons[0]);',
    '',
    'int map_buttons[] {',
    ', '.join((str(i) for i in range(len(buttons)))),
    '};',
    '#pragma endregion Buttons',
]
#endregion

#region save buttons setup.h
with open(path, 'w') as file:
    file.write('\n'.join(lines))
print('Buttons saved')
#endregion
