#include "MQTT_AsyncClient.h"

// Declaring Objects - Instantiate Objects (ClassName ObjectName;) = an instance of the class.
// The data members and member functions of class can be accessed using the dot(‘.’) operator with the object (e.g. obj.data_member)
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

long MQTT_lastReconnectAttempt = 0;
// String buildtimestamp_s= digitalClockDisplay( BUILD_TIMESTAMP);

void connectToMqtt()
{
  DEBUG_MSG("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  DEBUG_MSG("Connected to Wi-Fi - onWifiConnect. So, go re-setup MQTT.\n\r");
  localIP_s = IpAddress2String(WiFi.localIP()); // update IP address
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  DEBUG_MSG("MQTT onWifiDisconnect.\n");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  // wifiReconnectTimer.once(2, connectToWifi);

  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  // mqttReconnectTimer.detach();
  // wifiReconnectTimer.once(2, connectToWifi);// note: by default, ESP8266 will attempt to automatically reconnect
  // connectToWifi might need to point to configureWifiAP
}

void onMqttConnect(bool sessionPresent)
{
  DEBUG_MSG("** Connected to the broker **\n\r");
  DEBUG_MSG("Session present: %d\n\r", sessionPresent);
  // Goal: (re)Establish the subscriptions. Also announce presense (i.e.
  // on-line)
  uint16_t packetIdSub;

  MQTT_Status_msg();
  // use char topic2Pub getting value defined in the main.cpp as String MQTT_pub_topic
  char topic2Pub[MQTT_MAX_TOPIC_LENGTH];
  // topic2Pub[0] = "\0";
  strcpy(topic2Pub, MQTT_pub_topic.c_str());
  // strcat(topic2Pub, host_Name.c_str());
  DEBUG_MSG("Publishing to topic: '%s' with message: '%s' and QoS of: %d\n\r", MQTT_pub_topic.c_str(), mqtt_announce, MQTT_QOS);
  DEBUG_MSG("Subscribing to topic: '%s' \n\r", MQTT_sub_topic.c_str());

  if (!MQTT_pub(topic2Pub, mqtt_announce, MQTT_QOS))
  {
    DEBUG_MSG("Try to connect again?");
  }

  DEBUG_MSG("Subscribing to these topics: \n\r");
  int mqtt_qos = MQTT_QOS;
  char mqtt_subtopic[MQTT_MAX_TOPIC_LENGTH];

  for (size_t i = 0; i < mqtt_SUB_topics_array.size(); i++)
  {
    strcpy(mqtt_subtopic, mqtt_SUB_topics_array[i].c_str());
    packetIdSub = mqttClient.subscribe(mqtt_subtopic, mqtt_qos);
    DEBUG_MSG("Topic: %d is: '%s',  QoS %d, packetId: %d.\n\r", i,
              mqtt_SUB_topics_array[i].c_str(), mqtt_qos, packetIdSub);
  }

  DEBUG_MSG("Expecting acknowledgments with packetID incrementing)\n\rEnd of list\n\r");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  DEBUG_MSG("** Disconnected from the broker **\n\r");
  DEBUG_MSG("Reconnecting to MQTT...");
  // mqttClient.connect();
  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(2, connectToMqtt);

  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  DEBUG_MSG("** Subscribe acknowledged ** for packetId: %d  qos: %d\n\r",
            packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
  DEBUG_MSG("** Unsubscribe acknowledged **  ");
  DEBUG_MSG(" for packetId: %d \n\r", packetId);
}

void onMqttMessage(char *topic, char *payload,
                   AsyncMqttClientMessageProperties properties, size_t len,
                   size_t index, size_t total)
{
  digitalWrite(LED4, 1);
  // std::string s_payload(raw_payload);
  // payload[len] = '\0'; //NUL character to indicate end of String in the array - array is zero indexed << this causes ESP to crash
  // TODO: update so that uses a new buffer - this might solve the Soft WDT Reset error also
  //         char msg[length+1]; strcpy(msg,(char*)payload); msg[length] = 0;
  DEBUG_MSG("\n\r** MQTT message received ** \n\r");
  DEBUG_MSG("  topic: %s \n\r", topic);
  // DEBUG_MSG("  qos - quality of service: %d\n\r", properties.qos);
  // DEBUG_MSG("  duplicate: %d\n\r", properties.dup);
  // DEBUG_MSG("  retain: %d\n\r", properties.retain);
  DEBUG_MSG("  message (payload)\n\r>>%s<<\n\r", payload);
  DEBUG_MSG("  length of message: %d\n\r", len);
  // DEBUG_MSG("  index: %d\n\r", index);
  // DEBUG_MSG("  total: %d\n\r", total);
  /* clean up payload
    1. ensure there is NUll termination
    2. remove redundant STX and [ characters 
    3. if no delimiters (i.e. start, end character, exclamation point!), then don't even consider to send to BSM/MCC. Assume this is for ESP.
        - recognized ESP commands: 
          - "*?" - who are you 
          - "**reboot" - reboot
          - "*OTA" - look for ota
          = "*R" relay command
  */

  // clean up input for delimiters
  // TODO - use regex for more generalized/robust processing (https://en.cppreference.com/w/cpp/regex/regex_match)
  //   (!{0,})([[<\x01]+)([a-zA-Z.:*0-9?\/!]+)([]>\x03]+)
  // This will pick off two consecutive characters
  if (((payload[0] == STX || payload[0] == startMarkerALT || payload[0] == startMarker)) && (payload[1] == STX || payload[1] == startMarkerALT || payload[1] == startMarker))
  {
    DEBUG_MSG("Trimming payload...");
    for (size_t i = 0; i < len; i++)
    {
      payload[i] = payload[i + 1];
    }
    len = len - 1;
    DEBUG_MSG("...trim done.\n\r");
  }
  // payload[len] = 0x0;
  DEBUG_MSG("FreeHeap:%d \n\r", ESP.getFreeHeap());
  // DEBUG_MSG(", Topic: '%s'", topic);
  // DEBUG_MSG(", Payload length %u", len);
  // DEBUG_MSG(", Trimmed payload MSG: '%s' \n\r", payload);
  // // DEBUG_MSG(", reported strlen MSG: '%d' \n\r", strlen(payload));

  // // for no good reason - this should fix crashes
  // if ((len % 4) == 2)
  // {
  //   payload[len] = endMarker;
  //   len = len + 1;
  //   payload[len] = '\0';
  //   DEBUG_MSG("Adjusted payload MSG: '%s' \n\r", payload);
  // }

  if (properties.dup == 1)
  { // skip duplicates for now
    DEBUG_MSG("Skipping duplicate message.\n\r");
    return;
  }

  // Process messages directed to this device
  DEBUG_MSG("This device's MQTT_sub_topic: '%s' and topic is '%s'\n\r", MQTT_sub_topic.c_str(), topic);
  // DEBUG_MSG(" full topic length: %d, ", strlen(MQTT_sub_topic).c_str());
  // MQTT_sub_topic is Sensor/to_device/MILwright-D0D2/# << don't want the /# in the compare
  size_t MQTT_sub_topic_base_length = strlen(MQTT_sub_topic.c_str()) - 2;
  DEBUG_MSG("base_length: %u and topic length: '%u'\n\r", MQTT_sub_topic_base_length, strlen(topic));
  DEBUG_MSG("Is this msg for everyone (are we special)? %u\n\r", strncmp(topic, MQTT_sub_topic.c_str(), MQTT_sub_topic_base_length));
  // Check if topic is for this specific device?

  if (strncmp(topic, MQTT_sub_topic.c_str(), MQTT_sub_topic_base_length) == 0) // This message is for us: e.g Sensor/to_device/MILwright-D0D2/
  {
    DEBUG_MSG("This one is addressed to 'us' specifically! Payload (message): '%s' \n\r", payload);
    DEBUG_MSG(" with length %u\n\r", len);

    //   //TODO Request for OTA in topic? If so, use the message as the URL of the site hosting the update
    //   if (strstr(topic, MQTT_OTA_TOPIC_MODIFIER) && (strstr(topic, MQTT_LISTEN_TOPIC))) // OTA (e.g Sensor/to_device/MILwright-D0D2/ota)
    //   {
    //     DEBUG_MSG("\nStarting connection to OTA server %s...\n", OTA_SERVER);

    //     DEBUG_MSG("WiFi status: %s \n", wl_status_to_string(WiFi.status()));
    //     if (WiFi.status() == !WL_CONNECTED)
    //     {
    //       DEBUG_MSG("Not connected so can't process this request.\n\r");
    //     }
    //     else if (WiFi.status() == WL_CONNECTED)
    //     {
    //       String url = payload;
    //       checkForUpdates(url);
    //       /*
    //        https://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#implementation-overview
    //        https://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#http-server
    //       */
    //     }
    //     return;
    //   }

    if ((char)payload[0] == '*' || (char)payload[1] == '*') // interpret the "*" as a message directed to the ESP for processing
    {
      /* Command is directed to the ESP if starts with "*"
  	Look for a special character * | # | @ in the first or second position
                  otherwise expect that the command should be sent to the Serial port (e.g. MCC | BSM)
                  NOTE that characters _ | ! are already used by the MCC
                  NOTE: payload includes the <stx> and <etx> and equivalent characters (e.g. [])
          Given: input can either be direct (from a mosquitto_pub or from the debug dashboard). So, payload could be
                either "message" or "[message]"
  */
      DEBUG_MSG("Look for known ESP commands using pointer ... ");
      // Commands to ESP:
      char Reboot_cmd[] = "**reboot"; // using reboot instead of Reboot eliminates ambiguity with Relay commands
      char WRU_cmd[] = "*?";
      char OTA_cmd[] = "*OTA";
      char Relay_cmd[] = "*R";
      char Count_cmd[] = "*C";
      char *ptr2WRU = 0;
      ptr2WRU = strstr(payload, WRU_cmd);
      DEBUG_MSG("WRU pointer is %p\n\r", ptr2WRU);
      DEBUG_MSG("payload pointer is %p\n\r", payload);
      if (ptr2WRU && (ptr2WRU < (payload + 3)))
      // look for WRU_cmd in the payload in the first 2 positions
      // if ((char)payload[1] == '?' || (char)payload[2] == '?') // '?' means this is a message to ESP - publish a "we're fine!" message
      {
        DEBUG_MSG("Creating status message.. \n\r");
        MQTT_Status_msg();
        MQTT_echoPub(mqtt_announce);
        return;
      }
      else if (strstr(payload, OTA_cmd))
      {
        DEBUG_MSG("Received OTA check (%s) in payload %s \n", payload, OTA_cmd);
        checkForUpdates();
        return;
      }
      else if (strstr(payload, Reboot_cmd)) // Special action: if a reboot request
      {
        // These pin settings will setup the ESP for rebooting
        // without them the ESP will be in a mode ready to download from serial input.
        // It will be a constant reboot because then it trys to continually process the "Reboot" message.
        // ROOT Cause: the ESP MQTT is not acknowledging receiving the reboot, so the broker
        // keeps sending it (when qos>0). For now, workaround is to only send qos=1
        // and locally if it has been less than 30 seconds since the reboot then don't reboot again.
        pinMode(0, OUTPUT);
        digitalWrite(0, 1);
        pinMode(2, OUTPUT);
        digitalWrite(2, 1);
        pinMode(15, OUTPUT);
        digitalWrite(15, 0);

        //if more than 30 seconds since booting then probably okay to reboot
        if ((millis() - start_time) > 30000)
        {
          std::string RebootingMsg = "Rebooting...";
          DEBUG_MSG("%s\r\n", RebootingMsg.c_str());
          MQTT_echoPub(&RebootingMsg[0]);
          digitalWrite(15,0); // ensure the GPIO15 is off
          safeRestart();
          DEBUG_MSG("Trying reboot... Press Hard reset if no response.\r\n");
          int RebootTime = millis();
          while ((millis() - RebootTime) < 10000)
          {
            //do idle stuff
            // int i = rand();
          }
          DEBUG_MSG("Should have rebooted... Press Hard reset or power cycle unit.\r\n");
        }
        else
        {
          std::string RebootingMsg = "Ignoring 'reboot' - not enough time since last boot. Try again.";
          DEBUG_MSG("%s\r\n", RebootingMsg.c_str());
          MQTT_echoPub(&RebootingMsg[0]);
        }

        return;
      }
      else if (strstr(payload, Relay_cmd) )
      { //check if this subscription is for relay control on the ESP - expects: '[*Rxx]' or 'STX *Rxx ETX'
        DEBUG_MSG("Rec'd subscription is for relay control on the ESP.\r\n");
        MQTT_process_Relay(payload); // TODO: should only need one reference to the payload not two
        return;
      }
      else if (strstr(payload, Count_cmd) )
      { 
        //gets current count 
        //if command is CR then reset counter
        DEBUG_MSG("Rec'd request for current count.\r\n");
        // MQTT_echoPub("need to add count here");
        return;
      }
      else
      {
        std::string bad_Command = "Unrecognized ESP Command: ";
        // TODO: echo back the bad command, e.g.,  payload
        DEBUG_MSG("%s\r\n", bad_Command.c_str());
        MQTT_echoPub(&bad_Command[0]);
        return;
      }
    }

    if ((char)payload[1] != '*') // payload is Not for ESP, so send to Serial (e.g. BSM_LT)
    {
      String str_payload = Put_Serial(payload, len);
      DEBUG_MSG("payload (message): '%s' with length %u message sent to BSM: %s\n\r", payload, len, str_payload.c_str());
    }
  } // end processing of device specific commands (e.g Sensor/to_device/MILwright-D0D2)
  else if (strcmp(topic, MQTT_SUB_TOPIC_1) == 0)
  {
    DEBUG_MSG("MQTT_SUB_TOPIC_1: Here's the payload (message): '%s' with length %d\n\r", payload, len);
    MQTT_process_Relay(payload);
  }
  else if (strcmp(topic, MQTT_SUB_TOPIC_2) == 0)
  {
    String str_payload = Put_Serial(payload, len);
    DEBUG_MSG("MQTT_SUB_TOPIC_2: Here's the payload (message): '%s' with length %u message sent to BSM: %s\n\r", payload, len, str_payload.c_str());
  }
  else if (strcmp(topic, MQTT_SUB_TOPIC_3) == 0)
  {
    DEBUG_MSG("MQTT_SUB_TOPIC_3: Serial output of payload (message): '%s' with length %u bytes\n\r", payload, len);
    String str_payload = Put_Serial(payload, len);
    DEBUG_MSG("Message sent to BSM: %s\n\r", str_payload.c_str());
  }
  else if (strcmp(topic, MQTT_LISTEN_TOPIC) == 0) //TODO this could be used to process control requests
  {
    DEBUG_MSG("MQTT_LISTEN_TOPIC: Here's the payload (message): '%s' with length %d\n\r", payload, len);
    // if (addressedToUs)
    // {
    //   Put_Serial(payload); // send to payload to MCC
    // }
  }
  else // doesn't look like we caught this one - how'd it get here
  {
    std::string bad_Command = "Unrecognized or unprocessed topic";
    DEBUG_MSG("%s\r\n", bad_Command.c_str());
    MQTT_echoPub(&bad_Command[0]);
    return;
  }
  digitalWrite(LED4, 0);
  AP_Gateway_previous_ms = millis();
  DEBUG_MSG("   Done processing this topic: '%s' and message: '%s' \n\r", topic, payload);
}

void MQTT_Status_msg()
{
  DEBUG_MSG("Old MQTT_echoPub announcement of '%s' ", mqtt_announce);
  int mqtt_msg_len = snprintf(mqtt_announce, MQTT_MAX_MESSAGE_LENGTH, "{\"NTP_Time\":\"%s\", \"host_Name\":\"%s\",\"Version\":\"%d\",  \"Build\":\"%s\", \"On_Line\":1, \"IP\":\"%s\", \"RID\":\"%s\", \"Uptime\":%d}", digitalClockDisplay(now(), ESPHTTPServer._config.timezone, ESPHTTPServer._config.daylight).c_str(), host_Name.c_str(), APP_VERSION, buildtimestamp_s.c_str(), localIP_s.c_str(), Read_Mac_Address_s().c_str(), int(millis() / 1000));
  DEBUG_MSG("Trying MQTT_echoPub length '%d' announcement of '%s' ", mqtt_msg_len, mqtt_announce);
}

void MQTT_process_Relay(char *ptr2payload)
{
  // Look to see if this is a relay control command
  // Format is RrO  where 'R' means Relay command, 'r' = index of relay number, and 'O' is on/off state (1/0)
  // CUSTOMIZATION: set Relay_Pin in Default.h
  //
  DEBUG_MSG(">>>>>>>> R E L A Y   P R O C E S S I N G <<<<<<\n\rMessage Rec'd: '%s' \n\r", ptr2payload);
  // DEBUG_MSG("payload  pointer is %d address\n\r", &payload);
  // DEBUG_MSG("Message pointer is %d address\n\r", ptr2payload);
  // DEBUG_MSG("pointer Message  is %s\n\r", ptr2payload);
  // // DEBUG_MSG("payload[0] is %d with value %s\n\r", &payload, payload[0].c_str());
  if (ptr2payload[0] == 'R')
  {
    int Relay_Number = ptr2payload[1] - '0'; // subtract ASCII '0' to get the DEC of the index in the Relay_Pin array
    DEBUG_MSG("Decoded as relay#/pin#: %d/%d.", Relay_Number, Relay_Pin[Relay_Number]);
    int statePosition = 2; // uses zero indexed position
    Relay_State[Relay_Number] = ptr2payload[statePosition] % 2; // just use modulus of payload as only expecting On/Off;;
    DEBUG_MSG("  State: %d - %s\n\r", Relay_State[Relay_Number], (Relay_State[Relay_Number] == 0) ? "LOW" : "HIGH");
    // TODO: Verify it 0/low is on or off for LEDx (or Relay_Pin[x-1])
    // TODO: look for and set all of the states for the relays
    digitalWrite(Relay_Pin[Relay_Number], Relay_State[Relay_Number]);    
  }
  DEBUG_MSG("Relay processed\n\r");
}

String Put_Serial(char outputChars[], size_t length)
{
  //DEBUG_MSG("Outputting outputChars to serial port: '%s' with length %d\n\r",
  // outputChars, sizeof(outputChars));
  String str_payload = "";
  size_t i = 0;
  // outputChars should be terminated with a 0x0 value as last character, but just in case only output the designated length
  // for (i = 0; ((outputChars[i] != 0x0) && (i < MQTT_MAX_MESSAGE_LENGTH)); i++)
  // for (i = 0; ((outputChars[i] != 0x0) && (i < length)); i++)
  for (i = 0; (i < length); i++)
  {
    str_payload += (char)outputChars[i];
    SERIAL_OTHER_DEVICE_PORT.print((char)outputChars[i]);
  }
  str_payload[i] = 0x0;
  SERIAL_OTHER_DEVICE_PORT.println();
  return str_payload;
}

void onMqttPublish(uint16_t packetId)
{
  //DEBUG_MSG("** Publish acknowledged **");
  //DEBUG_MSG("  packetId: %d\n\r", packetId);
}

void MQTT_Async_setup()
{
  DEBUG_MSG("ESP8266 Aync MQTT Setup\n\r");
  // mqttClient.setMaxTopicLength(uint16_t MQTT_MAX_TOPIC_LENGTH);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setClientId(ESPHTTPServer._config.deviceName.c_str());
  mqttClient.setServer(mqtt_broker.c_str(), mqtt_port);

  // Customize the .setWill topic and LWT message
  // int n = snprintf(lastMessage, MQTT_MAX_MESSAGE_LENGTH, "host_Name '%s' is now off-line. On_line=0",
  //                 host_Name.c_str());
  lastMessage = "{\"host_Name\":\"" + ESPHTTPServer._config.deviceName + "\", \"RID\":\"" + Read_Mac_Address_s().c_str() + "\",  \"On_Line\":0}";
  lastTopic = MQTT_PUB_TOPIC;
  lastTopic += "/" + host_Name;
  // n = snprintf(topic2Pub, MQTT_MAX_TOPIC_LENGTH, "%s/%s", topic2Pub, host_Name.c_str());
  DEBUG_MSG("MQTT setWill topic: %s message: %s with QoS: %d\n\r", lastTopic.c_str(), lastMessage.c_str(), 1);

  mqttClient.setKeepAlive(5)
      .setCleanSession(false)
      .setWill(lastTopic.c_str(), 1, true, lastMessage.c_str())
      .setCredentials("username", "password")
      .setClientId(host_Name.c_str());

  // mqttClient.set_callback(receive_ota);   // TODO: Register our callback for receiving OTA's
  MQTT_lastReconnectAttempt = 0; //ISSUE: what is this used for?

  int mqtt_AttemptedSince = millis(); // start the timer
  int mqtt_Timer = 0;
  DEBUG_MSG("\n\rConnecting to MQTT...");
  yield();// reset the WDT - we've to a lot going on in setup()savin
  mqttClient.connect();
  while (!mqttClient.connected() && mqtt_Timer < 10000)
  {
    delay(1000);
    DEBUG_MSG("mqtt.");
    mqtt_Timer = millis() - mqtt_AttemptedSince;
  }
  DEBUG_MSG(">Time to connect: %d\n\r", mqtt_Timer);

  // Below is only support with https://pubsubclient.knolleary.net/api.html (include pubsubclient.h)
  //   switch  (mqttClient.state()){
  //     case	-4	:  DEBUG_MSG("	 MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time	 \n\r");
  //     case	-3	:  DEBUG_MSG("	 MQTT_CONNECTION_LOST - the network connection was broken	 \n\r");
  //     case	-2	:  DEBUG_MSG("	 MQTT_CONNECT_FAILED - the network connection failed	 \n\r");
  //     case	-1	:  DEBUG_MSG("	 MQTT_DISCONNECTED - the client is disconnected cleanly	 \n\r");
  //     case	0	:  DEBUG_MSG("	 MQTT_CONNECTED - the client is connected	 \n\r");
  //     case	1	:  DEBUG_MSG("	 MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT	 \n\r");
  //     case	2	:  DEBUG_MSG("	 MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier	 \n\r");
  //     case	3	:  DEBUG_MSG("	 MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection	 \n\r");
  //     case	4	:  DEBUG_MSG("	 MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected	 \n\r");
  //     case	5	:  DEBUG_MSG("	 MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect	 \n\r");
  // }
  //
  // need to put in a loop to wait 10 seconds for the initial connection
  if (mqttClient.connected())
  {
    DEBUG_MSG("	 MQTT_CONNECTED - the client is connected to broker: %s\n\r", mqtt_broker.c_str());
    digitalWrite(LED4, 0); 
  }
  else
  {
    DEBUG_MSG("	 MQTT_CONNECT_FAILED - the network connection failed	 \n\r");
    DEBUG_MSG("	Let's try disconnect and then connect again. \n\r");
    digitalWrite(LED4, 1); // leave it ON as indication that not completed
    mqttClient.disconnect();
    digitalWrite(LED4, 0); // turn off as try to reconnect
    mqttClient.connect();
    if (!mqttClient.connected())
    {
      DEBUG_MSG("	 MQTT_CONNECT_FAILED - check wifi connection and broker status. \n\r");
      digitalWrite(LED4, 1); // leave it ON as indication that not completed
    }
  }
}

bool MQTT_pub(char *topic, char *Message, uint8_t qos)
{

  DEBUG_MSG("\r\nNow in MQTT_pub with Free Heap size: %u\n\r", ESP.getFreeHeap());

  if (mqttClient.publish((char *)topic, qos, true, (char *)Message))
  {
    // publish(const char* topic, uint8_t qos, bool retain, const char* payload,
    // size_t length)
    DEBUG_MSG("MQTT published okay!!\n\r >>>>Topic: '%s'. \n\r>>>>Message: >%s<.\n\r",
              topic, Message);
    return true;
  }
  else
  {
    DEBUG_MSG("Oh, Nooo!? - MQTT publish failed. \n\r  >>>>Topic: '%s'. "
              "\n\r>>>>Message:'%s'.\n\r on MQTT broker:port %s:%d\n\r",
              topic, Message, mqtt_broker.c_str(), mqtt_port); // TRY AGAIN?
    return false;
  }
}

void MQTT_echoPub(char *msg2Pub)
{
  // where input is value pointed to by msg2Pub which is message to send back to broker.
  // The following will echo back to the MQTT broker the message just received
  // Expectation is that the first 3 characters are setting the state of the
  // Relay
  // this should only be done when the incoming topic is 'MQTT_LISTEN_TOPIC'
  // or
  // some other control type parameters

  char topic2Pub[MQTT_MAX_TOPIC_LENGTH] = MQTT_PUB_TOPIC;
  // Echo message from this host_Name
  strcat(topic2Pub, "/");
  strcat(topic2Pub, host_Name.c_str());
  strcat(topic2Pub, "/Status");

  // char msg2Pub[MQTT_MAX_MESSAGE_LENGTH] = "";
  // char msg2Pub[MQTT_MAX_MESSAGE_LENGTH] = "---- Received by ";
  // strcat(msg2Pub, host_Name.c_str());
  // part of echo when used to show the relay toggle TODO move this logic out
  // of
  // MQTT
  // msg2Pub[0] = payload[0];
  // msg2Pub[1] = payload[1];
  // msg2Pub[2] = payload[2];
  // strcat(msg2Pub, ": ");
  // strcat(msg2Pub, msg_payload);

  if (MQTT_pub((char *)topic2Pub, (char *)msg2Pub, MQTT_QOS))
  {
    DEBUG_MSG("MQTT callback published okay! Topic:'%s'. Message:'%s'.\n\r'",
              topic2Pub, msg2Pub);
  }
  else
  {
    DEBUG_MSG("Well that didn't work. Now what?");
  }
}
