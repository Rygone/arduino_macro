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
5. Open the `com.txt` file and write the COM port of your Arduino. (e.g. `COM3`, you can find the COM port in the Arduino IDE under `Tools > Port`)
6. Open the `map_buttons` folder and flash the `map_buttons.ino` file to your Arduino.
7. Run the `map_buttons.py` file to map the buttons of your Arduino and follow the instructions.
    ```bash
    python map_buttons.py
    ```
8. Flash the `arduino_macro.ino` file to your Arduino.
9. Open the `terminal` folder and run the `init_terminal.py` file to initialize the terminal.
    ```bash
    python init_terminal.py
    ```
<!-- 10. Run the `terminal.py` file to use the macro keyboard.
    ```bash
    python terminal.py
    ``` -->