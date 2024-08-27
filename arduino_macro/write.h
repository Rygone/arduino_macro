#define BLE_NAME            "Arduino Macro"
#define BLE_CPY             "RYGONE"

// select the pins to use
#if defined(ARDUINO_AVR_LEONARDO)
#include <Keyboard.h>
#define USB
#else
#include <BleKeyboard.h>
#define BLE
BleKeyboard Keyboard(BLE_NAME, BLE_CPY);

#endif

// define the types of keys
#define on                  0
#define off                 1
#define clk                 2
#define swh                 3
#define dly                 4

// define the status of the buttons
#define pressed(i)          (pressed_status &   (1 << i))
#define swap_pressed(i)     (pressed_status ^=  (1 << i))
#define reset_pressed(i)    (pressed_status &= ~(1 << i))

typedef struct {
  bool status;
  bool type;
  char code;
} status_t;

status_t status[] = {
  {false, false, char(0)},
  {false, false, char(0)},
  {false, false, char(0)},
  {false, false, char(0)}
};
const int status_size = sizeof(status) / sizeof(status[0]);

int pressed_status = 0;

// define the key structure
typedef struct {
  String name;
  char char_;
  unsigned int code;
  unsigned int type;
  int status;
} custom_key_t;

const custom_key_t custom_key[] = {
  {"win_on",    char(0x01), KEY_LEFT_GUI,    on,  0},
  {"win_off",   char(0x02), KEY_LEFT_GUI,    off, 0},
  {"ctrl_on",   char(0x03), KEY_LEFT_CTRL,   on,  1},
  {"ctrl_off",  char(0x04), KEY_LEFT_CTRL,   off, 1},
  {"alt_on",    char(0x05), KEY_LEFT_ALT,    on,  2},
  {"alt_off",   char(0x06), KEY_LEFT_ALT,    off, 2},
  {"shift_on",  char(0x18), KEY_LEFT_SHIFT,  on,  3},
  {"shift_off", char(0x19), KEY_LEFT_SHIFT,  off, 3},
  {"esc",       char(0x13), KEY_ESC,         clk,-1},
  {"tab",       char(0x1a), KEY_TAB,         clk,-1},
  {"caps",      char(0x17), KEY_CAPS_LOCK,   swh, 4},
  {"enter",     char(0x14), KEY_RETURN,      clk,-1},
  {"del",       char(0x0b), KEY_DELETE,      clk,-1},
  {"back",      char(0x0c), KEY_BACKSPACE,   clk,-1},
  {"up",        char(0x1e), KEY_UP_ARROW,    clk,-1},
  {"down",      char(0x1f), KEY_DOWN_ARROW,  clk,-1},
  {"left",      char(0x11), KEY_LEFT_ARROW,  clk,-1},
  {"right",     char(0x10), KEY_RIGHT_ARROW, clk,-1},
  {"delay_100", char(0x12), 100,             dly,-1},
  {"delay_1000",char(0x1d), 1000,            dly,-1},
};
const int custom_size = sizeof(custom_key) / sizeof(custom_key[0]);

// setup the keyboard
void write_setup() {
#if defined(BLE)
  Keyboard.begin();
#endif
}

// write the data
void write(String data) {
#if defined(USB)
  // start keyboard
  Keyboard.begin();
  delay(10);
#endif

  // write the data
  int len = data.length();
  for (int i = 0; i < len; i++) {
    bool custom = false;
    for (int j = 0; j < custom_size; j++) {
      if (data[i] == custom_key[j].char_) {
        if (custom_key[j].type == on) {
          status[custom_key[j].status].status = true;
          Keyboard.press(custom_key[j].code);
        } else if (custom_key[j].type == off) {
          status[custom_key[j].status].status = false;
          Keyboard.release(custom_key[j].code);
        } else if (custom_key[j].type == clk) {
          Keyboard.press(custom_key[j].code);
          delay(1);
          Keyboard.release(custom_key[j].code);
        } else if (custom_key[j].type == swh) {
          status[custom_key[j].status].status = !status[custom_key[j].status].status;
          Keyboard.press(custom_key[j].code);
          delay(1);
          Keyboard.release(custom_key[j].code);
        } else if (custom_key[j].type == dly) {
          delay(custom_key[j].code);
        }
        custom = true;
        break;
      }
    }
    if (!custom) {
      Keyboard.write(data[i]);
    }
    delay(1);
  }

  // release the keys unreleased
  for (int i = 0; i < status_size; i++) {
    if (status[i].status) {
      if (status[i].type) {
        Keyboard.press(status[i].code);
        delay(10);
      }
      Keyboard.release(status[i].code);
    }
  }
  delay(10);

#if defined(USB)
  // stop keyboard
  Keyboard.end();
#endif
}