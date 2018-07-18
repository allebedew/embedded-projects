
#include "encoder.h"
#include <Arduino.h>

#define ENC_A 5   // DT
#define ENC_B 36  // CLK
#define SW 16  // SW

void isr_encoder();
void fw();
void bw();

volatile int val = 0;
volatile bool forward = true;
volatile bool fired = false;
volatile unsigned long last_isr = 0;

void encoder_begin() {
    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);
    attachInterrupt(ENC_A, isr_encoder, CHANGE);
    attachInterrupt(ENC_B, isr_encoder, CHANGE);
}

int encoder_value() {
    return val;
}

void isr_encoder() {
    if (micros() - last_isr < 1000) {
        return;
    }
    last_isr = micros();

    // FW -> 00 01 11 10 00
    // BW -> 00 10 11 01 00

    bool a = digitalRead(ENC_A) == LOW;
    bool b = digitalRead(ENC_B) == LOW;

    if (a != b) {
        if (!fired) {
            forward = !a;
            fired = true;
        }
    } else if (!a && !b) {
        if (fired) {
            if (forward) {
                fw();
            } else {
                bw();
            }
            fired = false;
        }
    }
}

void fw() {
    val++;
}

void bw() {
    val--;
}
