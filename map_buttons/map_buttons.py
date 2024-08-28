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

buttons = []
#endregion

#region get buttons
try:
    while True:
        print(f'press button {len(buttons)} or press ctrl+c to exit')
        button = ''
        while button == '':
            button = serial_.readline().decode('utf-8').strip()
            if button in buttons:
                print(f'button {button} already pressed, press ctrl+c to exit')
                button = ''
        print(f'button: {button}')
        buttons.append(button)
except KeyboardInterrupt:
    pass
print()

print(f'press reset button')
reset = ''
while reset == '':
    reset = serial_.readline().decode('utf-8').strip()
    if reset in buttons:
        reset = [i for i, b in enumerate(buttons) if b == reset][0]
    else:
        reset = ''
print(f'reset button: {reset}')

print(f'press enter button')
enter = ''
while enter == '':
    enter = serial_.readline().decode('utf-8').strip()
    if enter in buttons:
        enter = [i for i, b in enumerate(buttons) if b == enter][0]
    else:
        enter = ''
print(f'enter button: {enter}')

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
    while len(lines) > 0 and lines[0] == '':
        lines.pop(0)
#endregion

#region add buttons
print('Adding buttons')
lines += [
    '#pragma region Buttons',
    'const int buttons[] = {',
    '  '+', '.join(buttons),
    '};',
    'const int buttons_size = sizeof(buttons) / sizeof(buttons[0]);',
    '',
    f'const int reset_button = {reset};',
    f'const int enter_button = {enter};',
    '',
    'int map_buttons[] {',
    '  '+', '.join((str(i) for i in range(len(buttons)))),
    '};',
    '#pragma endregion Buttons',
]
#endregion

#region save buttons setup.h
with open(path, 'w') as file:
    file.write('\n'.join(lines))
print('Buttons saved')
#endregion
