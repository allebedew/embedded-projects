
#include <Arduino.h>
#include "bike.h"

#define RPM_PIN         3
#define BUFF_L          10
#define MAX_DEL         200000UL
#define MIN_DEL         2000UL

volatile unsigned long buff[BUFF_L];
volatile unsigned int buff_i = 0;
volatile int state = 0;
volatile unsigned long last_rise = 0;
volatile unsigned long err = 0;

void bikeBegin() {
    pinMode(RPM_PIN, INPUT_PULLUP);
    // pinMode(RPM_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}

void bikeOnTimerTick() {
    unsigned long delay = micros() - last_rise;
    if (buff[0] != 0 && delay > MAX_DEL) {
        for (int i=0; i<BUFF_L; ++i) {
            buff[i] = 0;
        }
        return;
    }

    if (state == digitalRead(RPM_PIN)) {
        return;
    }
    state = digitalRead(RPM_PIN);

    digitalWrite(LED_BUILTIN, state);

    if (state == LOW) {
        return;
    }

    if (delay < MIN_DEL) {
        err++;
        return;
    }

    last_rise = micros();

    buff[buff_i] = delay;
    buff_i = buff_i >= BUFF_L - 1 ? 0 : buff_i + 1;
}

void bikeOnLoop() {
}

int bikeGetRPM() {
    bool hasZeros = false;
    unsigned long sum = 0;
    noInterrupts();
    for (int i=0; i<BUFF_L; ++i) {
        if (buff[i] == 0) {
            hasZeros = true;
            break;
        }
        sum += buff[i];
    }
    interrupts();
    if (hasZeros) {
        return 0;
    }
    unsigned long avg = sum/BUFF_L;
    return 60000000/avg;
}

bool bikeIsOn() {
    return bikeGetRPM() > 0 || digitalRead(RPM_PIN) == LOW;
}

void bikePrintDebug() {
    noInterrupts();
    for (int i = 0; i < BUFF_L; i++) {
        Serial.print(buff[i]);
        Serial.print("-");
    }
    Serial.println();
    Serial.println(micros() - last_rise);
    Serial.println(err);
    Serial.println();
    Serial.println();
    interrupts();

    int rpm = bikeGetRPM();

    Serial.print("RPM: ");
    Serial.println(rpm);
    Serial.println();
}
