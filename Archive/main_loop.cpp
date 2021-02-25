void loop()
{
  unsigned long currentMillis = millis();
  if (OTA_In_Process == false) // if not actively doing OTA update, do normal stuff
  {
    // >>>>>>>>>>>>>>>>> Check for messages from serial port
    mqtt_Pub_message = serialRead.handle(Other_Device_Port);
    if (mqtt_Pub_message != "") //publish message if anything found
    {
      mqtt_Pub_message.trim();
      // Prepend MAC_Address (RID)
      String mqtt_Pub_message_s = "{\"RID\":\"" + Read_Mac_Address_s() + "\", \"Serial_msg\":\"" + mqtt_Pub_message + "\"}"; // RadioID equiv of Read_Mac_Address_s
      MQTT_pub_calculated_Topic(mqtt_Pub_message_s);
    }
    // >>>>>>>>>>>>>>>>> Check on sensor to ground (has there been a cycle/toggle of the cycle sensor?)
    //  MQTT pub if a "cycle"
    mqtt_Pub_message = Sensor2Gnd_Loop();
    if (mqtt_Pub_message != "")
    {
      MQTT_pub_calculated_Topic(mqtt_Pub_message);
    }

    // >>>>>>>>>>>>>>>>> Update DHT readings every x milliseconds
    long now = millis();

    // int msg_length;
    if (now - lastMsg > intervalMillis_Cycle)
    {
      digitalWrite(Status_LED, HIGH);
      String Mqtt_Message = DHT_Read_JSON(); // get Mqtt_Message from sensor to Publish
      lastMsg = now;
      nist_time = digitalClockDisplay();
      DEBUG_MSG("Current nist_time: %s\n\r", nist_time.c_str());
      // publish reading if either T|R has changed <<< looks like the Temperature
      // is out of scope
      StaticJsonDocument<200> doc;
      deserializeJson(doc, Mqtt_Message);
      String Temperature = doc["T"];
      String Humidity = doc["H"];
      if (LastTemp != Temperature ||
          LastHumid != Humidity)
      { // format and output new reading
        DEBUG_MSG("Current nist_time: %s\n\r", nist_time.c_str());
        MQTT_pub_calculated_Topic(Mqtt_Message);
        LastTemp =
            Temperature;
        LastHumid =
            Humidity;
        // ++Packet_ctr;
        // char msg[150];
        // msg_length =
        //     snprintf(msg, 150, "Packet #%d contains: %s", Packet_ctr, Buffer);
        // DEBUG_MSG("Publish message length: %d with msg %s\n\r", msg_length,
        //           msg);
        // DEBUG_MSG("Buffer contents:  %s \n\r", Buffer);
      }
      else
      {
        DEBUG_MSG(" - No H/T change"); // so no output
      }
      digitalWrite(Status_LED, LOW);
    }
  }
  // time dependent actions
  if ((currentMillis % HEARTBEAT_INTERVAL) == 0)
  {
    digitalWrite(LED5, 1); 
    checkForUpdates(OTA_SERVER);
    digitalWrite(LED5, 0);
  }

  if (currentMillis > 0 && (int(currentMillis % (24 * 60 * 60 * 1000)) == 0)) // let's reboot every 24 hours of uptime
  {
    DEBUG_MSG("Running for a day, so, let's reboot %lu", currentMillis);
    ESP.restart();
  }
  int flash_LED = LED5;
  if (!flashing)
  {
    // DEBUG_MSG("AP_Gateway_previous: %lu, currentMillis; %lu, Blink_Interval %lu\n", AP_Gateway_previous_ms,currentMillis, AP_Gateway_Blink_Interval );
    if (AP_Gateway_previous_ms > 0 && ((currentMillis - AP_Gateway_previous_ms) > AP_Gateway_Blink_Interval))
    // if in STA and talking with Gateway and have exceeded Gateway Blink interval - i.e. now time to blink
    // example: AP_Gateway_Blink_Interval is 250 millisec and activity to AP_Gateway (AP_Gateway_previous_ms >0)
    {
      DEBUG_MSG(". LED is %d for Gateway activity.\n\r", digitalRead(flash_LED));
      flashing = true;
      Blink_count = 0;
      start_time = currentMillis;
      AP_Gateway_previous_ms = 0; //gets set to non-zero when Gateway activity
    }
  }
  else if (flashing)// when flashing mode
  {
    // DEBUG_MSG("+");
    if ((currentMillis - start_time >= AP_Gateway_Blink_Interval))
    {
      digitalWrite(flash_LED, !digitalRead(flash_LED));
      start_time = currentMillis;
      Blink_count++;
      DEBUG_MSG("State is %d at flash count is %d at currentMillis %lu.\n",digitalRead(flash_LED), Blink_count, currentMillis);
      if (Blink_count == 4)
      {
        flashing = false;
        digitalWrite(flash_LED, LOW); // leave LED ON
      }
    }
  }

  ESPHTTPServer.handle(); // this handles webserver for both AP and for STA modes and also OTA Updates
  ArduinoOTA.handle();
}