/*
Filename: Get_Serial_Data.cpp
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 2.0
Date: 04Aug2018
Comments: Derived from Get_Serial_Port
*/

#include "Get_Serial_Data.h"

static unsigned long serialTimeOut = 10000; //msec
unsigned long beginTime = 0;
unsigned long elapsedTime = 0;
static boolean recvInProgress = false;
bool newData = false;
int ndx = 0;
char rc;

// SoftwareSerial serial_B(14, 12, false, 256); //- only for ESP
//SoftwareSerial serial_B(10, 11); // otherwise use stock Arduino

// variables to hold the parsed data
const byte maxNumChars = 250; // sets the longest length of a string to expect
char receivedChars[maxNumChars] = {0};
char tempChars[maxNumChars] = {0}; // temporary array for use when parsing
char messagefromUART[maxNumChars] = {0};
int integerfromUART = 0;
float floatfromUART = 0.0;

// Class version of reading from serial -
String Serial_read::handle(String Serial_port)
{

  String readstring = loop_Serial_Read(Serial_port);
  return readstring;
}

String loop_Serial_Read(String Serial_port)
{
  // DEBUG_MSG("$");
  String data = "";
  messagefromUART[0] = '\0';
  // strcpy(messagefromUART, ""); // clear the message buffer
  tempChars[0] = '\0';
  // strcpy(tempChars, "");       // clear the message buffer
  recvWithStartEndMarkers(Serial_port); // See if UART (hosting device that ESP is connected - the MCC) has any data
  if (newData == true)
  {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    DEBUG_MSG("parseData: \n\r");
    // showParsedData();
    newData = false;
    data = receivedChars;
    data.trim();
    DEBUG_MSG("Here is the data '%s' returned from serial port: %s vs sent to parser: '%s'\n\r", data.c_str(), Serial_port.c_str(), receivedChars);
    receivedChars[0] = '\0'; // reset buffer for next data (null in first position)
  }

  //data = tempChars;
  // data = messagefromUART; // use if want to get a parsed string when
  // expecting 3 parameters of message, integer and float
  // TODO may want to look for and parse to a JSON String instead
  return data;
}

bool Serial_Port_DataAvail(String Serial_port)
{

  if (Serial_port == Host_Port)
  { // set up syntax to use SerialUSB.available()
    return SERIAL_HOST_PORT.available();
  }

  if (Serial_port == Other_Device_Port)
  { // set up syntax to use SerialUSB.available()
    return SERIAL_OTHER_DEVICE_PORT.available();
  }

#ifdef DEBUG_PORT
  if (Serial_port == Debug_Port)
  { // set up syntax to use SerialUSB.available()
    return DEBUG_PORT.available();
  }
#endif

  return 0; // means no data available
}

char Serial_Port_Read(String Serial_port)
{
  // DEBUG_MSG("~");
  if (Serial_port == Host_Port)
  { // set up syntax to use SerialUSB.available()
    return SERIAL_HOST_PORT.read();
  }

  if (Serial_port == Other_Device_Port)
  { // set up syntax to use SerialUSB.available()
    return SERIAL_OTHER_DEVICE_PORT.read();
  }

#ifdef DEBUG_PORT
  if (Serial_port == Debug_Port)
  { // set up syntax to use SerialUSB.available()
    return DEBUG_PORT.read();
  }
#endif

  return ETX; // if none of above just return with the End-of-Text character
}

unsigned long currentTime()
{
  unsigned long cTime;
  cTime = millis();
  return cTime;
}

