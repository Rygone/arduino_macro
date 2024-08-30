import os
import sys
from time import sleep

#region utils
sys.path.append(os.path.normpath(os.path.join(os.path.dirname(__file__), '..')))
import utils

com, baud = utils.variables()

serial_, mp, read, write, map_, custom, convert, replace = utils.setup(com, baud)
#endregion

#region get buttons
buttons = []
try:
    while True:
        print(f'\npress button {len(buttons)} or press ctrl+c if all buttons are pressed')
        button = ''
        while button == '':
            button = serial_.readline().decode('utf-8').strip()
            if button in buttons:
                print(f'button {len(buttons)} already pressed\n')
                print(f'\npress button {len(buttons)} or press ctrl+c if all buttons are pressed')
                button = ''
        sleep(0.5)
        print(f'button: {len(buttons)} successfully mapped to GPIO {button}')
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
        print(f'button: {len(buttons)} successfully mapped to GPIO {reset}')
        buttons.append(reset)
        reset = len(buttons) - 1
print(f'reset button: {reset}')

print(f'press enter button')
enter = ''
while enter == '':
    enter = serial_.readline().decode('utf-8').strip()
    if enter in buttons:
        enter = [i for i, b in enumerate(buttons) if b == enter][0]
    else:
        print(f'button: {len(buttons)} successfully mapped to GPIO {enter}')
        buttons.append(enter)
        enter = len(buttons) - 1
print(f'enter button: {enter}')
#endregion

#region utils
utils.close(serial_)
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
    f'#define reset_button {reset}',
    f'#define enter_button {enter}',
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
