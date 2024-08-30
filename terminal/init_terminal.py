import os
import sys

#region utils
sys.path.append(os.path.normpath(os.path.join(os.path.dirname(__file__), '..')))
import utils

com, baud = utils.variables()

serial_, mp, read, write, map_, custom, convert, replace = utils.setup(com, baud)
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

#region utils
utils.save_map(map_, custom)

utils.print_key(map_, replace)
utils.print_map(map_)
utils.print_custom(custom)

utils.close(serial_)
#endregion