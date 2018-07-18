
//
// ***************************************************
// Bike Led Control
// ***************************************************
// 
// TODO:
// - button timer tick setup
// - gauge, dark gauge
// - map rpm to gauge
// - pulsing animation
//

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>
#include <EEPROM.h>

#include "leds.h"
#include "buttons.h"
#include "bike.h"

void timerTick();
void buttonPress(int button);
void buttonLongpress(int button);
void updateLedColor();
void readEEPROM();

// enum Mem { Mem_Mode, Mem_Color, Mem_ModeRPM };

enum Mode { Mode_Off, Mode_Solid, Mode_RPM, Mode_Animation };
enum ModeRPM { ModeRPM_Gauge, ModeRPM_R, ModeRPM_G, ModeRPM_B, ModeRPM_W };

Mode mode;
ModeRPM modeRPM;
uint8_t color, max_color = 5;
uint8_t anim_step;

void setup() {
    ledsBegin();
    buttonBegin(buttonPress, buttonLongpress);
    bikeBegin();

    readEEPROM();
    
    Serial.begin(9600);
    Serial.println("*** Bike Led System ***");
    
    Timer1.initialize(50);
    Timer1.attachInterrupt(timerTick);
}

void timerTick() {
    bikeOnTimerTick();
    buttonOnTimerTick();
}

unsigned long last_print = 0;

void loop() {
    bikeOnLoop();
    buttonOnLoop();
    updateLedColor();

    // if (millis() - last_print > 500) {
    //     last_print = millis();
    //     bikePrintDebug();
    // }

    delay(10);
}

void buttonPress(int button) {
    Serial.print("Btn: ");
    Serial.println(button);

    if (button == 1) {
        mode = mode >= Mode_Animation ? Mode_Off : Mode(mode + 1);
        switch (mode) {
            case Mode_Solid:
                color = 0;
            default:
                break;
        }
        // EEPROM.update(Mem_Mode, mode);

    } else if (button == 2) {
        switch (mode) {

            case Mode_Solid:
                color = color >= max_color ? 0 : color + 1;
                // EEPROM.write(Mem_Color, color);
                break;
            case Mode_RPM:
                // EEPROM.write(mem_rpm_color, rpm_color);
                break;
            default:
                break;
        }
    }

    Serial.print("Mode: ");
    Serial.print(mode);
    Serial.print(" Color: ");
    Serial.println(color);
}

void buttonLongpress(int button) {
    mode = Mode_Off;
    // EEPROM.write(Mem_Mode, mode);
}

void readEEPROM() {
    // uint8_t mem_mode = EEPROM.read(Mem_Mode);
    // uint8_t mem_color = EEPROM.read(Mem_Color);
    // uint8_t mem_mode_rpm = EEPROM.read(Mem_ModeRPM);

    mode = Mode_Off;
    color = 0;
    modeRPM = ModeRPM_Gauge;
    // mode = mem_mode == 255 ? Mode_Off : Mode(mem_mode);
    // color = mem_color == 255 ? 0 : mem_color;
    // modeRPM = mem_mode_rpm == 255 ? ModeRPM_Gauge : ModeRPM(mem_mode_rpm);
}

void updateLedColor() {
    switch (mode) {
        case Mode_Solid:
            switch (color) {
                case 0: ledsSetColor(255, 255, 255); break;
                case 1: ledsSetColor(255, 0, 0); break;
                case 2: ledsSetColor(0, 255, 0); break;
                case 3: ledsSetColor(0, 0, 255); break;
                case 4: ledsSetColor(255, 255, 0); break;
                case 5: ledsSetColor(0, 255, 255); break;
            }
            break;
        case Mode_RPM: {
            uint8_t level = max(0, min(255, map(bikeGetRPM(), 1000, 10000, 0, 255)));
            switch (modeRPM) {
                case ModeRPM_Gauge: ledsSetWheel(level);  break;
                case ModeRPM_R:     ledsSetColor(level, 0, 0); break;
                case ModeRPM_G:     ledsSetColor(0, level, 0); break;
                case ModeRPM_B:     ledsSetColor(0, 0, level); break;
                case ModeRPM_W:     ledsSetColor(level, level, level); break;
            }
            break;
        }
        case Mode_Animation:
            ledsSetWheel((millis() / 10) % 255);
            break;
        default:
            ledsSetColor(0, 0, 0);
            break;
    }

    buttonSetLight(mode == Mode_Off ? 0 : 255);
}
