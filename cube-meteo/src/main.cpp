
#include <Arduino.h>
#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#undef min
#undef max
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

const char ssid[] = "Omlet Low";
const char pass[] = "lenagoodgirl";
const char cond_req[] = "http://api.openweathermap.org/data/2.5/weather?q=Kiev,UA&appid=e3f3f0e149c736248b95119d5cb66a61&units=metric";

void connectWiFi();
void updateBrightness();
void updateDisplay();
void updateWeatherIfNeeded();
void fetchWeather();

TM1637Display display(D5, D6);
BH1750 lightMeter(0x23);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3*60*60);

bool weather_loaded = false;
float temp = 0;
int hum = 0;
unsigned long last_updated = 0;
bool light_sens_online = false;
uint8_t last_brightness = 255;

void setup() {
  Serial.begin(115200);
  while (!Serial) {};
  delay(1000);

  Wire.begin(); // D1, D2
  light_sens_online = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.print("Light sensor: ");
  Serial.println(light_sens_online ? "Online" : "Offline");

  updateBrightness();
  updateDisplay();

  connectWiFi();
}

void loop() {
  timeClient.update();
  updateBrightness();
  updateWeatherIfNeeded();
  updateDisplay();
  delay(500);
}

void connectWiFi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void updateWeatherIfNeeded() {
  if (last_updated == 0 || millis()-last_updated > 15*60*1000) {
    last_updated = millis();
    fetchWeather();
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Can't fetch, WiFi offline");
    weather_loaded = false;
    return;
  }
  Serial.println("Fetching...");

  HTTPClient http;
  http.begin(cond_req);
  int status = http.GET();

  if (status != 200) {
    Serial.print("Fetch failed. Status: ");
    Serial.println(status);
    http.end();
    weather_loaded = false;
    return;
  }

  String payload = http.getString();
  Serial.print("Payload: ");
  Serial.println(payload.length());
  http.end();

  // Serial.println(payload);

  DynamicJsonBuffer buff(1024);
  JsonObject& root = buff.parseObject(payload);
  if (!root.success()) {
    Serial.println("Error parsing payload");
    weather_loaded = false;
    return;
  }
  JsonObject& main = root["main"];
  temp = main["temp"];
  hum = main["humidity"];

  weather_loaded = true;

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" Hum: ");
  Serial.println(hum);
}

void updateBrightness() {
  int level;
  if (light_sens_online) {
    uint16_t lux = lightMeter.readLightLevel();
    level = min(lux / 3, 7);
  } else {
    level = 7;
  }

  if (last_brightness != level || last_brightness == 255) {
    Serial.print("Brightness set to ");
    Serial.println(level);

    display.setBrightness(level);
    last_brightness = level;

    updateDisplay();
  }
}

void updateDisplay() {
  if (weather_loaded) {
    int intTemp = temp;
    int mode = (millis() / 1000) % 14;

    if (mode < 10) { // Temp
      int absTemp = abs(temp);
      uint8_t d1 = absTemp / 10;
      uint8_t d2 = absTemp % 10;
      bool neg = intTemp < 0;
      uint8_t seg[4];
      if (d1 == 0) {
        seg[0] = 0;
        seg[1] = neg ? SEG_G : 0;
      } else {
        seg[0] = neg ? SEG_G : 0;
        seg[1] = display.encodeDigit(d1);
      }
      seg[2] = display.encodeDigit(d2);
      seg[3] = SEG_A|SEG_B|SEG_F|SEG_G;
      display.setSegments(seg);

    // } else if (mode >= 7 && mode < 9) { // Smile
    //   if (intTemp > 0) {
    //     uint8_t seg[] = {SEG_A|SEG_B|SEG_F|SEG_G, SEG_D|SEG_E, SEG_C|SEG_D, SEG_A|SEG_B|SEG_F|SEG_G};
    //     display.setSegments(seg, 4, 0);
    //   } else {
    //     uint8_t seg[] = {SEG_A|SEG_B|SEG_F|SEG_G, SEG_E|SEG_G, SEG_C|SEG_G, SEG_A|SEG_B|SEG_F|SEG_G};
    //     display.setSegments(seg, 4, 0);
    //   }

    } else if (mode >= 10 && mode < 12) { // Humidity
      uint8_t pcnt[] = {SEG_A|SEG_B|SEG_F|SEG_G, SEG_C|SEG_D|SEG_E|SEG_G};
      display.setSegments(pcnt, 2, 2);
      display.showNumberDec(min(99, max(0, hum)), false, 2, 0);

    } else if (mode >= 12) { // Time
      bool dots = (millis() / 500) % 2 == 0;
      int h = timeClient.getHours();
      int m = timeClient.getMinutes();
      display.showNumberDecEx(h, dots ? 0xff : 0x00, false, 2, 0);
      display.showNumberDecEx(m, dots ? 0xff : 0x00, true, 2, 2);
    }
    
  } else {
    uint8_t seg[] = {SEG_G, SEG_G, SEG_G, SEG_G};
    display.setSegments(seg);
  }
}
