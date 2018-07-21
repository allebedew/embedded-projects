 /**************************************************************
 
 TODO:
 - Better hysteresis
 - OTA
 - Save state
 - Repeat sensor on error
 - InfluxDB reporting
 - HTTP API for homebridge
 - LED display
 
 *************************************************************/

#define BLYNK_PRINT Serial
//#define WITH_DHT22
//#define WITH_W1
//#define WITH_DISP
#define DEVICE      0

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <ArduinoOTA.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#ifdef WITH_DHT22
  #include <DHT.h>
#endif
#ifdef WITH_W1
  #include <OneWire.h> 
  #include <DallasTemperature.h>
#endif

#if DEVICE == 0
char auth[] = "a383e99a4d154bf0a27bde27d81a462b";
#else
char auth[] = "a383e99a4d154bf0a27bde27d81a462b";
#endif
char ssid[] = "Omlet Low";
char pass[] = "lenagoodgirl";

#define PIN_BLUE    13
#define PIN_RELAY   12
#define PIN_BUTTON  0
#define PIN_SENS    14
#define PIN_DISP    4

#define RELAY_ON    HIGH
#define RELAY_OFF   LOW
#define BLUE_ON     LOW
#define BLUE_OFF    HIGH
#define BUTTON_DN   LOW
#define BUTTON_UP   HIGH

// IN

#define VPIN_POWER  V5
#define VPIN_TARGET V6
#define VPIN_HYST   V9

// OUT

#define VPIN_HEATL  V7
#define VPIN_HEAT   V8
#define VPIN_TEMP_A V10
#define VPIN_TEMP_B V11
#define VPIN_HUM_A  V20
#define VPIN_HUM_B  V21
#define VPIN_TIND_A V30
#define VPIN_TIND_B V31

// =============================================
// Global Vars
// =============================================

BlynkTimer timer;
WidgetLED heatLed(VPIN_HEATL);
#ifdef WITH_DHT22
  DHT dht(PIN_SENS, DHT22);
#endif
#ifdef WITH_W1
  OneWire oneWire(PIN_SENS);
  DallasTemperature sensors(&oneWire);
#endif

bool isOn = true; // general switch
bool heatRequest = false; // heat relay should be on
bool buttonPushed = false;
float targetTemp = 24.0;
float hysteresis = 0.5;
float temp = NAN;
float hum = NAN;
float tempIdx = NAN; // temperature index
float sensorOk = false; // data from sensor read ok

// =============================================
// Main
// =============================================

void setup()
{
  Serial.begin(9600);
  Serial.println("Hello Motherfucker!");
  
  initLeds();
  setupDisplay();
  
  Blynk.begin(auth, ssid, pass);
  
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  #ifdef WITH_DHT22
    dht.begin();
  #endif
  #ifdef WITH_W1
    sensors.begin();
  #endif

  Serial.println("Loading finished.");
  
  updateBlynkControls();
  mainLoop();
  timer.setInterval(10000L, mainLoop);
}

void loop()
{
  ArduinoOTA.handle();
  Blynk.run();
  timer.run();

  if (digitalRead(PIN_BUTTON) == BUTTON_DN) {
    buttonPushed = true;
  } else if (buttonPushed) {
    buttonPushed = false;
    buttonAction();
  }
}

// =============================================
// Reads
// =============================================

BLYNK_READ(VPIN_TEMP_A)
{
  Serial.println("Blynk read request!");
}

// =============================================
// Actions
// =============================================

BLYNK_WRITE(VPIN_POWER)
{
  Serial.printf("Blynk power set: %d\n", param.asInt());

  isOn = param.asInt();
  updateHeatState();
  updateBlueLed();
  updateBlynk();
}

BLYNK_WRITE(VPIN_TARGET)
{
  targetTemp = param.asFloat();
  Serial.print("Blynk target set: "); Serial.println(targetTemp);
  
  updateHeatState();
  updateBlueLed();
  updateBlynk();
}

