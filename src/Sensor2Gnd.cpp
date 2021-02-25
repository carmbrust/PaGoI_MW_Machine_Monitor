/*
Filename: Sensor2Gnd.cpp
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 29Dec2019
*/
#include "Sensor2Gnd.h"


int Sensor2Gnd_state = 1;      // True is open, False is closed
int last_Sensor2Gnd_state = 0; // True is open, False is closed
float CycleTime = 0.00;
// // <NOTE:>These values are or can be overwritten by the values contained in the config file on SPIFFS (See load_config() )</NOTE:>
//
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
// unsigned  Relay2Gnd_debounceDelay;
// Relay2Gnd_debounceDelay = _config.debounceDelay;  // in msec - the debounce time; increase if the output
                                    // flickers, No press should have CT <4 seconds
                                    // but, some presses only hold the relay a very short time. E.G. <100 ms


int64_t millis64()
{
  // Handling long durations (e.g. uptime of the ESP/BSM)
  // if just use millis(), the UpTime will rollover every 49.7 days
  //  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
  static uint32_t low32, high32;
  uint32_t new_low32 = millis();
  if (new_low32 < low32)
    high32++;
  low32 = new_low32;
  return (uint64_t)high32 << 32 | low32;
}

String Sensor2Gnd_Loop(){
  // Check Sensor2Gnd state, create Mqtt_Message if state changed with CycleTime <<<<<<<<<<<<<<<
  // DEBUG_MSG("Sensor2Gnd State: %d\r", digitalRead(Sensor2Gnd));
  String Mqtt_Message="";
  int Sensor2Gnd_state_JustRead = digitalRead(Sensor2Gnd);
  // If the switch changed, due to noise or pressing or relay closure to ground:
  if (Sensor2Gnd_state_JustRead != last_Sensor2Gnd_state)
  {
    // reset the debouncing timer whenever there is a state change (e.g., press
    // or release)
    lastDebounceTime = millis();
    DEBUG_MSG("Sensor2Gnd change detected at: %lu\n\r", lastDebounceTime);
  }

  if ((millis() - lastDebounceTime) > ESPHTTPServer._config.debounceDelay)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
#if DEBUG == true
    if (Sensor2Gnd_state_JustRead != last_Sensor2Gnd_state)
    {
      Mqtt_Message = "Switch state is: " + String(Sensor2Gnd_state) +
                     " Current UT= " + String(millis() / 1000) +
                     " debounceDelay = " + String(ESPHTTPServer._config.debounceDelay);

      MQTT_pub_calculated_Topic(Mqtt_Message);
      DEBUG_MSG("MQTT Message: ''%s'\n\r ", Mqtt_Message.c_str());
    }
#endif
    // if the button state has changed:
    if (Sensor2Gnd_state_JustRead != Sensor2Gnd_state)
    {
      Sensor2Gnd_state = Sensor2Gnd_state_JustRead;

      if (Sensor2Gnd_state == 0)
      {
        // CycleTime = float(timeElapsed); // timeElapsed is in msec for this loop
        CycleTime = float(timeElapsed) / 1000;
        DEBUG_MSG("timeElapsed in msec:%d and the CycleTime: %6.2f ",
                  int(timeElapsed), CycleTime);
        //---
        // String L_Squig_Bracket = "\u007B";
        // String R_Squig_Bracket = "\u007D";
        Mqtt_Message = L_Squig_Bracket + "\"RID\":\"" + Read_Mac_Address_s() + "\"";
        Mqtt_Message += ",\"CT\":" + L_Squig_Bracket + "\"V\":\"" +
                        String(CycleTime) + "\",\"U\":\"Sec\"" +
                        R_Squig_Bracket;
        int UpTime = millis64() / 1000; //with UpTime as int, this will get just the seconds and no fractional part (which is good)
        Mqtt_Message += ",\"UT\":" + L_Squig_Bracket + "\"V\":\"" +
                        String(UpTime) + "\",\"U\":\"Sec\"" +
                        R_Squig_Bracket;
        DEBUG_MSG("MQTT Message: ''%s'\n\r ", Mqtt_Message.c_str());
        // Complete JSON
        Mqtt_Message += R_Squig_Bracket; // complete and close off the array
        char Buffer[150];
        Mqtt_Message.toCharArray(Buffer, Mqtt_Message.length() + 1);

        //---
        // Mqtt_Message =
        //     "Contact to ground detected. CycleTime= " + String(CycleTime);
        
        DEBUG_MSG("MQTT Message: ''%s'\n\r ", Mqtt_Message.c_str());
        timeElapsed = 0;
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the reference:
  last_Sensor2Gnd_state = Sensor2Gnd_state_JustRead;
  // LED is on when state is 0 (closed), LED is off when 1 - (open)
  digitalWrite(RELAY_0, !last_Sensor2Gnd_state);
  // END of Check Sensor2Gnd state>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  return Mqtt_Message;
}

