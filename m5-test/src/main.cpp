
#include <stdint.h>
#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

bool lamp_on = false;

void updateDisplay();
void loop100ms();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {    
    M5.begin();

    M5.Lcd.setTextSize(2);
    

    Serial.begin(9600);
    Serial.print("Test Serial\n");

    WiFi.begin("Omlet Low", "lenagoodgirl");
    
    mqtt.setServer("10.0.1.3", 1883);
    mqtt.setCallback(mqttCallback);
}

unsigned long last_loop100ms = 0;

void loop() {
    if (millis() - last_loop100ms > 100) {
        last_loop100ms = millis();
        loop100ms();
    }
    
    M5.update();
    mqtt.loop();

    if (M5.BtnA.wasPressed()) {
        if (mqtt.connected()) {
            mqtt.publish("cmnd/floorlamp1/POWER", "ON");
        }
    }

    if (M5.BtnB.wasPressed()) {
        if (mqtt.connected()) {
            mqtt.publish("cmnd/floorlamp1/POWER", "OFF");
        }
    }
}

void loop100ms() {
    updateDisplay();

    if (WiFi.status() == WL_CONNECTED) {
        if (!mqtt.connected()) {
            if (mqtt.connect("m5", "device", "gasp-vest-let-craig")) {
                // delay(200);
                mqtt.subscribe("stat/floorlamp1/POWER");
            }
        }
    }
}

void updateDisplay() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("MQTT Test\n");

    if (WiFi.status() == WL_CONNECTED) {
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("WiFi Connected %s\nRSSI %d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else if (WiFi.status() == WL_DISCONNECTED) {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.printf("WiFi Disconnected\n");
    }

    if (mqtt.connected()) {
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("MQTT Connected\n");
    } else {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.printf("MQTT Disconnected %d\n", mqtt.state());   
    }

    if (lamp_on) {
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("Lamp On\n");
    } else {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.printf("Lamp Off\n");   
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    M5.Speaker.beep();

    char* msg = (char*)malloc(length + 1);
    memcpy(msg, payload, length);
    *(msg+length) = 0x00;

    if (strcmp(topic, "stat/floorlamp1/POWER") == 0) {
        if (strcmp(msg, "ON") == 0) {
            lamp_on = true;
        } else if (strcmp(msg, "OFF") == 0) {
            lamp_on = false;   
        }
    }

    free(msg);
}
