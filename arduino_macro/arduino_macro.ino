#include "setup.h"
#include "write.h"
#include "store.h"

#define VERSION             "0.4"

// select the pins to use
#if defined(ARDUINO_AVR_LEONARDO)
#define LEONARDO
#define BUTTONS             0x1DC7FF
#define RGB(r, g, b)        {}
#define USB
#else
#define ESP32
#define BUTTONS             0x1FE3FFF
#define RGB(r, g, b)        {analogWrite(14, 255-r); analogWrite(15, 255-g); analogWrite(16, 255-b);}
#endif

#define BAUD_RATE           9600
#define TIMEOUT             100
#define BRT_MAX_DELAY       5000

#define DATA_OFFSET         2
#define DATA_CUT            0

// define the buttons
bool lock = true;
unsigned long btn_key = 0;
unsigned long btn_time = 0;

typedef struct {
  String name;
  void (*function)(String);
  bool lock;
  String usage;
} command_t;

void setup() {

  // check if the key is valid
  get_true_key();

  // set the lock
  lock = true;
  btn_key = 0;
  btn_time = millis();

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

  // setup the keyboard
  write_setup();

  // setup the store
  store_setup();

  // set the color
  RGB(255, 0, 0);
}

void len(String data) {
  send(String(len_() - DATA_OFFSET - DATA_CUT));
}

String hex(int i) {
  String str = String(i, HEX);
  if (str.length() == 1) {
    str = "0" + str;
  }
  return str;
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

void send_locked() {
  if (lock) {
    send("Device is locked");
  } else {
    send("Device is unlocked");
  }
}

bool check_idx_(int* idx) {
  if (*idx < 0) {
    return false;
  }
  *idx += DATA_OFFSET;
  if (*idx >= len_()) {
    return false;
  }
  return true;
}
#define check_idx(idx) if (!check_idx_(&idx)) { send("Invalid index"); return; }

void save(String data) {
  // extract the index
  int idx = extract_int(&data);
  check_idx(idx);

  // copy the data
  data_t d;
  if (!str_to_data(&data, &d)) {
    send("Data too long");
    return;
  }

  // send result
  if (save_(idx, &d)) {
    send("Saved " + data);
  } else {
    send("Error in saving");
  }
}

void load(String data) {
  // extract the index
  int idx = extract_int(&data);
  check_idx(idx);

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
    check_idx(idx);

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
      send(custom_key[i].name + ":0x" + hex(custom_key[i].char_));
    }
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

// write the data
void write_(String data) {
  // stop serial
  Serial.end();
  delay(10);

  // write the data
  write(data);

  // start serial
  delay(10);
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);
}

void echo(String data) {
  write_(data);
  send(data);
}

void locked(String data) {
  send_locked();
}

unsigned long get_true_key() {
  data_t d;
  load_(0, &d);
  for (int i = 1; i < LONG_DATA_SIZE; i++) {
    if (d.l[i] != d.l[0]) {
      clear_();
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
    if(data[i] <= '0' || data[i] > '0' + buttons_size - 2) {
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
  btn_key = 0;
  send("Device locked");
}

void info(String data) {
  send("Version: " VERSION);
  send("Baud rate: " + String(BAUD_RATE));
  send("Timeout: " + String(TIMEOUT));
  send_locked();
}

const command_t commands[] = {
  {"echo",    echo,    false, "<data>",         }, // write the data and send it back
  {"write",   write_,  false, "<data>",         }, // write the data
  {"send",    send,    false, "<data>",         }, // send the data
  {"keycode", keycode, false, "[<name>]",       }, // get the keycode of the key
  {"locked",  locked,  false, "",               }, // check if the device is locked
  {"len",     len,     false, "",               }, // get the number of data space
  {"save",    save,    true,  "<index> <data>", }, // save the data at index
  {"load",    load,    true,  "<index>",        }, // load the data at index
  {"clear",   clear,   true,  "[<index>]"       }, // clear the data (default all)
  {"unlock",  unlock_, false, "<key>",          }, // unlock the device (default 0)
  {"lock",    lock_,   false, "",               }, // lock the device
  {"key",     key_,    true,  "[<key>]",        }, // get/set the key to unlock
  {"info",    info,    false, "",               }, // get the information
};
const int commands_size = sizeof(commands) / sizeof(commands[0]);

void press_lock(int idx) {
  unsigned long dt = millis() - btn_time;
  btn_time = millis();
  if (dt > BRT_MAX_DELAY) {
    btn_key = 0;
  }
  if (idx == reset_button) {
    btn_key = 0;
  } else if (idx == enter_button) {
    unlock(btn_key);
  } else {
    idx += (reset_button < idx) ?
      ((enter_button < idx) ? -1 : 0):
      ((enter_button < idx) ?  0 : 1);
    btn_key = btn_key << 4 | (idx);
  }
}

void press_unlock(int idx) {
  // map the index
  idx = map_buttons[idx];
  check_idx(idx);

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
      int len = commands[i].name.length();

      // check if the command starts the same but the next character is not a space
      if (data.length() > len && data[len] != ' ') {
        continue;
      }

      // check if the command needs the device to be unlocked
      if (commands[i].lock && lock) {
        send_locked();
      } else {
        // remove the command name
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
      String usage = commands[i].name + "  " + commands[i].usage;
      String lock_ = (commands[i].lock && lock ? " (locked)" : "");
      while (usage.length() < 21 && lock_.length() > 0) {
        usage += " ";
      }
      send(usage + lock_);
    }
    return;
  }

  // send unknown command
  send("Unknown command, use 'help' to get the list of commands");
}

void loop() {

  // check a button is pressed
  for (int i = 0; i < buttons_size; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      if (!pressed(i)) {
        // press the button
        swap_pressed(i);

        // set the color
        RGB(lock ? 255 : 0, lock ? 0 : 255, lock ? 0 : 255);

        // press the button
        if (lock) {
          press_lock(i);
        } else {
          press_unlock(i);
        }
      }
    } else {

      // set the color
      RGB(lock ? 255 : 0, 0, lock ? 0 : 255);

      // release the button
      reset_pressed(i);
    }
  }

  // check if there is data in the serial
  if (Serial.available() > 0) {
    String data = Serial.readString();
    if (data.length() > 0) {

      // set the color
      RGB(lock ? 255 : 0, lock ? 0 : 255, 255);

      // compute the data
      data.trim();
      command(data);

      // set the color
      RGB(lock ? 255 : 0, 0, lock ? 0 : 255);
    }
  } else {
    delay(100);
  }
}