/*
Filename: Read_Sensors.cpp
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 24Dec2019
*/
#include "Read_Sensors.h"

DHT_Unified dht(DHTPIN, DHTTYPE, 11);

uint32_t delayMS = 0;



String sensor_type="";
String LastTemp="---.-";
String LastHumid="--.-";


// unsigned long time;

// prevDisplay = 0; // when the digital clock was displayed






void setup_DHT() {
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  sensor_type=sensor.name;
  DEBUG_MSG("Using DHTxx Unified Sensor: %d  - Output on ESP of %s\r\n", DHTTYPE, sensor_type.c_str());
 
  DEBUG_MSG("------------------------------------\r\n");
  DEBUG_MSG("Minimum Sensor delay: %lu\r\n", (unsigned long)sensor.min_delay);
  DEBUG_MSG("Temperature\r\n");
  DEBUG_MSG("Sensor: \t%s\r\n", sensor.name);
  DEBUG_MSG("Driver Ver: \t%lu\r\n", (unsigned long)sensor.version);
  DEBUG_MSG("Unique ID:\t%lu\r\n", (unsigned long)sensor.sensor_id);
  DEBUG_MSG("Max Value: \t%lu *C\r\n", (unsigned long)sensor.max_value);
  DEBUG_MSG("Min Value: \t%lu *C\r\n", (unsigned long)sensor.min_value);
  DEBUG_MSG("Resolution: \t%lu *C\r\n", (unsigned long)sensor.resolution);
  DEBUG_MSG("------------------------------------\r\n");

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  DEBUG_MSG("------------------------------------\r\n");
  DEBUG_MSG("Humidity\r\n");
  DEBUG_MSG("Sensor: \t%s\r\n", sensor.name);
  DEBUG_MSG("Driver Ver: \t%lu\r\n", (unsigned long)sensor.version);
  DEBUG_MSG("Unique ID:\t%lu\r\n", (unsigned long)sensor.sensor_id);
  DEBUG_MSG("Max Value: \t%lu percent RH\r\n", (unsigned long)sensor.max_value);
  DEBUG_MSG("Min Value: \t%lu percent RH\r\n", (unsigned long)sensor.min_value);
  DEBUG_MSG("Resolution: \t%lu percent RH\r\n", (unsigned long)sensor.resolution);
  DEBUG_MSG("------------------------------------\r\n");


  // DEBUG_MSG("Driver Ver:   \r\n");
  // DEBUG_MSG(sensor.version);
  // DEBUG_MSG("Unique ID:    \r\n");
  // DEBUG_MSG(sensor.sensor_id);
  // DEBUG_MSG("Max Value:    \r\n");
  // DEBUG_MSG(sensor.max_value);
  // DEBUG_MSG("%\r\n");
  // DEBUG_MSG("Min Value:    \r\n");
  // DEBUG_MSG(sensor.min_value);
  // DEBUG_MSG("%\r\n");
  // DEBUG_MSG("Resolution:   \r\n");
  // DEBUG_MSG(sensor.resolution);
  // DEBUG_MSG("%\r\n");
  // DEBUG_MSG("------------------------------------\r\n");

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  // set to one minute = 60000 milliseconds)
  // delayMS = 60000; // TODO: read from the gateway

  DEBUG_MSG(
      "DHT Initialization Complete! Interval (seconds) between readings: %d\r\n", (delayMS / 1000));
}

float getDHT_Temp() {
  float temp_F = 32.0;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  int attempt=0;

  while (isnan(event.temperature) && attempt<10) {
      DEBUG_MSG("Error reading temperature!\r\n");
      digitalWrite(RELAY_2, HIGH);   // toggle lights
      delay(100);                 // wait for a second
      digitalWrite(RELAY_2, LOW); // toggle lights
      delay(100);
      dht.temperature().getEvent(&event);
      attempt++;
      delay(100);
    }
  digitalWrite(RELAY_2, LOW);  //turn the LED off by making the voltage LOW
  delay(100);                  // wait for a second
  digitalWrite(RELAY_2, HIGH); // turn the LED off by making the voltage LOW
  delay(100);                  // wait for a second
  digitalWrite(RELAY_2, LOW);  // turn the LED on (HIGH is the voltage level)
  delay(100);                  // wait for a second
  digitalWrite(RELAY_2, HIGH); // turn the LED off by making the voltage LOW

  if ( !isnan(event.temperature) ) { temp_F = event.temperature * 9 / 5 + 32; }
  DEBUG_MSG("Temperature: \t%2.1f *C\t%3.1f *F\r\n", event.temperature, temp_F);
  return (temp_F);
}

float getDHT_Humid() {
  float humid_RH;
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    
      DEBUG_MSG("Error reading humidity!\r\n");
      digitalWrite(RELAY_2,
                   HIGH);         // turn the LED on (HIGH is the voltage level)
      delay(100);                 // wait for a second
      digitalWrite(RELAY_2, LOW); // turn the LED off by making the voltage LOW
      delay(100);                 // wait for a second
      digitalWrite(RELAY_2,
                   HIGH); // turn the LED on (HIGH is the voltage level)
    
    humid_RH = 0;
  } else {
    digitalWrite(RELAY_2, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(100);                  // wait for a second
    digitalWrite(RELAY_2, HIGH); // turn the LED off by making the voltage LOW
    delay(1000);                 // wait for a second
    digitalWrite(RELAY_2, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(100);                  // wait for a second
    humid_RH = event.relative_humidity;
  }

  DEBUG_MSG("Humidity: \t%3.1f percent RH\r\n", humid_RH);

  return (humid_RH);
}

String DHT_Read_JSON() {
  // returns JSON of RID, UT, Temperature and Humidity 
  
  // Format JSON header with RID, Sensor Type and UT
  String Mqtt_Message;

  Mqtt_Message = L_Squig_Bracket + "\"RID\":\"" + Read_Mac_Address_s().c_str() + "\""; // RadioID equiv of Read_Mac_Address_s
  Mqtt_Message += ",\"S\":\"";
  Mqtt_Message += sensor_type + "\"";
  // Mqtt_Message += sensor.name + "\"";
  int UpTime = millis() / 1000;
  Mqtt_Message += ",\"UT\":" + L_Squig_Bracket + "\"V\":\"" +
                  String(UpTime) + "\",\"U\":\"Sec\"" + R_Squig_Bracket;
  DEBUG_MSG("MQTT Message Read: %s\n\r", Mqtt_Message.c_str());
  // Get temperature part of JSON.
  // try up to x times to get a temperature not equal to 32
  int get_valid_temp_counter = 0;
  String Valid_Temperature = String(getDHT_Temp(), 1);
  while (Valid_Temperature == "32.0" and get_valid_temp_counter < 3) {
    Valid_Temperature = String(getDHT_Temp(), 1);
    get_valid_temp_counter++;
  }
  String Temperature = ",\"T\":" + L_Squig_Bracket + "\"V\":\"" + Valid_Temperature +
                "\",\"U\":\"F\"" + R_Squig_Bracket;
  // Get humidity part of JSON.
  String Humidity = ",\"H\":" + L_Squig_Bracket + "\"V\":\"" +
             String(getDHT_Humid(), 1) + "\",\"U\":\"%RH\"" + R_Squig_Bracket;
  // Complete JSON
  Mqtt_Message += Temperature + Humidity +
                  R_Squig_Bracket; // complete and close off the array
  char Buffer[150];                  
  Mqtt_Message.toCharArray(Buffer, Mqtt_Message.length() + 1);
  return Mqtt_Message;
}




