void setup_WiFi(bool APConfigMode) {
  delay(100);
  // Connecting to a WiFi network
WiFi.mode(WIFI_OFF);
  WiFi.macAddress(mac);
  RadioID = String(mac[0], HEX) + String(mac[1], HEX) + String(mac[2], HEX) +
            String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  // with ":" separators
  //    +":"+String(mac[1],HEX)
  //    +":"+String(mac[2],HEX)
  //    +":"+String(mac[3],HEX)
  //    +":"+String(mac[4],HEX)
  //    +":"+String(mac[5],HEX);
  //
  char this_Hostname[25] = "";
  sprintf(this_Hostname, "%s_%x%x", host, mac[4], mac[5]);
  host_Name = String(this_Hostname);
  WiFi.hostname(host_Name); // this should be enough to make the hostname unique
  DEBUG_PORT.println(host_Name);
  DEBUG_PORT.printf("MAC (aka: RadioID) address:%s\n\r ", RadioID.c_str());
  //
  WiFi.mode(WIFI_STA);
  // currentWifiStatus = WIFI_STA_DISCONNECTED;
  DEBUG_MSG("Connecting to %s\r\n", ssid);
WiFi.begin(ssid, password);
  // WiFi.begin("Liberty4", "Gr8Vu0fTam");
  // if (!_config.dhcp) {
  //     DEBUGLOG("NO DHCP\r\n");
  //     WiFi.config(_config.ip, _config.gateway, _config.netmask, _config.dns);
  // }
  // delay(2000);
  // delay(5000); // Wait for WiFi

  // TODO if haven't connected within 15 seconds then go into AP mode
  //  _apConfig.APenable = true;
  //  configureWifiAP();
  // return; << or not
  //
  while (!WiFi.isConnected()) {
    delay(1000);
    DEBUG_PORT.print(".");
  }
  DEBUG_PORT.println();
  /*if (WiFi.isConnected()) {
      currentWifiStatus = WIFI_STA_CONNECTED;
  }*/

  DEBUG_MSG("IP Address: %s\n", WiFi.localIP().toString().c_str());

  DEBUG_MSG("Gateway:    %s\r\n", WiFi.gatewayIP().toString().c_str());
  DEBUG_MSG("DNS:        %s\r\n", WiFi.dnsIP().toString().c_str());
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");

  ipAddr = DisplayAddress(WiFi.localIP());
  DEBUG_MSG("ESP8266 Hostname: %s\n\r", host_Name.c_str());
  DEBUG_MSG("Connected to: %s\n\r", ssid);
  DEBUG_MSG("IP address: %s\n\r", ipAddr.c_str());
  DEBUG_MSG("MAC (aka: RadioID) address:%s\n\r ", RadioID.c_str());
  DEBUG_PORT.printf("MAC (aka: RadioID) address:%s\n\r ", RadioID.c_str());
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DEBUG_MSG("signal strength (RSSI): %lddBm", rssi);
  // return hostname;
}

void setup_DHT() {
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  sensor_type = sensor.name;

  // delay=100; // needed?
  DEBUG_PORT.println("Using DHTxx Unified Sensor: " + String(DHTTYPE) +
                     " - Output on ESP");
  dht.temperature().getSensor(&sensor);
  DEBUG_PORT.println("------------------------------------");
  DEBUG_PORT.print("Minimum Sensor delay: ");
  DEBUG_PORT.println(sensor.min_delay);
  DEBUG_PORT.println("Temperature");
  DEBUG_PORT.print("Sensor:       ");
  DEBUG_PORT.println(sensor.name);
  DEBUG_PORT.print("Driver Ver:   ");
  DEBUG_PORT.println(sensor.version);
  DEBUG_PORT.print("Unique ID:    ");
  DEBUG_PORT.println(sensor.sensor_id);
  DEBUG_PORT.print("Max Value:    ");
  DEBUG_PORT.print(sensor.max_value);
  DEBUG_PORT.println(" *C");
  DEBUG_PORT.print("Min Value:    ");
  DEBUG_PORT.print(sensor.min_value);
  DEBUG_PORT.println(" *C");
  DEBUG_PORT.print("Resolution:   ");
  DEBUG_PORT.print(sensor.resolution);
  DEBUG_PORT.println(" *C");
  DEBUG_PORT.println("------------------------------------");

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  DEBUG_PORT.println("------------------------------------");
  DEBUG_PORT.println("Humidity");
  DEBUG_PORT.print("Sensor:       ");
  DEBUG_PORT.println(sensor.name);
  DEBUG_PORT.print("Driver Ver:   ");
  DEBUG_PORT.println(sensor.version);
  DEBUG_PORT.print("Unique ID:    ");
  DEBUG_PORT.println(sensor.sensor_id);
  DEBUG_PORT.print("Max Value:    ");
  DEBUG_PORT.print(sensor.max_value);
  DEBUG_PORT.println("%");
  DEBUG_PORT.print("Min Value:    ");
  DEBUG_PORT.print(sensor.min_value);
  DEBUG_PORT.println("%");
  DEBUG_PORT.print("Resolution:   ");
  DEBUG_PORT.print(sensor.resolution);
  DEBUG_PORT.println("%");
  DEBUG_PORT.println("------------------------------------");

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 100;
  // set to one minute = 60000 milliseconds)
  delayMS = 60000; // TODO: read from the gateway

  DEBUG_PORT.print(
      "DHT Initialization Complete! Interval (seconds) between readings: ");
  DEBUG_PORT.println(delayMS / 1000);
}

void setup_OTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("esp8266_OTA");
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"1234");
  ArduinoOTA.onStart([]() { DEBUG_PORT.println("OTA Start"); });
  ArduinoOTA.onEnd([]() { DEBUG_PORT.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PORT.println("OTA Error[%u]: " + error);
    if (error == OTA_AUTH_ERROR)
      DEBUG_PORT.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      DEBUG_PORT.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      DEBUG_PORT.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      DEBUG_PORT.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      DEBUG_PORT.println("End Failed");
  });

  ArduinoOTA.begin();
  DEBUG_PORT.println("OTA Ready");
  DEBUG_PORT.print("IP address: ");
  DEBUG_PORT.println(WiFi.localIP());
}

void setupTimeNTP() {
  DEBUG_PORT.println("TimeNTP");

  DEBUG_PORT.println("Starting UDP");
  Udp.begin(localPort);
  DEBUG_PORT.print("Local port: ");
  // DEBUG_PORT.println(Udp.localPort());
    DEBUG_PORT.println(localPort);
  DEBUG_PORT.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(TimeRefresh4NIST);
}

void setup_SPIFFS() {

  // "mount" the filesystem
  bool result = SPIFFS.begin();
  DEBUG_PORT.println("       SPIFFS opened: " + result);
  {
    DEBUG_PORT.println("Files found in SPI Filesystem");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DEBUG_PORT.printf("  File: %s, size: %s\n", fileName.c_str(),
                        formatBytes(fileSize).c_str());
    }
    DEBUG_PORT.printf("\n");
  }
}

void read_Config_setup() {

  if (!loadConfig()) {
    DEBUG_PORT.println("Failed to load config");
  } else {
    DEBUG_PORT.println("RadioID: "+RadioID);
    DEBUG_PORT.println("Config file loaded!");
  }
}
