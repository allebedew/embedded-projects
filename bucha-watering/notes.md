static const uint8_t D0   = 16;
static const uint8_t D1   = 5; SCL
static const uint8_t D2   = 4; SDA
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

ESP:
http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html

dsip model:
1602c
https://www.openhacks.com/uploadsproductos/eone-1602a1.pdf

expander:
PCF8574T
I2C 0x3F

-30 dBm	Amazing	Max achievable signal strength. The client can only be a few feet from the AP to achieve this. Not typical or desirable in the real world
-67 dBm	Very Good	Minimum signal strength for applications that require very reliable, timely delivery of data packets.	VoIP/VoWiFi, streaming video
-70 dBm	Okay	Minimum signal strength for reliable packet delivery.	Email, web
-80 dBm	Not Good	Minimum signal strength for basic connectivity. Packet delivery may be unreliable.	N/A
-90 dBm	Unusable