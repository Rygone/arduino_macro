import os
import sys

#region utils
sys.path.append(os.path.normpath(os.path.join(os.path.dirname(__file__), '..')))
import utils
print(utils.__dict__)

com, baud = utils.variables()

serial_, mp, read, write, map_, custom, convert, replace = utils.setup(com, baud)

utils.print_custom(custom)
#endregion

#region loop
try:
    while True:
        command = input('> ').strip()
        if command.startswith('save '):
            command = command.split()
            if len(command) > 2:
                command = command[0] + ' ' + command[1] + ' ' + convert(' '.join(command[2:]))
            else:
                command = ' '.join(command)
        elif command.startswith('echo ') or command.startswith('write ') or command.startswith('send '):
            command = command.split()
            if len(command) > 1:
                command = command[0] + ' ' + convert(' '.join(command[1:]))
            else:
                command = ' '.join(command)
        write(command)
        data = read()
        while len(data) > 0:
            print(data)
            data = read()
except KeyboardInterrupt:
    pass
#endregion

#region utils
utils.close(serial_)
#endregion