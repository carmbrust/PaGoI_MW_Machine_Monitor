/*
Filename: Get_Serial_Data.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 2.1
Date: 04Aug2018
Revision: 20191230 
Comments: Derived from Get_Serial_Port
TODO Make a library which means eliminating/minimizing external dependencies
*/
#ifndef Get_Serial_Data_H
#define Get_Serial_Data_H

// because this is now a library, the main should already be loaded.
// <But:> look for dependencies in main</But:>
#include "main.h"

/* Goal for this modules
1. Read from Serial and report back as return string
2. Don't block other processes so allow processing of a character at a time
3. Only report back on encapsulated messages (startMarker and endMarker)
4. Use DEBUG_PORT for output of any debugging messages
5. Allow specification of more than one read port.
1. Workaround: duplicate this module and make one for Serial_ESP_Port and DEBUG_PORT
 	- Only has to read from a few ports: Serial, Serial1, Serial2 or maybe from SoftwareSerial or from I2c
		- only 2-3 places where the port is used, so could use case or if-else logic to get to the desired ports
		- and if use (Stream &port) could
    if port=0 then readPort (Serial)
    if port=1 then readPort(Serial1)
    string readPort(Stream &port)
    {while (port.avail)}

*/

//TODO - DONE
// 1. Change to add a class where Serial_read.handle() returns string read from serial
// (consider CmdMessenger - https://github.com/thijse/Arduino-CmdMessenger as example) - overkill in wrong direction
//
/* TODO address the actual port pointer to use each of the hardware serial objects.
// Question: how to use SoftwareSerial instead to make more general
// Question: Could Stream be used as the totally generic way?
HardwareSerial *port;
if (something == somethingElse)
    port = &Serial;
else
    port = &Serial1;

byte b = 5;
port->write(b);
*/

#ifdef USE_SW_SER
    extern SoftwareSerial Serial_B;
 #endif

  extern String Host_Port; // port of the USB serial interface
  extern String Other_Device_Port; // port of the ESP serial interface
  extern String Debug_Port;


class Serial_read
{ public:
 String handle(String port);// allows for the return of the String data read from the serial port
};
//  do stuff; happens in Get_Serial_Data.cpp
//
//
//TODO use structure/technique to Allow listeners or handlers for:
//  - WRU type messages
//  - MQTT message composition for publishing messages


String   loop_Serial_Read(String port); // read from serial port - USB
// what if loop_Serial_Read(UART_Port);
// String   loop_serial_Bial_Read(); // read from serial port - SoftwareSerial

void recvWithStartEndMarkers(String port);
void parseData();
void showParsedData();
void MQTT_pub_calculated_Topic
(String);

// Variables used in this module and defined elsewhere: 
// extern String R_Squig_Bracket;
// extern String L_Squig_Bracket;

#endif
