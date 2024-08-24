#include <Keyboard.h>
#include <EEPROM.h>
#include "setup.h"

#define BAUD_RATE           9600
#define TIMEOUT             100

#define DATA_OFFSET         2
#define DATA_CUT            0
#define DATA_SIZE           32
#define INT_DATA_SIZE       (DATA_SIZE/sizeof(int))
#define LONG_DATA_SIZE      (DATA_SIZE/sizeof(long))

#define on                  0
#define off                 1
#define clk                 2
#define swh                 3

#define pressed(i)          (pressed_status &   (1 << i))
#define swap_pressed(i)     (pressed_status ^=  (1 << i))
#define reset_pressed(i)    (pressed_status &= ~(1 << i))

bool lock = true;
unsigned long key = 0;

typedef struct {
  String name;
  char char_;
  unsigned int code;
  unsigned int type;
  int status;
} key_t;

const key_t custom_key[] = {
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
};
const int custom_size = sizeof(custom_key) / sizeof(custom_key[0]);

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

typedef union {
  byte          b[DATA_SIZE];
  char          c[DATA_SIZE];
  int           i[INT_DATA_SIZE];
  unsigned int  u[INT_DATA_SIZE];
  unsigned long l[LONG_DATA_SIZE];
} data_t;

typedef struct {
  String name;
  void (*function)(String);
  bool lock;
} command_t;

const int keys_size = sizeof(cipher_keys) / DATA_SIZE;

void setup() {

  // set the lock
  lock = true;
  key = 0;

  // load the status
  for (int i = 0; i < custom_size; i++) {
    if (custom_key[i].status >= 0) {
      int s = custom_key[i].status;
      status[s].code = custom_key[i].code;
      status[s].type = custom_key[i].type == swh;
    }
  }

  // set the buttons
  for (int i = 0; i < buttons_size; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  // start the serial
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);

  // start the serial
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);
}

void write(String data) {
  // stop serial and start keyboard
  Serial.end();
  delay(10);
  Keyboard.begin();
  delay(10);

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

  // stop keyboard and start serial
  Keyboard.end();
  delay(10);
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);
}

int len_() {
  return (EEPROM.length() / DATA_SIZE) - DATA_OFFSET - DATA_CUT;
}

void len__(String data) {
  send(String(len_() - DATA_OFFSET - DATA_CUT));
}

String hex(int i) {
  String str = String(i, HEX);
  if (str.length() == 1) {
    str = "0" + str;
  }
  return str;
}

int read(data_t* data, int i) {
  int j = i / 2;
  int s = i % 2;
  int r = (data->i[j] >> (s ? 0 : 8)) & 0xFF;
  return r;
}

void write(data_t* data, int i, int v) {
  int j = i / 2;
  int s = i % 2;
  int r = (data->i[j] & (s ? 0xFF00 : 0xFF)) | ((v & 0xFF) << (s ? 0 : 8));
  data->i[j] = r;
}

void shuffle(int i, data_t* data) {
  int j = (i + 1) % DATA_SIZE;
  int s = read(data, i);
  int l = (j + ((s ^ (s >> 4)) & 0xF)) % DATA_SIZE;

  int t = read(data, l);
  write(data, l, read(data, j));
  write(data, j, t);
}

bool save_(int idx, data_t* data) {
  // check if the index is valid
  if (idx >= len_() || idx < 0) {
    return false;
  }

  // encrypt the data
  for (int k = 0; k < keys_size; k++) {
    const int key = k * INT_DATA_SIZE;

    // cipher the data
    for (int i = 0; i < INT_DATA_SIZE; i++) {
      data->u[i] ^= cipher_keys[i + key];
    }

    // shuffle the data
    for (int i = 0; i < DATA_SIZE; i++) {
      shuffle(i, data);
    }
  }

  // save the data
  int offset = idx * DATA_SIZE;
  for (int i = 0; i < DATA_SIZE; i++) {
    EEPROM.update(i + offset, (int)data->b[i]);
  }

  return true;
}

bool load_(int idx, data_t* data) {
  // check if the index is valid
  if (idx >= len_() || idx < 0) {
    return false;
  }

  // load the data
  int offset = idx * DATA_SIZE;
  for (int i = 0; i < DATA_SIZE; i++) {
    data->b[i] = EEPROM.read(i + offset);
  }

  // decrypt the data
  for (int k = keys_size - 1; k >= 0; k--) {
    const int key = k * INT_DATA_SIZE;

    // shuffle the data
    for (int i = DATA_SIZE - 1; i >= 0; i--) {
      shuffle(i, data);
    }

    // cipher the data
    for (int i = INT_DATA_SIZE - 1; i >= 0; i--) {
      data->u[i] ^= cipher_keys[i + key];
    }
  }

  return true;
}

