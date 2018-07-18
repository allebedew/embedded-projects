
#include <Adafruit_NeoPixel.h>
#include "TimerOne.h"

#define BTN_LED   8
#define BTN_IN    A0
#define TACH      2
#define SPD       3
#define STRP1     12
#define STRP2     10

#define TACH_BUF  10

Adafruit_NeoPixel btn_strp = Adafruit_NeoPixel(2, BTN_LED, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strp1 = Adafruit_NeoPixel(10, STRP1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strp2 = Adafruit_NeoPixel(10, STRP2, NEO_RGB + NEO_KHZ800);

volatile int mode = 0;
unsigned int rpm = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello");

  btn_strp.begin();
  btn_strp.show();
  strp1.begin();
  strp1.show();
  strp2.begin();
  strp2.show();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TACH, INPUT_PULLUP);
  pinMode(SPD, INPUT_PULLUP);

  Timer1.initialize(1000); //50Hz
  Timer1.attachInterrupt(timer1);

//  attachInterrupt(digitalPinToInterrupt(TACH), tachInterrupt, CHANGE);
}

void loop() {
  Serial.println(analogRead(BTN_IN));
  
//  calculateRPM();
  updateLightForMode();
  displayMode(false);
  
//  printDebugInfo();

  delay(100);
}

void timer1() {
  readButton();
//  readTach();
}

// ================== Output ===================

uint8_t last_btn_brght = 0;

void buttonBrightness(uint8_t brght) { // 0..255
  uint32_t color = btn_strp.Color(0, brght, 0);
  for (int i = 0; i < btn_strp.numPixels(); ++i) {
    btn_strp.setPixelColor(i, color);
  }
  btn_strp.show();
  last_btn_brght = brght;
}

uint8_t last_r = 0, last_g = 0, last_b = 0;

void lightColor(uint8_t r, uint8_t g, uint8_t b) {
  last_r = r; 
  last_g = g; 
  last_b = b;
  uint32_t color1 = strp1.Color(r, g, b);
  uint32_t color2 = strp2.Color(r, g, b);
  for (int i = 0; i < strp1.numPixels(); ++i) {
    strp1.setPixelColor(i, color1);
  }
  for (int i = 0; i < strp2.numPixels(); ++i) {
    strp2.setPixelColor(i, color2);
  }
  strp1.show();   
  strp2.show();   
}

uint32_t wheel(byte pos, Adafruit_NeoPixel strp) {
  pos = 255 - pos;
  if(pos < 85) {
    return strp.Color(255 - pos * 3, 0, pos * 3);
  }
  if(pos < 170) {
    pos -= 85;
    return strp.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return strp.Color(pos * 3, 255 - pos * 3, 0);
}

void lightWheel(uint8_t level) {
  uint32_t color1 = wheel(level, strp1);
  uint32_t color2 = wheel(level, strp2);
  for (int i = 0; i < strp1.numPixels(); ++i) {
    strp1.setPixelColor(i, color1);
  }
  for (int i = 0; i < strp2.numPixels(); ++i) {
    strp2.setPixelColor(i, color2);
  }
  strp1.show();   
  strp2.show();  
}

void animationCircle() {
  for(uint16_t i=0; i<256; i++) { // 5 cycles of all colors on wheel
    lightWheel(i & 255);
    delay(5);
  }
}

// ================== Light Modes ===================
//
// 0 off, 1 red, 2 green, 3 blue, 4 white, 5 rpm
//

const int max_mode = 20;

void updateLightForMode() {

  lightWheel((255.0/20.0) * mode);
  return;
  
  switch (mode) {
    case 0: 
      lightColor(0, 0, 0);
      break;
    case 1: 
      lightColor(255, 0, 0);
      break;
    case 2: 
      lightColor(0, 255, 0);
      break;
    case 3: 
      lightColor(0, 0, 255);
      break;
    case 4: 
      lightColor(255, 255, 255);
      break;
    case 5: 
      animationCircle();
      break;
  }  
}

// ================ Button and Mode =================

const int btn_lvls[] = {258, 519, 413};
volatile int last_btn = 0;

void readButton() {
  int lvl = analogRead(BTN_IN);
  int btn = 0;
  for (int i = 0; i < 3; ++i) {
    int btn_lvl = btn_lvls[i];    
    if (lvl > btn_lvl - 50 && lvl < btn_lvl + 50) {
      btn = i + 1;
      break;
    }
  }
  if (btn == 0 && last_btn > 0) {
    handleButton(last_btn);
    last_btn = 0;
  } else {
    last_btn = btn;
  }
}

void handleButton(int btn) {
  if (btn == 1 && mode < max_mode) {
    mode++;
  } else if (btn == 2 && mode > 0) {
    mode--;
  }
}

int displayed_mode = 0;

void displayMode(bool force) {
  if (!force && mode == displayed_mode) {
    return;
  }
  displayed_mode = mode;
  
  Serial.print("Selected mode: ");
  Serial.println(mode);
  Serial.print("Level: ");
  Serial.println((255.0/20.0) * mode);
  
  if (mode == 0) {
    buttonBrightness(0);
    
  } else {
    for (int i = 0; i < mode; ++i) {
      delay(50);
      buttonBrightness(0);
      delay(50);
      buttonBrightness(255);
    }
  }
}

// ==================== Read Tach ======================

volatile bool lastTach = false;

void readTach() {
  bool tach = digitalRead(TACH);
  if (tach != lastTach) {
    lastTach = tach;
    handleTach();
  }
}

// ================== Tach Interrupt ===================

volatile int ints = 0;
volatile unsigned long lt = 0;
volatile unsigned long tach_buf[TACH_BUF];
volatile unsigned long tach_buf_i = 0;

void handleTach() {
  unsigned long t = micros();
  unsigned long d = t - lt;

//  if (d < 1000 || d > 60000) {
//    return;
//  }

  lt = t;

  tach_buf[tach_buf_i] = d / 1000;
  tach_buf_i++;
  if (tach_buf_i >= TACH_BUF) {
    tach_buf_i = 0;
  }
  
  ints++;
  
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}

void calculateRPM() {
  unsigned long sum = 0;
  for (int i = 0; i < TACH_BUF; ++i) {
    if (tach_buf[i] == 0) {
      rpm = 0;
      return;
    }
    sum += tach_buf[i];
  }
  unsigned long avg_t = sum / TACH_BUF;
  rpm = 60000 / avg_t;
}

// ================== Debug ===================

void printDebugInfo() {
  for (int i = 0; i < TACH_BUF; ++i) {
    Serial.print(tach_buf[i]);
    Serial.print(" ");
  }
  Serial.println("");
  
  Serial.print("RPM=");
  Serial.print(rpm);\
  Serial.print(" int=");
  Serial.print(ints);
  Serial.print(" v=");
  Serial.println(digitalRead(TACH));
}

