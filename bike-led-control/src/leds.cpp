
#include <Adafruit_NeoPixel.h>
#include "leds.h"

#define STRP1_PIN       5
#define STRP2_PIN       2

#define STRP1_LEDS      19 // Tail
#define STRP2_LEDS      32 // Front
#define BRGHT           255

Adafruit_NeoPixel strp1;
Adafruit_NeoPixel strp2;

void ledsBegin() {
    strp1 = Adafruit_NeoPixel(STRP1_LEDS, STRP1_PIN, NEO_GRB + NEO_KHZ800);
    strp1.begin();
    strp1.setBrightness(BRGHT);
    strp1.show();
    strp2 = Adafruit_NeoPixel(STRP2_LEDS, STRP2_PIN, NEO_RGB + NEO_KHZ800);
    strp2.begin();
    strp2.setBrightness(BRGHT);
    strp2.show();
}

struct LedsColor {
    uint8_t r, g, b;
};

void showColor(LedsColor color) {
    uint32_t color1 = strp1.Color(color.r, color.g, color.b);
    for (uint16_t i = 0; i < strp1.numPixels(); ++i) {
        strp1.setPixelColor(i, color1);
    }
    strp1.show();   
    uint32_t color2 = strp2.Color(color.r, color.g, color.b);
    for (uint16_t i = 0; i < strp2.numPixels(); ++i) {
        strp2.setPixelColor(i, color2);
    }
    strp2.show();
}

void ledsSetColor(uint8_t r, uint8_t g, uint8_t b) {
    LedsColor color = {r, g, b};
    showColor(color);
}

void ledsSetWheel(uint8_t pos) {
    pos = 255 - pos;
    LedsColor color;
    if (pos < 85) {
        color.r = 255 - pos * 3;
        color.g = 0;
        color.b = pos * 3;
    } else if(pos < 170) {
        pos -= 85;
        color.r = 0;
        color.g = pos * 3;
        color.b = 255 - pos * 3;
    } else {
        pos -= 170;
        color.r = pos * 3;
        color.g = 255 - pos * 3;
        color.b = 0;
    }
    showColor(color);
}

void ledsSetGauge(uint8_t pos) { // red yellow green blue violet
    pos = 255 - pos;
    LedsColor color;
    if (pos < 85) {
        color.r = 255 - pos * 3;
        color.g = pos * 3;
        color.b = 0;
    } else if(pos < 170) {
        pos -= 85;
        color.r = 0;
        color.g = 255 - pos * 3;
        color.b = pos * 3;
    } else {
        pos -= 170;
        color.r = pos;
        color.g = 0;
        color.b = 255 - pos * 3;
    }
    showColor(color);
}

void playStartupAnimation() {
    for (uint16_t j = 0; j < 20; j++) {
        for (uint16_t i = 0; i < strp1.numPixels(); ++i) {
            uint8_t c = i < j ? 255 : 0;
            strp1.setPixelColor(i, strp1.Color(c, c, c));
        }
        for (uint16_t i = 0; i < strp2.numPixels(); ++i) {
            uint8_t c = i < j ? 255 : 0;
            strp2.setPixelColor(i, strp2.Color(c, c, c));
        }
        strp1.show();
        strp2.show();
        delay(50);
    }
}