BLYNK_WRITE(VPIN_HYST)
{
  hysteresis = param.asFloat() / 10.0;
  Serial.print("Blynk hysteresis set: ", param.asInt()); Serial.println(hysteresis);
  
  updateHeatState();
  updateBlueLed();
  updateBlynk();
}

void buttonAction()
{
  Serial.println("Button action!");

  isOn = !isOn;
  updateHeatState();
  updateBlueLed();
  updateBlynkControls();
  updateBlynk();
}

// =============================================
// Functions
// =============================================

void initLeds()
{
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_DISP, OUTPUT);

  digitalWrite(PIN_BLUE,  BLUE_OFF);
  digitalWrite(PIN_RELAY, RELAY_OFF);
  delay(1000);
  digitalWrite(PIN_RELAY, RELAY_ON);
  delay(1000);
  digitalWrite(PIN_BLUE,  BLUE_ON);
  delay(1000);
  digitalWrite(PIN_RELAY, RELAY_OFF);
  delay(1000);
  digitalWrite(PIN_BLUE,  BLUE_OFF);
  delay(1000);
}

void mainLoop()
{
  readSensor();
  updateHeatState();
  updateBlueLed();
  displayState();
  updateBlynk();
}

 // updates: temp, hum, tempIdx, sensorOk
void readSensor()
{
  int attempts = 10
  do {
    Serial.print("Reading temp... ");
#ifdef WITH_DHT
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    sensorOk = !isnan(temp) && !isnan(hum);
    tempIdx = sensorOk ? dht.computeHeatIndex(temp, hum, false) : NAN;
#endif
#ifdef WITH_W1
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    hum = NAN;
    sensorOk = !isnan(temp);
    tempIdx = NAN;
#endif
    attempts--;

    if (sensorOk) {
      Serial.print(" T="); Serial.print(temp);
      Serial.print(" H="); Serial.print(hum);
      Serial.print(" TI="); Serial.println(tempIdx);
    } else {
      Serial.println("Sensor error!");
    }
  } while (!sensorOK && attempts > 0);
}

// updates: heatRequest, PIN_RELAY
void updateHeatState()
{
  if (isOn && sensorOk) {
    if (temp < targetTemp - hysteresis) {
      heatRequest = true;
    } else if (temp > targetTemp + hysteresis) {
      heatRequest = false;
    }
  } else {
    heatRequest = false;
  }

  digitalWrite(PIN_RELAY, heatRequest ? RELAY_ON : RELAY_OFF);

  Serial.printf("Heat request = %d\n", heatRequest);
}

void updateBlueLed()
{
  //digitalWrite(PIN_BLUE, Blynk.connected() ? BLUE_ON : BLUE_OFF);
  bool ledOn = isOn;
  digitalWrite(PIN_BLUE, ledOn ? BLUE_ON : BLUE_OFF);
}

// updates: VPINs
void updateBlynk()
{
  if (sensorOk) {
    Blynk.virtualWrite(VPIN_TEMP_A, temp);
    Blynk.virtualWrite(VPIN_TEMP_B, temp);
    Blynk.virtualWrite(VPIN_HUM_A, hum);
    Blynk.virtualWrite(VPIN_HUM_B, hum);
    Blynk.virtualWrite(VPIN_TIND_A, tempIdx);
    Blynk.virtualWrite(VPIN_TIND_B, tempIdx);
  }
  Blynk.virtualWrite(VPIN_HEAT, heatRequest ? 1 : 0);
  if (heatRequest) {
    heatLed.on();
  } else {
    heatLed.off();
  }
}

// updates: VPINs
void updateBlynkControls()
{
  Blynk.virtualWrite(VPIN_POWER, isOn);
  Blynk.virtualWrite(VPIN_TARGET, targetTemp);
  Blynk.virtualWrite(VPIN_HYST, hysteresis * 10.0);
}

void displayState()
{
  clearDisplay();
  setCursor(0);
  if (sensorOk) {
    char str[10];
    sprintf(str, "%d", (int)(temp * 10));
    displayString(str);
  } else {
    displayString(" -- ");
  }
}

