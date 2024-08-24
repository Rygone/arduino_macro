#define BAUD_RATE           9600
#define TIMEOUT             100

#include "setup.h"
#include "write.h"
#include "store.h"

// select the pins to use
#if defined(ARDUINO_AVR_LEONARDO)
#define Type                "Leonardo"
#define BUTTONS             0x1DC7FF
#define RGB(r, g, b)        {}
#else
#define Type                "ESP32"
#define BUTTONS             0x1FE3FFF
#define RGB(r, g, b)        {analogWrite(14, 255-r); analogWrite(15, 255-g); analogWrite(16, 255-b);}
#endif


// define the buttons
bool lock = true;
unsigned long key = 0;

typedef struct {
  String name;
  void (*function)(String);
  bool lock;
} command_t;

void setup() {

  // check if the key is valid
  get_true_key();

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
  {"len",     len,     false},
  {"save",    save,    true },
  {"load",    load,    true },
  {"clear",   clear,   true },
  {"unlock",  unlock_, false},
  {"lock",    lock_,   false},
  {"key",     key_,    true},
  {"info",    info,    false},
};
const int commands_size = sizeof(commands) / sizeof(commands[0]);

void press_lock(int idx) {
  if (idx == 0) {
    key = 0;
  } else if (idx == buttons_size - 1) {
    unlock(key);
  } else {
    key = key << 4 | (idx);
  }
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