# Arduino Macro
Arduino Macro is a tool to use a Arduino Leonardo or Arduino ESP32 as a macro keyboard.
It is based on the [Arduino Keyboard library](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/) and [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library.
Depending on the board you use, you can use the Arduino Leonardo as a USB keyboard or the ESP32 as a Bluetooth keyboard.

## Setup
1. Install the [Arduino IDE](https://www.arduino.cc/en/software)
2. Add the libraries:
    - For the Arduino Leonardo: Go to `Sketch > Include Library > Manage Libraries...` and search for `Keyboard` and install it.
    - For the ESP32: Follow the instructions on the [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/README.md) repository.
3. Install python3 with the following packages:
    ```bash
    pip install pyserial
    pip install numpy
    ```
4. Connect the Arduino to your computer.
5. Create a `com.txt` file and write the COM port of your Arduino. (e.g. `COM3`, you can find the COM port in the Arduino IDE under `Tools > Port`)
6. Open the `map_buttons` folder and flash the `map_buttons.ino` file to your Arduino.
7. Run the `map_buttons.py` file to map the buttons of your Arduino and follow the instructions.
    ```bash
    python map_buttons.py
    ```
    (note: first `button` will be the `reset key` and the last `button` will be the `enter key`)
8. Go back to the root folder and run the `gen_key.py` file to generate a cipher key:
    ```bash
    python gen_key.py
    ```
9. Flash the `arduino_macro.ino` file to your Arduino.
10. Open the `terminal` folder and run the `init_terminal.py` file to initialize the terminal (carrefull, run this script in a terminal as it will emulate lots of keyboard combination).
    ```bash
    python init_terminal.py
    ```
11. Run the `terminal.py` file to use communication with the Arduino. (note: the arduino can change the COM port, so you may need to modify the `com.txt` file)
    ```bash
    python terminal.py
    ```
12. Once the terminal is open, you can send the commands `help` to see the available commands.
13. Once the Arduino setup as you want, you no longer need to run the `terminal.py` file, you can use the Arduino as a macro keyboard.


## arduino_macro scetch
The `arduino_macro` scetch is the main scetch to use the Arduino as a macro keyboard.
It can be programmed with the `terminal.py` or any other serial communication tool.

To unlock the device you can set press the `reset key` and then press the `password` (default `0`) and then press the `enter key` to validate the password.
You can change the `password` with the `key` command.
(note: the `password` is a succession of number between `1` and `nb_buttons-1` as the `reset key` and the `enter key` are reserved)

By default, the `buttons` are mapped to the corresponding `memories` (e.g. `button 0` is mapped to `memory 0`).
So to set `button 0` to `Hello World!`, you can use the following command in the `terminal.py`:
```
save 0 Hello World!
```
Once the button is set, you can press the `button 0` and it will write `Hello World!`.
This mapping can be changed in the `setup.h` file by changing the `map_buttons` array.


### Commands of the arduino_macro scetch
- `help`: Show the available commands.
- `echo <data>`: Write the data and send it back.
- `write <data>`: Write the data.
- `send <data>`: Send the data.
- `keycode [<name>]`: Get the keycode of the key.
- `locked`: Check if the device is locked.
- `len`: Get the number of data space.
- `save <index> <data>`: Save the data at index.
- `load <index>`: Load the data at index.
- `clear [<index>]`: Clear the data (default all).
- `unlock <key>`: Unlock the device (default 0).
- `lock`: Lock the device.
- `key [<key>]`: Get/set the key to unlock.
- `info`: Get the information.