int extract_int(String* data_ptr) {
  // check if the data is empty
  if (data_ptr->length() == 0) {
    return -1;
  }

  // extract the number
  String data = *data_ptr;
  bool hexa = false;
  int multiplier = 10;
  int idx = 0;
  char* c = (char*)data.c_str();

  // detect if it is a hexa number
  if (data[0] == '0' && (data[1] == 'x' || data[1] == 'X')) {
    hexa = true;
    multiplier = 16;
    c += 2;
  }

  // extract the number
  for (;*c != ' ' && *c != 0;c++) {
    if (*c >= '0' && *c <= '9') {
      idx = idx * multiplier + *c - '0';
    } else if (hexa) {
      if (*c >= 'a' && *c <= 'f') {
        idx = idx * multiplier + *c - 'a' + 10;
      } else if (*c >= 'A' && *c <= 'F') {
        idx = idx * multiplier + *c - 'A' + 10;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  *data_ptr = data.substring(c - data.c_str() + 1);
  return idx;
}

void send(String data) {
  Serial.println(data);
}

bool str_to_data(String* str, data_t* data) {
  // check if the data is too long
  int len = str->length();
  if (len + 1 > DATA_SIZE) {
    return false;
  }

  // copy the data
  int i = 0;
  for (; i < len; i++) {
    data->c[i] = (*str)[i];
  }
  data->b[i++] = 0;
  for (; i < DATA_SIZE; i++) {
    char r = (char)random(32, 127);
    data->c[i] = r;
  }

  return true;
}

bool data_to_str(data_t* data, String* str) {
  // load the data
  for (int i = 0; i < DATA_SIZE; i++) {
    if (data->c[i] == 0) {
      break;
    }
    *str += data->c[i];
  }

  return true;
}

bool check_idx(int* idx) {
  if (*idx < 0) {
    return false;
  }
  *idx += DATA_OFFSET;
  if (*idx >= len_()) {
    return false;
  }
  return true;
}

void save(String data) {
  // extract the index
  int idx = extract_int(&data);
  if (!check_idx(&idx)) {
    send("Invalid index");
    return;
  }

  // copy the data
  data_t d;
  if (!str_to_data(&data, &d)) {
    send("Data too long");
    return;
  }

  // send result
  if (save_(idx, &d)) {
    send("Saved");
  } else {
    send("Error in saving");
  }
}

void load(String data) {
  // extract the index
  int idx = extract_int(&data);
  if (!check_idx(&idx)) {
    send("Invalid index");
    return;
  }

  // load the data
  data_t d;
  if (!load_(idx, &d)) {
    send("Error in loading");
    return;
  }

  // extract the data
  String str;
  if (!data_to_str(&d, &str)) {
    send("Error in converting");
    return;
  }

  // send the data
  send(str);
}

void clear(String data) {
  if (data.length() == 0) {
    // clear the data
    data_t d;
    int i = 0;
    d.b[++i] = 0;
    d.b[++i] = 0;

    // iterate over the data
    for (int idx = DATA_OFFSET; idx < len_() - DATA_CUT; idx++) {
      for (i = 2; i < DATA_SIZE; i++) {
        char r = (char)random(32, 127);
        d.c[i] = r;
      }

      // save the data
      if (!save_(idx, &d)) {
        send("Error in saving");
        return;
      }
    }
  } else {
    // extract the index
    int idx = extract_int(&data);
    if (!check_idx(&idx)) {
      send("Invalid index");
      return;
    }

    // clear the data
    data_t d;
    int i = 0;
    d.b[i++] = 0;
    d.b[i++] = 0;
    for (; i < DATA_SIZE; i++) {
      char r = (char)random(32, 127);
      d.c[i] = r;
    }

    // send result
    if (!save_(idx, &d)) {
      send("Error in clearing");
    }
  }
  send("Cleared");
}

void keycode(String data) {
  if (data.length() <= 1) {
    for (int i = 0; i < custom_size; i++) {
      data += custom_key[i].name + ":0x" + hex(custom_key[i].char_);
      if (i < custom_size - 1)
        data += ",";
    }
    send(data);
    return;
  } else {
    for (int i = 0; i < custom_size; i++) {
      if (data == custom_key[i].name) {
        send("0x" + hex(custom_key[i].char_));
        return;
      }
    }
    send("Unknown key");
  }
}

void echo(String data) {
  write(data);
  send(data);
}

void locked(String data) {
  if (lock) {
    send("Device is locked");
  } else {
    send("Device is unlocked");
  }
}

unsigned long get_true_key() {
  data_t d;
  load_(0, &d);
  for (int i = 1; i < LONG_DATA_SIZE; i++) {
    if (d.l[i] != d.l[0]) {
      return 0;
    }
  }
  return d.l[0];
}

bool unlock(unsigned long key) {
  unsigned long true_key = get_true_key();

  // check if the key is correct
  if (key == true_key) {
    lock = false;
    return true;
  }
  return false;
}

bool extract_key(String data, unsigned long* key) {
  *key = 0;
  for (int i = 0; i < data.length(); i++) {
    if(data[i] < '0' || data[i] > '0' + buttons_size - 1) {
      return false;
    }
    int p = data[i] - '0';
    *key = *key << 4 | p;
  }
  return true;
}

void set_key(String data) {
  unsigned long key = 0;
  if (!extract_key(data, &key)) {
    send("Invalid key");
    return;
  }
  data_t d;
  for (int i = 0; i < LONG_DATA_SIZE; i++) {
    d.l[i] = key;
  }
  if (!save_(0, &d)){
    send("Error in saving");
    return;
  }
  send("Key set");
}

void get_key() {
  unsigned long true_key = get_true_key();
  String key = "";
  for (int i = (sizeof(true_key)*2) - 1; i >= 0; i--) {
    int c = (true_key >> (i * 4)) & 0xF;
    if(c > 0 || key.length() > 0) {
      key += String(c);
    }
  }
  if (key.length() == 0){
    key += "0";
  }
  send(key);
}

void key_(String data) {
  if (data.length() > 0){
    set_key(data);
  } else {
    get_key();
  }
}

void unlock_(String data) {
  unsigned long key = 0;
  if (!extract_key(data, &key)) {
    send("Invalid key");
    return;
  }
  if (unlock(key)) {
    send("Device unlocked");
  } else {
    send("Invalid key");
  }
}

void lock_(String data) {
  lock = true;
  key = 0;
  send("Device locked");
}

void info(String data) {
  send("Baud rate: " + String(BAUD_RATE));
  send("Timeout: " + String(TIMEOUT));
  if(lock) {
    send("Device is locked");
  } else {
    send("Device is unlocked");
  }
}

const command_t commands[] = {
  {"echo",    echo,    false},
  {"write",   write,   false},
  {"send",    send,    false},
  {"keycode", keycode, false},
  {"locked",  locked,  false},
  {"len",     len__,   false},
  {"save",    save,    true },
  {"load",    load,    true },
  {"clear",   clear,   true },
  {"unlock",  unlock_, false},
  {"lock",    lock_,   false},
  {"key",     key_,    true},
  {"info",    info,    true},
};
const int commands_size = sizeof(commands) / sizeof(commands[0]);

void press_lock(int idx) {
  if (idx == 0) {
    key = 0;
  } else if (idx == 1) {
    key = key >> 4;
  } else {
    key = key << 4 | (idx-1);
  }
  unlock(key);
}

void press_unlock(int idx) {
  // map the index
  idx = map_buttons[idx];

  // check if the index is valid
  if (!check_idx(&idx)) {
    send("Invalid index");
    return;
  }

  // load the data
  data_t d;
  if (!load_(idx, &d)) {
    send("Error in loading");
    return;
  }

  // extract the data
  String str;
  if (!data_to_str(&d, &str)) {
    send("Error in converting");
    return;
  }

  write(str);
}

void command(String data) {
  // iterate over the commands
  for (int i = 0; i < commands_size; i++) {
    // check if the command is the same
    if (data.startsWith(commands[i].name)) {
      // check if the command needs the device to be unlocked
      if (commands[i].lock && lock) {
        send("Device is locked");
      } else {
        // remove the command name
        int len = commands[i].name.length();
        if (data.length() > len) {
          data = data.substring(len + 1);
        } else {
          data = "";
        }

        // execute the command
        commands[i].function(data);
      }
      return;
    }
  }

  // check if the command is help
  if(data.startsWith("help")) {
    send("Commands:");
    for (int i = 0; i < commands_size; i++) {
      send("  " + commands[i].name + (commands[i].lock && lock ? " (locked)" : ""));
    }
    return;
  }

  // send unknown command
  send("Unknown command, use help to get the list of commands");
}

void loop() {
  for (int i = 0; i < buttons_size; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      if (!pressed(i)) {
        swap_pressed(i);
        if (lock) {
          press_lock(i);
        } else {
          press_unlock(i);
        }
      }
    } else {
      reset_pressed(i);
    }
  }
  if (Serial.available() > 0) {
    String data = Serial.readString();
    if (data.length() > 0) {
      data.trim();
      command(data);
    }
  } else {
    delay(100);
  }
}