
#include <SoftwareSerial.h>

#define PIN_TX    4
#define PIN_RX    7

SoftwareSerial sserial(PIN_RX, PIN_TX);

unsigned int counter = 0;
char tempString[10];

//void setup()
//{
//  
//  
//  

//  setDecimals(0b111111);
//  
////  delay(1500);
////  setBrightness(127);
////  delay(1500);
////  setBrightness(255);
//  delay(1500);
//  clearDisplay();  
//}

//void loop()
//{
//
//}

void setupDisplay()
{
  sserial.begin(9600);
  clearDisplay();
  setBrightness(255);
  sserial.print("oOoO");
}

void setCursor(byte digit)
{
  sserial.write(0x79);
  sserial.write(digit);
}

void clearDisplay()
{
  sserial.write(0x76);
}

void setBrightness(byte value)
{
  sserial.write(0x7A);
  sserial.write(value);
}

void setDecimals(byte decimals)
{
  sserial.write(0x77);
  sserial.write(decimals);
}

void setDigit(byte digit, byte value)
{
  if (digit >= 0 && digit <= 3) {
    sserial.write(0x7B + digit);
    sserial.write(value);
  }
}

void displayString(char string[])
{
  sserial.print(string);
  Serial.print("Printed: ");
  Serial.println(string);
}

