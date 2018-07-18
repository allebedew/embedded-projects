#include <Adafruit_NeoPixel.h>

#define PIN 13
#define NUM_LEDS_1 60
#define NUM_LEDS_2 72
#define NUM_LEDS NUM_LEDS_1 + NUM_LEDS_2
#define BRIGHTNESS 25

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();

  Serial.println("All levels");

  for (uint8_t i = 0; i < 128; i++) {
    strip.setPixelColor(i, levelType2Color(i, i * 2));
  }
  strip.show();
  delay(10000);

  Serial.println("Level 1 animation");
  
  for(int j = 0; j < 5; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      showLevelType1(i);
      delay(10);    
    }
  }

  Serial.println("Level 2 animation");
  
  for(int j = 0; j < 5; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      showLevelType2(i);
      delay(10);
    }
  }

}

void loop() {

  Serial.println("Moving pixel");
  
  
  for (int i = 0; i < strip.numPixels(); i++) {

    strip.setPixelColor(i-1, 0, 0, 0);

    uint8_t level = (255 * i) / strip.numPixels();
    uint32_t color = levelType1Color(i, level);

    Serial.print(i);Serial.print(" ");
    Serial.print(level);Serial.print(" ");
    Serial.println(color);
    
    strip.setPixelColor(i, color);
    strip.show();
    delay(250);
  }
}

void showLevelType1(uint8_t a) {
  for (uint16_t i = 0; i < strip.numPixels(); ++i) {
    strip.setPixelColor(i, levelType1Color(i, a));
  }
  strip.show();
}

void showLevelType2(uint8_t a) {
  for (uint16_t i = 0; i < strip.numPixels(); ++i) {
    strip.setPixelColor(i, levelType2Color(i, a));
  }
  strip.show();
}

uint32_t levelType1Color(uint16_t i, uint8_t a) {
  const uint8_t steps = 5;
  uint8_t stepSize = 255 / steps;
  uint8_t step = a / stepSize;
  uint8_t c = (a - (step * stepSize)) * steps;
  switch (step) {
    case 0: return color(i, 0, c, 255);
    case 1: return color(i, 0, 255, 255 - c);
    case 2: return color(i, c, 255, 0);
    case 3: return color(i, 255, 255 - c, 0);
    default: return color(i, 255, 0, 0);
  }
}

uint32_t levelType2Color(uint16_t i, uint8_t a) {
  const uint8_t steps = 6;
  uint8_t stepSize = 255 / steps;
  uint8_t step = a / stepSize;
  uint8_t c = (a - (step * stepSize)) * steps;
  switch (step) {
    case 0: return color(i, 0, 0, c);
    case 1: return color(i, 0, c, 255);
    case 2: return color(i, 0, 255, 255 - c);
    case 3: return color(i, c, 255, 0);
    case 4: return color(i, 255, 255 - c, 0);
    default: return color(i, 255, 0, 0);
  }
}

uint32_t color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
  if (i >= 0 && i < NUM_LEDS_1) {
    return strip.Color(r, g, b);
  } else if (i >= NUM_LEDS_1 && i < NUM_LEDS_1 + NUM_LEDS_2) {
    return strip.Color(g, r, b);
  } else {
    return 0;
  }  
}

