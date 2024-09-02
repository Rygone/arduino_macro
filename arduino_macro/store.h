#include <EEPROM.h>


// define the
#if defined(ARDUINO_AVR_LEONARDO)

#define DATA_SIZE           64

#define EEPROM_LEN          1024
#define EEPROM_READ(i)      EEPROM.read(i)
#define EEPROM_WRITE(i, v)  EEPROM.update(i, v)
#define EEPROM_BEGIN()

#else

#define DATA_SIZE           128

#define EEPROM_LEN          4096
#define EEPROM_READ(i)      EEPROM.read(i)
#define EEPROM_WRITE(i, v)  {EEPROM.write(i, v); EEPROM.commit();}
#define EEPROM_BEGIN()      EEPROM.begin(EEPROM_LEN)

#endif

// define the storage data
#define INT_DATA_SIZE       (DATA_SIZE/sizeof(int))
#define LONG_DATA_SIZE      (DATA_SIZE/sizeof(long))

const int keys_size = sizeof(cipher_keys) / DATA_SIZE;
int idxs[EEPROM_LEN / DATA_SIZE] = {0};

typedef union {
  byte          b[DATA_SIZE];
  char          c[DATA_SIZE];
  int           i[INT_DATA_SIZE];
  unsigned int  u[INT_DATA_SIZE];
  unsigned long l[LONG_DATA_SIZE];
} data_t;

// setup the keyboard
void store_setup() {
  // start the EEPROM
  EEPROM_BEGIN();

  // load the index
  for (int i = 0; i < EEPROM_LEN / DATA_SIZE; i++) {
    idxs[i] = i;
  }
  for (int k = keys_size - 1; k >= 0; k--) {
    for (int i = 0; i < EEPROM_LEN / DATA_SIZE; i++) {
      unsigned int l = i % DATA_SIZE;
      l = cipher_keys[k * DATA_SIZE + l];
      unsigned int j = (i + l) % (EEPROM_LEN / DATA_SIZE);
      int t = idxs[i];
      idxs[i] = idxs[j];
      idxs[j] = t;
    }
  }
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

int len_() {
  return EEPROM_LEN / DATA_SIZE;
}

bool save_(int idx, data_t* data) {
  // check if the index is valid
  if (idx >= len_() || idx < 0) {
    return false;
  }

  // get the index
  idx = idxs[idx];

  // check if null data
  bool all_null = true;
  for (int i = 0; i < LONG_DATA_SIZE; i++) {
    if (data->l[i] != 0) {
        all_null = false;
        break;
    }
  }

  if (!all_null) {
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
  }

  // save the data
  int offset = idx * DATA_SIZE;
  for (int i = 0; i < DATA_SIZE; i++) {
    EEPROM_WRITE(i + offset, (int)data->b[i]);
  }

  return true;
}

bool load_(int idx, data_t* data) {
  // check if the index is valid
  if (idx >= len_() || idx < 0) {
    return false;
  }

  // get the index
  idx = idxs[idx];

  // load the data
  int offset = idx * DATA_SIZE;
  for (int i = 0; i < DATA_SIZE; i++) {
    data->b[i] = EEPROM_READ(i + offset);
  }

  // check if null data
  bool all_null = true;
  for (int i = 0; i < LONG_DATA_SIZE; i++) {
    if (data->l[i] != 0) {
        all_null = false;
        break;
    }
  }  if (all_null) {
    return true;
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

bool clear_() {
  // clear the data
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM_WRITE(i, 0);
  }

  return true;
}