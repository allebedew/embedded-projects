#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT     Serial

#define BLK_RSSID       V11
#define BLK_WTR_REL     V12
#define BLK_STRT        V13
#define BLK_WTR_LEFT    V14

#define PIN_WTR         D0

char auth[] = "406c847c32a54c3f83cba0dc37dccbe2";

// SDA D2, SCL D1, 5V
LiquidCrystal_I2C lcd(0x3F, 16, 2);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3*60*60);
WidgetLED water_led(BLK_WTR_REL);

// VARS

const uint8_t start_hour = 19;
const unsigned long water_duration = 30 * 60 * 1000;

unsigned long water_on_millis = 0;
unsigned long water_complete_millis = 0;
bool water_on = false;

void loop100ms();
void loop1s();
void loop10s();
void checkTimings();
void updateRelay();
void updateDisplay();
void updateBlink();

void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Bucha Watering Online!");

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_WTR, OUTPUT);
    digitalWrite(PIN_WTR, HIGH);

    lcd.init();
    lcd.backlight();

    WiFi.begin("Strawberry", "lenagoodgirl");
    Blynk.config(auth);

    timeClient.begin();
}

unsigned long last_upd1 = 0, last_upd2 = 0, last_upd3 = 0;

void loop() {
    if (millis() - last_upd1 > 100) {
        loop100ms();
        last_upd1 = millis();
    }
    if (millis() - last_upd2 > 1000) {
        loop1s();
        last_upd2 = millis();
    }
    if (millis() - last_upd3 > 10000) {
        loop10s();
        last_upd3 = millis();
    }
    Blynk.run();
}

void loop100ms() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!Blynk.connected()) {
            Blynk.connect();
        }
        timeClient.update();
    }

    checkTimings();
    updateRelay();
    updateDisplay();
}

void loop1s() {
    if (water_on) {
        updateBlink();
    }
}

void loop10s() {
    if (!water_on) {
        updateBlink();
    }
}

// BLYNK

BLYNK_WRITE(BLK_STRT) {
    if (param.asInt() > 0) {

        water_on = true;
        water_on_millis = millis();

    } else {

        water_on = false;
        water_complete_millis = millis();

    }
    updateRelay();
}

// MAIN LOGIC

void checkTimings() {
    if (!water_on &&
        timeClient.getHours() == start_hour && 
        timeClient.getMinutes() == 0 &&
        millis() - water_complete_millis > water_duration) {

        // start watering
        water_on = true;
        water_on_millis = millis();
        Blynk.virtualWrite(BLK_STRT, 1);
        Blynk.notify("Starting watering :)");
    }

    if (water_on && millis() - water_on_millis > water_duration) {

        // stop watering
        water_on = false;
        water_complete_millis = millis();
        Blynk.virtualWrite(BLK_STRT, 0);
        Blynk.notify("Watering finished");
    }
}

void updateRelay() {
    uint8_t relay = water_on ? LOW : HIGH;
    if (digitalRead(PIN_WTR) != relay) {
        Serial.print("Switching relay ");
        Serial.println(relay == LOW ? "ON" : "OFF");
        digitalWrite(PIN_WTR, relay);
        updateBlink();
    }
}

// ============================

void updateDisplay() {

    // Disp - 16 x 2
    
    // WiFi Status - 11x0 + 3
    lcd.setCursor(13, 0);
    switch (WiFi.status()) {
        case WL_IDLE_STATUS: 
            lcd.printf("IDL"); break;
        case WL_NO_SSID_AVAIL:
            lcd.printf("N/A"); break;
        case WL_CONNECT_FAILED: 
            lcd.printf("FAL"); break;
        case WL_CONNECTION_LOST:
            lcd.printf("LST"); break;
        case WL_DISCONNECTED: 
            lcd.printf("DIS"); break;
        case WL_CONNECTED:
            lcd.printf("%3.d", WiFi.RSSI()); break;
        default:
            lcd.printf("UNK"); break;
    }

    // Blk - 11x0 - 1
    lcd.setCursor(11, 0);
    if (Blynk.connected()) {
        lcd.printf("B");
    } else {
        lcd.printf("*");
    }

    // Time - 0x0 + 10
    lcd.setCursor(0, 0);
    lcd.printf("%2.1d:%2.2d:%2.2d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());

    lcd.setCursor(0, 1);
    if (water_on) {
        int sec_total = (water_duration - (millis() - water_on_millis)) / 1000;
        int sec = sec_total % 60;
        int min = sec_total / 60;
        lcd.printf("Water On %d:%.2d", min, sec);
    } else {
        lcd.printf("Water Off         ");
    }
}

void updateBlink() {
    Blynk.virtualWrite(BLK_RSSID, WiFi.RSSI());
    bool is_on = digitalRead(PIN_WTR) == LOW;
    if (is_on) {
        water_led.on();
    } else {
        water_led.off();
    }
    if (water_on) {
        int sec_total = (water_duration - (millis() - water_on_millis)) / 1000;
        Blynk.virtualWrite(BLK_WTR_LEFT, sec_total);
    } else {
        Blynk.virtualWrite(BLK_WTR_LEFT, 0);
    }
}
