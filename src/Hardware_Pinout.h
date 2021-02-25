/*
Filename: Hardware_Pinout.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.1
Date: 31May2018
Last_Update: 27Nov2018
*/
#ifndef Hardware_Pinout_H
#define Hardware_Pinout_H

//TODO use different definitions depending on platformio.ini env: variable
// NOTE: ESP power requirements are 3.3V±10%@350mA plus LED on requirements (~20ma * 5 = 100ma) - could be .5 amp max
// Uncomment the type of sensor in use: (Read_Sensors.h)
//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)


// GPIO definitions        GPIO#       Function
#define Analog A0    // Analog max 3.3v
#define HEARTBEAT 16 // used LED (D0/GPIO16) or LED_BUILTIN is on D4/gpio02
#define Sensor2Gnd 5 // CycleTimeRel (GPIO14)
#define DHTPIN 4     // DHT sensor
#define NUM_RELAYS 5 // used to set up array for relays - only use first 2

    // NOTE: RELAY_GPIO#'s are defined in platformio.ini build
// #define RELAY_0 0 // Relay Green (14)
// #define RELAY_1 2 // Relay Yellow (12)
// #define RELAY_2 14 // Relay Red 
    // Note/Caution: using GPIO1 overrides U0TXD - so won't be able to use serialTX)

//#define SwapTX            13       // secondary serial TX
//#define SwapTX            16      // secondary serial TX

extern const int Status_LED;

extern int Relay_Pin[];
extern int Relay_State[];
/*
 * Pin mapping for ESP8266 WeMOS D1 Mini
 * Note: GPIO# is the pin assignment # to use when assigning name/function to a pin above
Pin Function                      ESP-8266 Pin/Signal
TX  TXD                           TXD GPIO1
RX  RXD                           RXD GPIO3
A0  Analog input, max 3.3V input  A0
D0  IO                            GPIO16
D1  IO, SCL                       GPIO5
D2  IO, SDA                       GPIO4
D3  IO, 10k Pull-up               GPIO0
D4  IO, 10k Pull-up, BUILTIN_LED  GPIO2
D5  IO, SCK  (aka: CLK)           GPIO14
D6  IO, MISO (aka: CS)            GPIO12
D7  IO, MOSI (aka: DIN)           GPIO13
D8  IO, 10k Pull-down, SS         GPIO15
G   Ground                        GND
5V  5V/Vin                        N/C on ESP8266
3V3 3.3V  regulated RT9013-33     3.3V
RST Reset                         RST
*/
/*
 * NodeMCU  pin mapping.
Pin numbers written on the board itself do not correspond to
ESP8266 GPIO pin numbers.

D numbers on the board      GPIO
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/

#endif
