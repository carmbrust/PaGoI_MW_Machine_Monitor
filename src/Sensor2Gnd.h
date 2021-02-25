/*
Filename: Sensor2Gnd.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 29Dec2019
*/


#ifndef SENSOR2GND_H
#define SENSOR2GND_H

#include "main.h"
#include <elapsedMillis.h>

// Depends on the following Arduino libraries:

// variables used by this module
// int Sensor2Gnd_state;      // 1 (True) is open, 0 (False) is closed
// int last_Sensor2Gnd_state; // 1 (True) is open, (False) is closed
extern unsigned long lastDebounceTime; 
// extern  long debounceDelay;  
// extern struct strConfig _config;  // allows this module to reference _config.debounceDelay
extern int intervalMillis_Cycle;
extern int lastMsg;  

extern String R_Squig_Bracket;
extern String L_Squig_Bracket;

extern elapsedMillis timeElapsed;

extern float CycleTime;
// extern String nist_time;
// extern char Buffer[150]; 

// Declare functions defined in this module:
String Sensor2Gnd_Loop();
int64_t millis64();

#endif