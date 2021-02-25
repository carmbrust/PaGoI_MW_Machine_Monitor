/*
Filename: Hardware_Pinout.cpp
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 31May2018
*/
#include "Hardware_Pinout.h"



const int Status_LED = HEARTBEAT; // default pin# for LED
/*
Define relationship between Relay/LED/Indicator and the GPIO#/specific pin

*/
int Relay_Pin[] = {RELAY_0, RELAY_1, RELAY_2, RELAY_3, LED5}; // for relays 0, 1, 2 (Green, Yellow, Red, Blue, Orange)
// int Relay_State[NUM_RELAYS] = {0, 0, 0, 0, 0};// initial open or closed state of each of the relays
int Relay_State[] = {0, 0, 0, 0, 0};// initial open or closed state of each of the relays