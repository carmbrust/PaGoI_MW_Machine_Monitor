/*
Project: Machine Monitor WiFi (MILwright_FSBrowser_AP_Config)
Filename: main.h
Client: PaGoI  .
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Date: 08Feb2018, 31May2018
Version: .98 (note: change  APP_VERSION in defaults.h)

Purpose of program:
1. Autojoin WiFi network with store SSID/password
2. If WiFi network is not available, start AP_Mode and allow HTTP access to:
  - define network configuration
  - MCC_BSM setup and calibration
3. In normal mode:
  - read WRU data of connected BSM
  - monitor and periodic report of the MCC_BSM or BSM_LT
  - support configuration of:
        - network parameters
        - BSM_LT battery mfg and serial number
  - network monitoring of the modules

TODO list
- Update Get_Serial to be a class that can be called from many modules (aync?)
- MCC_BSM_read - get "all" of the common/defined statis parameters
  - support a WRU macro
- MCC_BSM_write - set/write to the MCC_BSM

Changelog:
20180531:
        - update Get_Serial_Port to support the port as a variable (defined in
defautlts)
        -
20180308:
  - added special prefix for ESP directed commands (e.g. * - means to ESP, *Rrs
is to relay r with state s (0/1), *? is the WRU for the ESP) 20170818:
  - added header structure to main.h
  - support read/write of call to SPIFFS from main loop


*/
#ifndef main_H
#define main_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
// #include "Arduino.h"
#endif

// // std::string functions
// #include <iostream>
// using namespace std;
#include <stdio.h>  /* printf, */
#include <stdlib.h> /* atof */

#if USE_SW_SER
#include <SoftwareSerial.h>
#endif

// local configuration files
#include "debug.h"
#include "defaults.h"
#include "Hardware_Pinout.h"

// load external library dependencies first

#include <TimeLib.h>

#include <string>
#include <ArduinoJson.h>
#include <FS.h>
#include <Ticker.h>

#include <NtpClientLib.h>


#ifdef ESP32
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <ESP8266HTTPClient.h> // also included in ESP8266httpUpdate.h
#include <ESP8266httpUpdate.h> 
#include <Ticker.h>
#endif

#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>


#include <ArduinoOTA.h>
// #include <ESP8266WiFi.h>
#include <WiFiClient.h>  // 
// #include <ESP8266WiFi.h> // declared in modules that use it - MD_FSWebServerLib and MQTT_AsyncClient. Also, included in platform libraries so shouldn't need to be included
// #include <ESPAsyncTCP.h>
// #include <AsyncTCP.h>





// load the application specific modules
// #include "MQTT_AsyncClient.h" // why is this commented out? 
// #include "Timelib.h" // this should load TimeLib which points to _time.h instead of Time.h which will case ESPAsyncWebServert to generate messages: gmtime and time not declared in this scope
#include <AsyncMqttClient.h> // (http://platformio.org/lib/show/346/AsyncMqttClient) built on [me-no-dev/ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
// AsyncMqttClient mqttClient;
#include "MD_FSWebServerLib.h"

#include "Read_Sensors.h"
#include "Sensor2Gnd.h"
#include "Get_Serial_Data.h"

// #include <ESPmDNS.h>


// #include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <TimeLib.h>
// #include <NtpClientLib.h>
// #include <ESPAsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <ESP8266mDNS.h>
// #include <FS.h>
// #include <Ticker.h>
// #include <ArduinoOTA.h>
// #include <ArduinoJson.h>


//


// Declarations for functions used by this module:
String loop_Serial_Read(String Serial_port);
void MQTT_Async_setup();
void checkForUpdates();
// extern String   Get_Serial::loop_Serial_Read(); // read from serial port -
// USB extern void setup_OTA(); extern void loop_OTA();

// Declarations for functions defined in this module : 
void flashLED(int pin, int times, unsigned long delayTime);
String IpAddress2String(IPAddress address);
String Read_Mac_Address_s();
String calculated_Hostname();
string digitalClockDisplay(int UTS, int time_offset_hrs_10x, int daylight);
bool MQTT_pub(char *topic, char *Message, uint8_t qos);
// void OTA_Setup(String password);
const char* wl_status_to_string(wl_status_t status);
const char *wifi_mode_to_string(WiFiMode_t mode);
void changeState(int LED);

extern char Buffer[150];

// Variables used in this module and defined elsewhere: 
extern String R_Squig_Bracket;
extern String L_Squig_Bracket;


// variables used by module Sensor2Gnd.h
extern int Sensor2Gnd_state;      // True is open, False is closed
extern int last_Sensor2Gnd_state; // True is open, False is closed
extern unsigned long lastDebounceTime; 
// extern  long debounceDelay;  
extern int intervalMillis_Cycle;
extern int lastMsg;  
extern float CycleTime;
// extern struct strConfig _config; 
extern int64_t millis64();  // used to get uptime where uptime can be > 49.7 days

// functions defined in module Sensor2Gnd.h
extern String Sensor2Gnd_Loop();

// Variables defined in this module (optional but nice to help identify where they came from)
extern unsigned long start_time;
extern unsigned long Client_previous_ms;
extern string buildtimestamp_s;
extern String host_Name;
extern String localIP_s;
extern String host_Name;
extern String localIP_s;
// bool Heartbeat_state; // defined in main.cpp
extern String MQTT_pub_topic; //ISSUE: this may need to be either char, char* or string (i.e. c++ string not arduino String)
extern String MQTT_sub_topic; //ISSUE:  this may need to be either char, char* or string (i.e. c++ string not arduino String)
extern vector<string> mqtt_SUB_topics_array; // is the list of the list topics
extern unsigned long AP_Gateway_previous_ms;
// extern uint32_t delayMS; // delay between temp/humid readings
// DHT related items
extern const int Status_LED;

// ISSUE: review sub topics to ensure not doing a loop
// MQTT_pub() is using char* and yet the above naming wants to use string - go one way or another

// From main.h will be using these functions/modules and methods are defined in
// FSWebServerLib.h extern String getMac_address_s; extern struct _config; extern

// Declaration for an object - defined elsewhere, but used here 

// extern strConfig _config; 
// Ticker timers
extern Ticker LED_Upload;
extern Ticker LED_StationModeConnected;
extern Ticker LED_SoftAPModeConnected;
extern Ticker LED_Webserver;
extern Ticker LED_MQTT_Message;
extern Ticker LED_Booting;
extern Ticker LED_MQTT_connected_to_broker;
extern Ticker Firmware_Update_Check;

// extern elapsedMillis timeElapsed;

#endif
