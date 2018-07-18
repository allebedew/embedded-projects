
#include <Adafruit_NeoPixel.h>
#include "buttons.h"

#define BTN_LED_PIN     4
#define BTN_IN_PIN      A0
#define BTN_1_PIN       11
#define BTN_2_PIN       12

#define BTN_LEDS        2
#define ULONG_MAX       4294967295UL
#define READ_D          1000UL
#define LONGPRESS_D     1000000UL

Adafruit_NeoPixel btn_strp;
press_ptr press_callback, longpress_callback;
unsigned long last_read = 0;

void buttonBegin(press_ptr press, press_ptr longpress) {
    press_callback = press;
    longpress_callback = longpress;

    pinMode(BTN_1_PIN, INPUT_PULLUP);
    pinMode(BTN_2_PIN, INPUT_PULLUP);

    btn_strp = Adafruit_NeoPixel(BTN_LEDS, BTN_LED_PIN, NEO_RGB + NEO_KHZ800);
    btn_strp.begin();
    btn_strp.setBrightness(255);
    btn_strp.show();
}

void buttonSetLight(uint8_t level) {
    uint32_t color = btn_strp.Color(level, 0, 0);
    for (uint16_t i = 0; i < btn_strp.numPixels(); ++i) {
        btn_strp.setPixelColor(i, color);
    }
    btn_strp.show();
}

// const int btn_lvls[] = {335, 673, 535};
const int btn_lvls[] = {258, 518, 413}; // on USB

volatile int btn_flag = 0;
volatile unsigned long btn_flag_micros = ULONG_MAX, cooldown_micros = ULONG_MAX;
volatile int btn_state = 0;
volatile bool is_longpress = false;

void buttonOnTimerTick() {
    if (micros() - last_read < READ_D) {
        return;
    }
    last_read = micros();
    if (micros() - cooldown_micros < LONGPRESS_D) {
        return;
    }
    cooldown_micros = ULONG_MAX;
    if (btn_state != 0) {
        return;
    }

    int lvl = analogRead(BTN_IN_PIN);
    int btn = 0;
    for (int i = 0; i < 3; ++i) {
        int btn_lvl = btn_lvls[i];    
        if (lvl > btn_lvl - 50 && lvl < btn_lvl + 50) {
            btn = i + 1;
            break;
        }
    }

    if (btn == 0) {
        if (digitalRead(BTN_1_PIN) == LOW) {
            btn = 1;
        } else if (digitalRead(BTN_2_PIN) == LOW) {
            btn = 2;
        }
    }

    if (btn > 0 && btn == btn_flag && micros() - btn_flag_micros > LONGPRESS_D) {
        btn_state = btn;
        is_longpress = true;
        btn_flag = 0;
        btn_flag_micros = ULONG_MAX;

    } else if (btn == 0 && btn_flag > 0) {
        btn_state = btn_flag;
        is_longpress = false;
        btn_flag = 0;
        btn_flag_micros = ULONG_MAX;

    } else if (btn_flag != btn) {
        btn_flag = btn;
        btn_flag_micros = micros();
    }
}

void buttonOnLoop() {    
    // Serial.print("Btnlvl: ");
    // Serial.println(analogRead(BTN_IN_PIN));

    int btn_state_copy;
    bool is_longpress_copy;
    noInterrupts();
    btn_state_copy = btn_state;
    is_longpress_copy = is_longpress;
    interrupts();
    if (btn_state_copy == 0) {
        return;
    }
    if (is_longpress_copy) {
        longpress_callback(btn_state_copy);
    } else {
        press_callback(btn_state_copy);
    }
    noInterrupts();
    if (is_longpress_copy) {
        cooldown_micros = micros();
    }
    btn_state = 0;
    is_longpress = false;
    interrupts();
}
