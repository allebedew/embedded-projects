
#define BLYNK_USE_DIRECT_CONNECT

//#define BLYNK_PRINT Serial

#include <BlynkSimpleSerialBLE.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"
#include <SPI.h>

char auth[] = "761e9fd52f1b4376b51cbec74938ab44";

#define BLE_REQ   6
#define BLE_RDY   7
#define BLE_RST   4

#define LF_PIN    3
#define LB_PIN    5
#define RF_PIN    9
#define RB_PIN    10

BLESerial SerialBLE(BLE_REQ, BLE_RDY, BLE_RST);

void setup() {
  Serial.begin(9600);

  SerialBLE.setLocalName("Blynk");
  SerialBLE.setDeviceName("Blynk");
  SerialBLE.setAppearance(0x0080);
  SerialBLE.begin();

  Blynk.begin(SerialBLE, auth);

  Serial.println("Waiting for connections...");
}

int power, l, r, lf, lb, rf, rb;

unsigned long long last_upd = 0;

void loop() {
  SerialBLE.poll();

  if (SerialBLE) {
    Blynk.run();
  }

  if (millis() - last_upd > 200) {
    Blynk.virtualWrite(V2, lf);
    Blynk.virtualWrite(V3, lb);
    Blynk.virtualWrite(V4, rf);
    Blynk.virtualWrite(V5, rb);

    last_upd = millis();
  }
}

BLYNK_WRITE(V1) {
  int x = param[0].asInt() - 128;
  int y = param[1].asInt() - 128;

  power = map(abs(y), 0, 128, 0, 255);
  l = map(max(0, x), 128, 0, 0, power);
  r = map(abs(min(0, x)), 128, 0, 0, power);

  lf = y > 0 ? r : 0;
  lb = y < 0 ? r : 0;
  rf = y > 0 ? l : 0;
  rb = y < 0 ? l : 0;     

  analogWrite(LF_PIN, lf);
  analogWrite(LB_PIN, lb);
  analogWrite(RF_PIN, rf);
  analogWrite(RB_PIN, rb);

//   Serial.print("Joy x=");
//   Serial.print(x);
//   Serial.print(" y=");
//   Serial.print(y);
//   
//   Serial.print(" *** L=");
//   Serial.print(l);
//   Serial.print(" R=");
//   Serial.print(r);
//   
//   Serial.print(" *** LF=");
//   Serial.print(lf);
//   Serial.print(" LB=");
//   Serial.print(lb);
//
//   Serial.print(" *** RF=");
//   Serial.print(rf);
//   Serial.print(" RB=");
//   Serial.println(rb);
}
