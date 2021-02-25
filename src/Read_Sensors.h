/*
Filename: Read_Sensors.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 24Dec2019
*/


#ifndef READ_SENSORS_H
#define READ_SENSORS_H

#include "main.h"



// Depends on the following Arduino libraries:
//#endif

// - Adafruit Unified Sensor Library:
// https://github.com/adafruit/Adafruit_Sensor
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>



// Declaration (definition in .cpp)
void setup_DHT();
float getDHT_Humid();
float getDHT_Temp();
String DHT_Read_JSON();

//variables defined in this module 
extern uint32_t delayMS;

extern char LastPubReading[150]; // used to compare last published reading
// extern String Temperature;
// extern String Humidity;
extern String LastTemp;
extern String LastHumid;
// extern String Mqtt_Message ;
extern String sensor_type;

// extern DHT_Unified dht(uint8_t, uint8_t, uint8_t);



// 
// variables used by this module 
extern long TimeRefresh4NIST;
extern String R_Squig_Bracket;
extern String L_Squig_Bracket;
// extern char Buffer[150]; 

#endif