void recvWithStartEndMarkers(String Serial_port)
{
  // int ndx = 0;
  // char rc;

  // DEBUG_MSG("=");
  while (Serial_Port_DataAvail(Serial_port) && newData == false)
  {

    // newData is set to true when an end character has been set (hopefully
    // after a start character has been received)
    // Note each serial.read has a implied one second time out.
    // DEBUG_MSG("=");
    rc = Serial_Port_Read(Serial_port);
    receivedChars[ndx] = rc;
    // DEBUG_MSG("\n\rFirst character in receive buffer is: %d on reading Data from %s data is:'%d'\n\r",receivedChars[0], Serial_port.c_str(), rc);
    if (rc == startMarker || rc == startMarkerALT || rc == STX)
    {
      // to capture on any input add  ( || rc > 0)  to criteria above
      recvInProgress = true;
      ndx = 0;
      beginTime = currentTime(); // set the time for determining time out
                                 // (serialTimeOut)
      // receivedChars[ndx] = rc;                         //
      DEBUG_MSG("\n\rBegin receiving new string on port %s at time %lu with  '%d' <<\n\r", Serial_port.c_str(), beginTime, rc);
    }
    // Serial.print(rc);
    // DEBUG_MSG("\n\r>>Received>>>Index %d and character %c<<<\n\r",ndx, rc);
    if (recvInProgress == true)
    {
      receivedChars[ndx] = rc;
      // DEBUG_MSG( " Wrote at Index %d Character Value: %d ",ndx, receivedChars[ndx]);
      if ((rc == endMarker || rc == endMarkerALT || rc == ETX || rc == CAN) == false)
      {
        // continue adding characters to packet unless receiving an endMarker,
        // ETX, NL or we've reached a timeout
        //TODO if CAN look for two (or 3) consecutive else ignore
        // DEBUG_MSG("Char added via index: '%c' value: %d at index %d and string is now:%s",   receivedChars[ndx],receivedChars[ndx],ndx, receivedChars );
        ndx++;
        if (ndx >= maxNumChars)
        { // ensure that we don't overflow buffers
          ndx = maxNumChars - 1;
        }
        receivedChars[ndx] = '\0'; // terminate the string - this will get
                                   // overwritten by the next char
                                   // DEBUG_MSG("Rec'd Chars - thus far: '%s' and %d  character count.",receivedChars, ndx);
      }
      else
      { // Must be one of the terminating characters, so end Serial input
        // and process data
        // receivedChars[ndx] = rc; // append terminating character
        receivedChars[ndx + 1] = '\0';
        // append NULL character to denote end of string
        recvInProgress = false;
        newData = true; // this ends the while loop and lets main function
                        // know that there is data
        //elapsedTime = currentTime() - beginTime;

        DEBUG_MSG("\n\rReceived end character at count: %d  with ASCII code '%d'. Input is '%s' "
                  "Elaspsed time for input: %lu\n\r",
                  ndx, rc,
                  receivedChars, currentTime() - beginTime);
        // DEBUG_MSG("string read (index,value,char):\n\r");
        //   for (int i = 0; i < (ndx+1); i++){ DEBUG_MSG("%d, %d, %c\n\r", i, receivedChars[i], receivedChars[i]);}
        // delay(3000);
        ndx = 0;
      }
    }

  } // end  of while Serial.available
  // DEBUG_MSG("recvInProgress=%d, elapsedTime %d \r",recvInProgress, elapsedTime);
  if ((elapsedTime > serialTimeOut) == true)
  {
    receivedChars[0] =
        '\x15';                // set to NAK to indicate that input wasn't completed
    receivedChars[1] = '\x00'; // append NULL character to denote end of string
    // Send to the MCC so that it can try again.
    DEBUG_MSG(
        "\n\rTimeout!  Elaspsed time for input: %lu\n\r",
        currentTime() - beginTime);
    SERIAL_OTHER_DEVICE_PORT.print(receivedChars); //Sending NAK to ESP (Or BSM_LT?) to let source serial device know that send was bad
    ndx = 0;
    newData = true;
    recvInProgress = false;
    elapsedTime = 0;
  }
}

//============

void parseData()
{ // split the data into its parts

  char *strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ","); // get the first part - the string
  strcpy(messagefromUART, strtokIndx); // copy it to messagefromUART

  strtokIndx =
      strtok(NULL, ","); // this continues where the previous call left off
  if (strtokIndx)
  {
    integerfromUART = atoi(strtokIndx); // convert this part to an integer
  }
  else
  {
    integerfromUART = 0;
  }

  strtokIndx = strtok(NULL, ",");
  if (strtokIndx)
  {
    floatfromUART = atof(strtokIndx); // convert this part to a float
  }
  else
  {
    floatfromUART = 0;
  }
}

//============

void showParsedData()
{
  DEBUG_MSG("Message %s \n\r", messagefromUART);
  DEBUG_MSG("Integer %d \n\r", integerfromUART);
  DEBUG_MSG("Float %f \n\r", floatfromUART);
}

 void MQTT_pub_calculated_Topic 
 //TODO: should be 'Topic_from_DeviceName' from deviceName which returns the topic 
 // then just use as MQTT_pub(Topic_from_DeviceName(), mqtt_Pub_message, MQTT_QOS)
(String mqtt_Pub_message){
      LED_MQTT_Message.attach(.25, changeState, Relay_Pin[1]);

      DEBUG_MSG("MQTT pub msg: %s\n\r", mqtt_Pub_message.c_str());
      char mqtt_Pub_serial[MQTT_MAX_MESSAGE_LENGTH];
      // sprintf(mqtt_Pub_serial, " '%s'", mqtt_Pub_message.c_str());
      sprintf(mqtt_Pub_serial, "%s", mqtt_Pub_message.c_str()); // ??? couldn't use mqtt_Pub_message as char ?? 
      
      char topic2Pub[MQTT_MAX_TOPIC_LENGTH] = MQTT_PUB_TOPIC;
      strcat(topic2Pub, "/");
      strcat(topic2Pub, ESPHTTPServer._config.deviceName.c_str());
      MQTT_pub(topic2Pub, mqtt_Pub_serial, MQTT_QOS);
      AP_Gateway_previous_ms = millis(); //start blink time for AP_Gateway activity

      LED_MQTT_Message.detach(); // may not be enough time, but let's see it blinks
    }
