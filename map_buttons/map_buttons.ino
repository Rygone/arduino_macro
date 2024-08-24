// map_buttons
#define BAUD_RATE           9600
#define TIMEOUT             100

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

unsigned long pressed_ = 0;

#define pressed(i)          ((pressed_ >> i) & 1)
#define press(i)            (pressed_ |= (1 << i))
#define release(i)          (pressed_ &= ~(1 << i))

void setup() {

  // start the serial
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);

  // pull up the button pins
  unsigned long buttons = BUTTONS;
  for (int i = 0; buttons > 0; i++, buttons >>= 1) {
    if (buttons & 1) {
      pinMode(i, INPUT_PULLUP);
    }
  }


  // set the RGB pins
  RGB(255, 0, 0);
}

unsigned int iter = 0;
unsigned int div_f = 0xFFFF / 3;
unsigned int div_p = div_f + (unsigned int)5;
unsigned int div_m = div_f - (unsigned int)5;
#define apply(x) (x = (x < div_m) ? x / ((div_m) / 255) : ((x < div_p) ? 255 : ((x < 2 * div_m) ? 255 - (x - div_f) / ((div_m) / 255) : 0)))

void loop() {
  RGB(0, 0, 255);
  unsigned long buttons = BUTTONS;
  for (int i = 0; buttons > 0; i++, buttons >>= 1) {
    if (buttons & 1) {
      if (digitalRead(i) == LOW) {
        if (!pressed(i)) {
          press(i);
          Serial.println(String(i));
        }
      } else {
        if (pressed(i)) {
          release(i);
        }
      }
    }
  }
  delay(100);
}