#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

#define BLK_RSSID   V11
#define BLK_WTR     V12
#define BLK_OVRD    V13

#define PIN_WTR     D5

#define WATER_TIME  (60 * 60000)

char auth[] = "406c847c32a54c3f83cba0dc37dccbe2";

// SDA D2, SCL D1, 5V
LiquidCrystal_I2C lcd(0x3F, 16, 2);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3*60*60);

void loop100ms();
void loop1s();
void loop10s();
void checkWateringStatus();
void updateRelay();
void updateDisplay();
void startWatering();
void stopWatering();

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
    if (millis() - last_upd1 > 1000) {
        loop1s();
        last_upd1 = millis();
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

    checkWateringStatus();
    updateRelay();
    updateDisplay();
}

void loop1s() {
}

void loop10s() {
    Blynk.virtualWrite(BLK_RSSID, WiFi.RSSI());
    Blynk.virtualWrite(BLK_WTR, digitalRead(PIN_WTR));
}

// VARS

unsigned long last_time_trigger = 0;
unsigned long water_on_millis = 0;
bool water_override = false;
bool water_on = false;

// BLYNK

BLYNK_WRITE(BLK_OVRD) {
    water_override = param.asInt() > 0;
    updateRelay();
}

// MAIN LOGIC

void updateRelay() {
    digitalWrite(PIN_WTR, !(water_on || water_override));
    Blynk.virtualWrite(BLK_WTR, digitalRead(PIN_WTR) == LOW);
}

void checkWateringStatus() {

    


}

void checkOffTrigger() {
    if (water_on && (millis() - water_on_millis) > WATER_TIME) {
        stopWatering();
    }
}

void startWatering() {
    if (water_on) return;

    water_on = true;
    water_on_millis = millis();
}

void stopWatering() {
    if (!water_on) return;

    water_on = false;
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
    if (water_override) {
        lcd.printf("Water Override    ");
    } else if (water_on) {
        lcd.printf("Water On %d min   ", (WATER_TIME - (millis() - water_on_millis)) / 60000);
    } else {
        lcd.printf("Water Off         ");
    }
}
