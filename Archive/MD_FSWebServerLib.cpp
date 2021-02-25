// DEBUG_MSG// Modified by Chris Armbrust, Marin Digital 24July2017
// Original: https://github.com/gmag11/FSBrowserNG
//

#include "MD_FSWebServerLib.h"
#include <Streamstring.h>

Credentials storedCreds;

AsyncFSWebServer ESPHTTPServer(80);

const char Page_WaitAndReload[] PROGMEM = R"=====(
<meta http-equiv="refresh" content="10; URL=./index.htm">
Please Wait....Configuring and Restarting.
)=====";

const char Page_Restart[] PROGMEM = R"=====(
<meta http-equiv="refresh" content="10; URL=admin/restart">
Please Wait.... Module is Restarting.
)=====";

// String RebootMsg = "<meta http-equiv=\"refresh\" content=\"30; URL=/index.htm\">Please Wait... Node is Restarting.";

AsyncFSWebServer::AsyncFSWebServer(uint16_t port) : AsyncWebServer(port) {}

/*void AsyncFSWebServer::secondTick()
{
    _secondFlag = true;
}*/

/*void AsyncFSWebServer::secondTask() {
    //DEBUG_MSG("%s\r\n", NTP.getTimeDateString().c_str());
    sendTimeData();
}*/

void AsyncFSWebServer::s_secondTick(void *arg)
{
  AsyncFSWebServer *self = reinterpret_cast<AsyncFSWebServer *>(arg);
  if (self->_evs.count() > 0)
  {
    self->sendTimeData();
  }
}

void AsyncFSWebServer::sendTimeData()
{
  String data = "{";
  data += "\"time\":\"" + NTP.getTimeStr() + "\",";
  data += "\"date\":\"" + NTP.getDateStr() + "\",";
  data +=
      "\"lastSync\":\"" + NTP.getTimeDateString(NTP.getLastNTPSync()) + "\",";
  data += "\"uptime\":\"" + NTP.getUptimeString() + "\",";
  data +=
      "\"lastBoot\":\"" + NTP.getTimeDateString(NTP.getLastBootTime()) + "\"";
  data += "}\r\n";
  DEBUG_MSG(data.c_str());
  _evs.send(data.c_str(), "timeDate");
  DEBUG_MSG("%s\r\n", NTP.getTimeDateString().c_str());
  data = String();
  // DEBUG_MSG(__PRETTY_FUNCTION__);
  // DEBUG_MSG("\r\n")
}

String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

bool isApprovedAP(String AP) // Is this an "Authorized" AP (used to select strongest Access Point / Gateway)
{
  char *p;
  p = strstr(AP.c_str(), WIFI_APPROVED_AP_PREFIX); //TODO: change to Global config variable
  if (p)
  {
    return true;
  }
  //   p = strstr(AP.c_str(), _config.ssid); //Allow a AP defined in config file
  // if (p)
  // {
  //   return true;
  // }
  return false;
}

void AsyncFSWebServer::begin(FS *fs)
{
  // Start the
  // <webserver:>
  // 1. start WiFi in either AP or STA mode
  // 2. if STA mode, start NTP
  // 3. Start MDNS which allows finding by host name (might make sense only for
  // STA mode)
  // </webserver:>
  //
  _fs = fs;
  //   DBG_OUTPUT_PORT.begin(SERIAL_BAUDRATE);

  // #ifndef RELEASE
  //   DBG_OUTPUT_PORT.setDebugOutput(true);
  // #endif // RELEASE
  // NTP client setup
  // TODO WiFi setup - may need to redefine the AP_ENABLE_BUTTON based on the
  // value of _config.WiFiMode
  WiFi.persistent(false); // don't rewrite flash on changes to SSID, passphrase and disconnect are called. Explicitly manage wifi mode and reconnects with handlers
  if (CONNECTION_LED >= 0)
  {
    pinMode(CONNECTION_LED, OUTPUT);    // CONNECTION_LED pin defined as output
    digitalWrite(CONNECTION_LED, HIGH); // Turn LED off
  }
  if (AP_ENABLE_BUTTON > 0)
  {
    pinMode(AP_ENABLE_BUTTON, INPUT_PULLUP);
    // If this pin is HIGH during
    // startup ESP will run in AP_ONLY
    // mode. Backdoor to change WiFi
    // settings when configured WiFi is
    // not available.
  }
  // analogWriteFreq(200);
  //
  // if (CONNECTION_LED >= 0)
  // {
  //   digitalWrite(CONNECTION_LED, HIGH); // Turn LED off
  // }
  DEBUG_MSG("File system mounted!\n\r");
  delay(1000);
  if (!_fs) // If SPIFFS is not started
    _fs->begin();

#ifndef RELEASE
  { // List files
    DEBUG_MSG("\nHere is the list of files in SPIFFS:\n\r");
    Dir dir = _fs->openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();

      DEBUG_MSG("FS File: %s, size: %s\r\n", fileName.c_str(),
                formatBytes(fileSize).c_str());
    }
    DEBUG_MSG("\n");
  }
#endif // RELEASE

  if (!load_config())
  {                            // Try to load configuration from file system
  DEBUG_MSG("load config.json failed, trying default config\n");
    defaultConfig();           // Load defaults if any error
    _apConfig.APenable = true; //TODO issue if file system is empty. Allow AP mode so can at least get to ESP via AP
  }

  // set the WiFiMode preference from either the AP_ENABLE_BUTTON or from the
  // config file (when AP_ENABLE_BUTTON=0 i.e. no button is wired in hardware)
  if (AP_ENABLE_BUTTON >= 0)
  {
    // leave APenable to value set in header file
    if (AP_ENABLE_BUTTON == 0)
    { // 0 means read from config file
      if (_config.WiFiMode == "AP")
      {
        _apConfig.APenable = true;
        wifiAP_timer = millis(); // start the timer to non-zero
      }
      if (_config.WiFiMode == "STA")
      {
        _apConfig.APenable = false;
        wifiAP_timer = 0; // set timer to zero to indicate that not running in AP mode
      }
    }
    else
    {
      _apConfig.APenable = !digitalRead(
          AP_ENABLE_BUTTON); // Read AP button. If button is pressed activate AP
      DEBUG_MSG("Read AP Enable from pin #:%d", AP_ENABLE_BUTTON);
    }
    DEBUG_MSG(
        "AP enabled read pin#: %d  <WiFiMode:>%s</WiFiMode:>AP Enable = %d\n\r",
        AP_ENABLE_BUTTON, _config.WiFiMode.c_str(), _apConfig.APenable);
  }
  loadHTTPAuth();
  // WIFI INIT

  // Register event handlers.
  // Callback functions will be called as long as these handler objects exist.
  // Register wifi Event to control connection LED
  onStationModeConnectedHandler =
      WiFi.onStationModeConnected([this](WiFiEventStationModeConnected data) {
        this->onWiFiConnected(data);
        LED_StationModeConnected.attach(1, changeState, LED4); // starts blinking on LED4 (blue) every .5 seconds
      });
  onStationModeDisconnectedHandler = WiFi.onStationModeDisconnected(
      [this](WiFiEventStationModeDisconnected data) {
        this->onWiFiDisconnected(data);
        LED_StationModeConnected.detach();
        // igrr commented on Jun 20, 2016
        // As it was before, you should not do any lengthy operations from WiFi event callbacks. Certainly not run a loop waiting for the connection. You can set a flag in disconnect callback and then check the value of flag in your loop function.
        // Another thing to note: by default, ESP will attempt to reconnect to WiFi network whenever it is disconnected. You don't need to handle this manually. A good way to simulate disconnection from an AP would be to reset the AP. ESP will report disconnection, and it will try to reconnect automatically.
      });

  // void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  //   Serial.println("Connected to Wi-Fi.");
  //   connectToMqtt();
  // }

  // void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  //   Serial.println("Disconnected from Wi-Fi.");
  //   mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //   wifiReconnectTimer.once(2, connectToWifi);
  // }

  WiFi.hostname(_config.deviceName.c_str());
  // TODO - check that the _config.WiFiMode is AP or AP_STA or STA and then
  // branch accordlingly
  if (AP_ENABLE_BUTTON >= 0)
  {
    DEBUG_MSG("Enable ESP WiFimode AP =%d\r\n", _apConfig.APenable);
    if (_apConfig.APenable == true)
    {
      configureWifiAP(); // Set AP mode if AP button was pressed
    }
    else
    {
      configureWifi(); // Set WiFi config
      if (!WiFi.isConnected())
      {
        configureWifiAP(); // the STA mode failed so go into AP mode
      }
    }
  }
  else
  { // this condition is for when not enabling AP configuration from
    // either a button push or from a value in the config file
    DEBUG_MSG("Enable ESP WiFimode: Station\r\n");
    configureWifi(); // Set WiFi config because that is the normal WiFi mode
    // ->> this condition should be handled by WiFi mode STA timeout
    // if (!WiFi.isConnected())
    // {
    //   configureWifiAP(); // the STA mode failed so go into AP mode}
    // }
  }

  // TODO is this necessary if not in STA mode? Skip?
  if (_config.updateNTPTimeEvery > 0)
  {
    // while ((year(now()) < 1980) || (year(now()) > 2035))// attempt to ensure getting a sync with ntpserver
    // {
    //   int i;
    //   // delay(500);
    //   DEBUG_MSG("t");
    //   NTP.begin(_config.ntpServerName, _config.timezone / 10, _config.daylight); // sets the clock to the local time
    //   i++;
    //   if (i > 10)
    //   {
    //     DEBUG_MSG("\nCouldn't get good time, restart and try again.\n");
    //     ESP.restart();
    //   }
    // }
    // Enable NTP sync
    NTP.begin(_config.ntpServerName, _config.timezone / 10, _config.daylight); // sets the clock to the local time
    //  NTP.begin(_config.ntpServerName, 0, 0 ); // leave time in UTS/UTC
    NTP.setInterval(15, _config.updateNTPTimeEvery * 60);
  }
  DEBUG_MSG("Open http://");
  DEBUG_MSG(_config.deviceName.c_str());
  DEBUG_MSG(".local/edit to see the file browser\r\n");
  DEBUG_MSG("Flash chip size: %u\r\n", ESP.getFlashChipRealSize());
  DEBUG_MSG("Sketch size: %u\r\n", ESP.getSketchSize());
  DEBUG_MSG("Free flash space: %u\r\n", ESP.getFreeSketchSpace());

  _secondTk.attach(
      1.0f, &AsyncFSWebServer::s_secondTick,
      static_cast<void *>(this)); // Task to run periodic things every second

  AsyncWebServer::begin();
  serverInit(); // Configure handlers and start Web server

  MDNS.begin(
      _config.deviceName
          .c_str()); // I've not got this to work. Need some investigation.
  MDNS.addService("http", "tcp", 80);
  DEBUG_MSG("MDNS started\n");
  // OTA_Setup(_httpAuth.wwwPassword.c_str());
  DEBUG_MSG("END Setup\n");
} //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> End of AsyncWebServer begin

bool AsyncFSWebServer::load_config()
{
  File configFile = _fs->open(CONFIG_FILE, "r");
  if (!configFile)
  {
    DEBUG_MSG("Failed to open working config file. \n\r");
    return false;
  }

  size_t size = configFile.size();
  /*if (size > 1024) {
      DEBUG_MSG("Config file size is too large");
      configFile.close();
      return false;
  }*/

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  configFile.close();
  DEBUG_MSG("JSON '%s' file size: %d bytes\r\n", CONFIG_FILE, size);
  DynamicJsonDocument jsonDoc(1024);
  // StaticJsonBuffer<1024> jsonBuffer;
  DeserializationError error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    DEBUG_MSG("Failed to parse config file with error %s\r\n", error.c_str());
    // bool removed =configFile.remove();
    // DEBUG_MSG("Deleted file successfully - %s\r\n", removed ? "Yes": "No");
    return false;
  }
#ifndef RELEASE
  String temp;
  serializeJsonPretty(jsonDoc, temp);
  Serial.println(temp);
#endif

  _config.ssid = jsonDoc["ssid"].as<const char *>();

  _config.password = jsonDoc["pass"].as<const char *>();

  // if the length of the ssid is zero - means not a good SSID access point so reload from the default file
    // Load any existing credentials - maybe this is factory reset with fresh file system
  // want to preserve any existing WLAN connections
  Credentials storedCreds;
  storedCreds = loadCredentials();

  DEBUG_MSG("storedCreds.ssid: '%s' with length %d\n", storedCreds.ssid, sizeof(storedCreds.ssid));
  DEBUG_MSG("storedCreds.password: '%s' with length %d\r\n", storedCreds.password, sizeof(storedCreds.password));
  DEBUG_MSG("_config.ssid: '%s' ", _config.ssid.c_str());
  DEBUG_MSG("_config.password: '%s'\r\n", _config.password.c_str());

  if (_config.ssid  == "") // use default AP from stored credentials - 
  { 
    DEBUG_MSG("SSID: '%s'  from _config.ssid is '' - blank?. \n", _config.ssid.c_str());
    // memcpy(&_config.ssid,     &storedCreds.ssid,      strlen(storedCreds.ssid));
    // memcpy(&_config.password, &storedCreds.password,  sizeof(storedCreds.password));
    // memcpy(_config.ssid,     storedCreds.ssid,      strlen(storedCreds.ssid));
    // strcpy(_config.ssid.c_str(),     storedCreds.ssid) ;
    // strcpy(_config.password.c_str(), storedCreds.password);
    _config.ssid = storedCreds.ssid;
    _config.password = storedCreds.password;
    saveCredentials(storedCreds);
  }
  DEBUG_MSG("SSID: '%s' ", _config.ssid.c_str());
  DEBUG_MSG("PASS: '%s'\r\n", _config.password.c_str());

  _config.ip =
      IPAddress(jsonDoc["ip"][0], jsonDoc["ip"][1], jsonDoc["ip"][2], jsonDoc["ip"][3]);
  _config.netmask = IPAddress(jsonDoc["netmask"][0], jsonDoc["netmask"][1],
                              jsonDoc["netmask"][2], jsonDoc["netmask"][3]);
  _config.gateway = IPAddress(jsonDoc["gateway"][0], jsonDoc["gateway"][1],
                              jsonDoc["gateway"][2], jsonDoc["gateway"][3]);
  _config.dns =
      IPAddress(jsonDoc["dns"][0], jsonDoc["dns"][1], jsonDoc["dns"][2], jsonDoc["dns"][3]);

  _config.dhcp = jsonDoc["dhcp"].as<bool>();

  _config.ntpServerName = jsonDoc["ntp"].as<const char *>();
  _config.updateNTPTimeEvery = jsonDoc["NTPperiod"].as<long>();
  _config.timezone = jsonDoc["timeZone"].as<long>();
  _config.daylight = jsonDoc["daylight"].as<long>();
  _config.deviceName = jsonDoc["deviceName"].as<const char *>();

  _config.mqtt_broker_ip =
      IPAddress(jsonDoc["mqtt_broker_ip"][0], jsonDoc["mqtt_broker_ip"][1],
                jsonDoc["mqtt_broker_ip"][2], jsonDoc["mqtt_broker_ip"][3]);
  _config.mqtt_QoS = jsonDoc["mqtt_QoS"].as<long>();
  _config.mqtt_port = jsonDoc["mqtt_port"].as<long>();
  _config.mqtt_retain = jsonDoc["mqtt_retain"].as<bool>();
  // _config.meia = jsonDoc["meia"].as<const char *>();
  // _config.ueia = jsonDoc["ueia"].as<const char *>();
  _config.WiFiMode = jsonDoc["WiFiMode"].as<const char *>();

   _config.debounceDelay = jsonDoc["debounceDelay"].as<long>();

  // config.connectionLed = jsonDoc["led"];
  // TODO - retrieve and initialize MCC data - note: BSM/Battery data can change
  // each charge - may want to retrieve that from MySQL
  //           therefore: that logic may need to just be on the retrieval page
  //           and retreived as requested
  //          although - at bootup need to do a "Who's there?" (WRU) and log
  //          that to the SQL database

  DEBUG_MSG("Data read from config and _config.* values now initialized.\r\n");
  DEBUG_MSG("SSID: %s ", _config.ssid.c_str());
  DEBUG_MSG("PASS: %s\r\n", _config.password.c_str());
  DEBUG_MSG("NTP Server: %s\r\n", _config.ntpServerName.c_str());
  DEBUG_MSG("deviceName: %s\r\n", _config.deviceName.c_str());
  mqtt_broker = IpAddress2String(ESPHTTPServer._config.mqtt_broker_ip);
  DEBUG_MSG("MQTT broker: %s\r\n", mqtt_broker.c_str());
  // DEBUG_MSG("Connection LED: %d\n", config.connectionLed);
  DEBUG_MSG("In function: %s\r\n", __PRETTY_FUNCTION__);

  return true;
}

bool AsyncFSWebServer::defaultConfig()
{
  // DEFAULT CONFIG - read from default config file
  DEBUG_MSG("Trying to open default config file %s\n", CONFIG_FILE_DEFAULT);
  // char macStr[18] = {0};
  // byte mac[6];
  File configFile = _fs->open(CONFIG_FILE_DEFAULT, "r");
  if (!configFile)
  {
    DEBUG_MSG("Failed to open default config file. Has the FS been initialized "
              "correctly?\n");
    return false;
  }
  else {DEBUG_MSG("opened default config file\n");}

  size_t size = configFile.size();
  /*if (size > 1024) {
      DEBUG_MSG("Config file size is too large");
      configFile.close();
      return false;
  }*/

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  configFile.close();
  DEBUG_MSG("Using default config JSON file %s with size: %d bytes\r\n", CONFIG_FILE_DEFAULT, size);
  DynamicJsonDocument jsonDoc(1024);
  // StaticJsonBuffer<1024> jsonBuffer;
  DeserializationError error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
    DEBUG_MSG("Failed to parse default config file with error %d\r\n", error.code());
    return false;
  }
#ifndef RELEASE
  String temp;
  serializeJsonPretty(jsonDoc, temp);
  Serial.println(temp);
#endif

  // Load any existing credentials - maybe this is factory reset with fresh file system
  // want to preserve any existing WLAN connections
  Credentials storedCreds;
  storedCreds = loadCredentials();
  if (strlen(storedCreds.ssid) == 0) // use default AP from file
  { 
    _config.ssid = jsonDoc["AP_Default_SSID"].as<const char *>();
    _config.password = jsonDoc["AP_Default_PASS"].as<const char *>();
    // update the EEPROM
    memcpy(&storedCreds.ssid, &_config.ssid, sizeof(_config.ssid));
    memcpy(&storedCreds.password, &_config.password, sizeof(_config.password));
    // storedCreds.ssid = _config.ssid;
    // storedCreds.password = _config.password;

    saveCredentials(storedCreds);
  }
  else // update the SPIFFS AP values from the values that were stored
  {
    // memcpy(&_config.ssid, &storedCreds.ssid, sizeof(storedCreds.ssid));
    // memcpy(&_config.password, &storedCreds.password,  sizeof(storedCreds.password));
    _config.ssid = storedCreds.ssid;
    _config.password = storedCreds.password;
    saveCredentials(storedCreds);
  }

  _config.ip = IPAddress(jsonDoc["static_IP"][0], jsonDoc["static_IP"][1],
                         jsonDoc["static_IP"][2], jsonDoc["static_IP"][3]);
  _config.netmask = IPAddress(jsonDoc["netmask"][0], jsonDoc["netmask"][1],
                              jsonDoc["netmask"][2], jsonDoc["netmask"][3]);
  _config.gateway = IPAddress(jsonDoc["gatewayIP"][0], jsonDoc["gatewayIP"][1],
                              jsonDoc["gatewayIP"][2], jsonDoc["gatewayIP"][3]);
  _config.dns =
      IPAddress(jsonDoc["dns"][0], jsonDoc["dns"][1], jsonDoc["dns"][2], jsonDoc["dns"][3]);
  _config.dhcp = jsonDoc["dhcp"].as<bool>();
  _config.ntpServerName = jsonDoc["NTP_Server"].as<const char *>();
  _config.updateNTPTimeEvery = jsonDoc["NTP_Update_Interval"].as<long>();
  _config.timezone = jsonDoc["NTP_Time_Offset"].as<long>();
  _config.daylight = jsonDoc["NTP_Day_Light"].as<long>();
  _config.deviceName = calculated_Hostname();
  _config.mqtt_broker_ip =
      IPAddress(jsonDoc["MQTT_Broker"][0], jsonDoc["MQTT_Broker"][1],
                jsonDoc["MQTT_Broker"][2], jsonDoc["MQTT_Broker"][3]);
  _config.mqtt_QoS = jsonDoc["MQTT_QoS"].as<long>();
  _config.mqtt_port = jsonDoc["MQTT_Port"].as<long>();
  _config.mqtt_retain = jsonDoc["MQTT_Retain"].as<bool>();

  _config.Model = jsonDoc["Model"].as<const char *>();
  _config.Manufacturer = jsonDoc["Manufacturer"].as<const char *>();
  _config.Acquired = jsonDoc["Acquired"].as<const char *>();
  _config.TargetCT = jsonDoc["TargetCT"].as<float>();
  _config.debounceDelay = jsonDoc["debounceDelay"].as<long>();

  _config.WiFiMode = jsonDoc["ServerMode"].as<const char *>();

  //   _config.ssid =  "MILwrightAP";
  //   _config.password =  "MILwright_SVR!";
  //   _config.dhcp =1;
  //   _config.ip = IPAddress(192, 168, 100,154);
  //   _config.netmask = IPAddress(255, 255, 255, 0);
  //   _config.gateway = IPAddress(192, 168, 100, 1);
  //   _config.dns = IPAddress(192, 168, 100, 1);
  //   _config.ntpServerName = "192.168.100.1";
  //   _config.updateNTPTimeEvery = 15;
  //   _config.timezone = -80; // GMT-8 = Pacific timezone - San Francisco, CA
  //   _config.daylight = 1;
  //   // _config.deviceName = "ESP8266fs";
  //   WiFi.macAddress(mac);
  //   sprintf(macStr, "MILwright-%02X%02X",  mac[4], mac[5]);
  //   _config.deviceName  =String(macStr);
  //   //config.connectionLed = CONNECTION_LED;
  // _config.mqtt_broker_ip= IPAddress( 192, 168, 100, 1);
  // _config.mqtt_QoS = 0;
  // _config.mqtt_port= 1883;
  // _config.mqtt_retain = true;
  //       _config.meia = jsonDoc["meia"].as <const char *>();
  // _config.ueia="00000000";

  save_config();
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
  return true;
}

bool AsyncFSWebServer::save_config()
{
  // flag_config = false;
  DEBUG_MSG("Saving config\r\n");
  // DynamicJsonDocument jsonDoc(1024);
  StaticJsonDocument<1024> jsonDoc;
  // StaticJsonBuffer<1024> jsonBuffer;
  //JsonObject &json = jsonBuffer.createObject();
  jsonDoc["ssid"] = _config.ssid;
  jsonDoc["pass"] = _config.password;

  JsonArray jsonip = jsonDoc.createNestedArray("ip");
  jsonip.add(_config.ip[0]);
  jsonip.add(_config.ip[1]);
  jsonip.add(_config.ip[2]);
  jsonip.add(_config.ip[3]);

  JsonArray jsonNM = jsonDoc.createNestedArray("netmask");
  jsonNM.add(_config.netmask[0]);
  jsonNM.add(_config.netmask[1]);
  jsonNM.add(_config.netmask[2]);
  jsonNM.add(_config.netmask[3]);

  JsonArray jsonGateway = jsonDoc.createNestedArray("gateway");
  jsonGateway.add(_config.gateway[0]);
  jsonGateway.add(_config.gateway[1]);
  jsonGateway.add(_config.gateway[2]);
  jsonGateway.add(_config.gateway[3]);

  JsonArray jsondns = jsonDoc.createNestedArray("dns");
  jsondns.add(_config.dns[0]);
  jsondns.add(_config.dns[1]);
  jsondns.add(_config.dns[2]);
  jsondns.add(_config.dns[3]);

  jsonDoc["dhcp"] = _config.dhcp;
  jsonDoc["ntp"] = _config.ntpServerName;
  jsonDoc["NTPperiod"] = _config.updateNTPTimeEvery;
  jsonDoc["timeZone"] = _config.timezone;
  jsonDoc["daylight"] = _config.daylight;
  jsonDoc["deviceName"] = _config.deviceName;
  jsonDoc["WiFiMode"] = _config.WiFiMode;
  jsonDoc["mqtt_QoS"] = _config.mqtt_QoS;
  jsonDoc["mqtt_port"] = _config.mqtt_port;
  jsonDoc["mqtt_retain"] = _config.mqtt_retain;
  jsonDoc["Model"] = _config.Model;
  jsonDoc["Manufacturer"] = _config.Manufacturer;
  jsonDoc["Acquired"] = _config.Acquired;
  jsonDoc["TargetCT"] = _config.TargetCT;
  jsonDoc["debounceDelay"] = _config.debounceDelay;
  // jsonDoc["led"] = config.connectionLed;

  JsonArray jsonMQTT_Broker = jsonDoc.createNestedArray("mqtt_broker_ip");
  jsonMQTT_Broker.add(_config.mqtt_broker_ip[0]);
  jsonMQTT_Broker.add(_config.mqtt_broker_ip[1]);
  jsonMQTT_Broker.add(_config.mqtt_broker_ip[2]);
  jsonMQTT_Broker.add(_config.mqtt_broker_ip[3]);

  // jsonDoc["ueia"] = _config.ueia;
  // jsonDoc["meia"] = _config.meia;

  // TODO is this being written to the file
  File configFile = _fs->open(CONFIG_FILE, "w");
  if (!configFile)
  {
    DEBUG_MSG("Failed to open config file %s for writing\r\n", CONFIG_FILE);
    configFile.close();
    return false;
  }

#ifndef RELEASE
  String temp;
  serializeJsonPretty(jsonDoc, temp);
  Serial.println(temp);
  DEBUG_MSG("Length: %d\n\r", jsonDoc.memoryUsage());
#endif

  serializeJsonPretty(jsonDoc, configFile); // this writes to the file
  DEBUG_MSG("JSON written to file: '%s'\n\r", CONFIG_FILE);
  configFile.flush();
  configFile.close();
  DEBUG_MSG("File closed. \n\r");
  return true;
}

bool AsyncFSWebServer::loadHTTPAuth()
{
  File configFile = _fs->open(SECRET_FILE, "r");
  if (!configFile)
  {
    DEBUG_MSG("Failed to open secret file. So, disabling HTTPAuth? \r\n");
    _httpAuth.auth = false;
    _httpAuth.wwwUsername = "";
    _httpAuth.wwwPassword = "";
    configFile.close();
    return false;
  }

  size_t size = configFile.size();
  /*if (size > 256) {
      DEBUG_MSG("Secret file size is too large\r\n");
      httpAuth.auth = false;
      configFile.close();
      return false;
  }*/

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  configFile.close();
  DEBUG_MSG("JSON secret file size: %d bytes\r\n", size);
  DynamicJsonDocument jsonDoc(256);
  // StaticJsonBuffer<256> jsonBuffer;
  DeserializationError error = deserializeJson(jsonDoc, buf.get());

  if (error)
  {
#ifndef RELEASE
    String temp;
    serializeJsonPretty(jsonDoc, temp);
    DEBUG_MSG("%s\n\r", temp.c_str());
    DEBUG_MSG("Failed to parse secret file with error %d\r\n", error.code());
#endif // RELEASE
    _httpAuth.auth = false;
    return false;
  }
#ifndef RELEASE
  String temp;
  serializeJsonPretty(jsonDoc, temp);
  DEBUG_MSG("%s\n\r", temp.c_str());
#endif // RELEASE

  _httpAuth.auth = jsonDoc["auth"];
  _httpAuth.wwwUsername = jsonDoc["user"].as<char *>();
  _httpAuth.wwwPassword = jsonDoc["pass"].as<char *>();

  DEBUG_MSG(_httpAuth.auth ? "Secret initialized.\r\n" : "Auth disabled.\r\n");
  if (_httpAuth.auth)
  {
    DEBUG_MSG("User: %s\r\n", _httpAuth.wwwUsername.c_str());
    DEBUG_MSG("Pass: %s\r\n", _httpAuth.wwwPassword.c_str());
  }
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");

  return true;
}

void AsyncFSWebServer::handle()
{
  // ArduinoOTA.handle(); // Should this be here or in main loop?
  //   if(!NTP.update()) {
  //   NTP.forceUpdate();
  // }
  // If AP configuration mode (wifiAP_timer > 0) and no activity for > wifiAP_timeout then set to mode
  // STA and do configureWifi
  if ((wifiAP_timer > 0) && ((millis() - wifiAP_timer) > wifiAP_timeout))
  {
    DEBUG_MSG("Reached AP mode timeout, so going back to STA mode\r\n");
    _config.WiFiMode = "STA";
    wifiAP_timer = 0;
    save_config();
    configureWifi(); //<<< should this disconnect AP mode and (re)connect STA mode?
    DEBUG_MSG(__FUNCTION__);
    DEBUG_MSG("\r\n");
    _fs->end(); // SPIFFS.end();
    // send(200, "text/html", Page_Restart);
    // delay(3000);
    // ESP.restart();
  }
  // If attempt connection STA mode (wifiDisconnectedTime > 0) and no activity for > wifiDisconnected_timeout then set to mode
  // AP and configureWifiAP
  if ((wifiDisconnectedTime > 0) && ((millis() - wifiDisconnectedTime) > wifiDisconnected_timeout))
  {
    DEBUG_MSG("Reached STA attempt connect to configured SSID mode timeout, so changing to AP mode\r\n");
    wifiDisconnectedTime = 0;
    configureWifiAP();
  }
}

void AsyncFSWebServer::configureWifiAP()
{
  // NOTE: need to set/use same channel for WIFI_AP, WIFI_STA and WIFI_AP_STA so that
  //    WiFi radio is operating on same channel for the STA (channel is controlled/set by the Access Point)
  //    Potential Problem:>if not connected in WIFI_STA mode to an AP, then don't know the channel to use/set for the WIFI_AP mode
  //         - workaround/solution: explicit set the channel on the Gateway/AP (for RPi, have set to channel 6 )
  //         - explicitly setting channel with WIFI_CHANNEL
  // TODO if SoftAP then serve config pages else if STA mode then serve other set of pages.
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
  DEBUG_MSG("Start of configureWifi AP function...\r\n");
  // Set Red and Orange LED on to indicate now availible as AP
  digitalWrite(LED3, 1);
  digitalWrite(LED4, 1); // NOTE: if LED5 (pin15) is set and a reboot, ESP goes into reboot loop 

  // WiFi.softAPdisconnect(true); //is this supported even?
  // if (_config.WiFiMode == "AP")
  // {
  //   // WiFi.disconnect(true); // if explicitly set to AP mode then disconnect from the STA Mode and  station mode will be turned off
  //   WiFi.mode(WIFI_OFF);
  // }
  WiFi.mode(WIFI_OFF); // turn off the WiFi and then put into new mode
  // WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
  // WiFi.getMode(): return current Wi-Fi mode (one out of four modes above)

  WiFi.mode(WIFI_AP); // WIFI_AP - should be AP mode only. To put into STA+AP use mode= WIFI_AP_STA
  // String APname = _apConfig.APssid + (String)ESP.getChipId();
  String APname = _config.deviceName;
  if (_httpAuth.auth)
  {
    WiFi.softAP(APname.c_str(), _httpAuth.wwwPassword.c_str(), WIFI_CHANNEL);
    DEBUG_MSG("AP Pass enabled: %s\r\n", _httpAuth.wwwPassword.c_str());
  }
  else
  {
    // WiFi.softAP(APname.c_str());
    WiFi.softAP(APname.c_str(), "", WIFI_CHANNEL); // want to assign channel even if no password defined
    DEBUG_MSG("AP Password disabled - look for AP named: %s\r\n",
              APname.c_str());
  }
  if (CONNECTION_LED >= 0)
  {
    flashLED(CONNECTION_LED, 3, 150);
  }
  if (_config.WiFiMode == "STA")
  {                          // if preferred mode is STA and didn't connect, then start timer
    wifiAP_timer = millis(); // start the timer for being in the WiFiMode = AP
  }
  else
  {
    wifiAP_timer = 0;
  }                                 //if the desire is to be in <AP>mode then contuine indefinitely</AP>
  digitalWrite(Relay_Pin[0], HIGH); // turn off LED to indicate that not in STA mode (not connected to Gateway AP)
}

void AsyncFSWebServer::configureWifi()
{
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
  DEBUG_MSG("Start of configureWifi function...\r\n");
  DEBUG_MSG("Current WiFi mode is '%s'\r\n", wifi_mode_to_string(WiFi.getMode()));
  // Set Red and Orange LED on to indicate Not in AP mode
  digitalWrite(LED3, 0);
  digitalWrite(LED5, 0);
  // DEBUG_MSG(">>>configureWifi function...check if STA mode status \r\n");
  // if (WiFi.getMode() == WIFI_STA)
  // {
  //   // try connecting with existing credentials
  //   DEBUG_MSG("In STAtion mode, so let's begin...\r\n");
  //   Connect_Secure_Auth_AP();
  //   // WiFi.begin(_config.ssid.c_str(), _config.password.c_str());
  // }
  // else
  // {
  // WiFi.softAPdisconnect(true);
  // WiFi.disconnect(true); // seems like need to disconnect if changing AP points (SSID has changed) - but if trying to reconnect this will clear the known SSID/pw, so if want to change SSID great else this will reset the stored SSID/pw
  WiFi.mode(WIFI_OFF);
  DEBUG_MSG("Wifi radio now turned off...\r\n");
  // delay(1000); // this could be causing the soft WDT resets on long disconnects
  WiFi.mode(WIFI_STA);
  DEBUG_MSG("Wifi radio set to STA mode...\r\n");
  // delay(1000);
  // }

  // #ifndef RELEASED
  // int status = WL_IDLE_STATUS; // the Wifi radio's status, declare and initialize for debug

  // currentWifiStatus = WIFI_STA_DISCONNECTED;
  // scan for existing networks: shouldn't be needed except in debug mode
  // DEBUG_MSG("\r\nConfigWiFi()  - Scan available networks...\r\n");
  // WiFi.disconnect(); // this clears previous SSID and password from any autoreconnect
  // delay(100);
  // int networks_Available = WiFi.scanNetworks();
  // DEBUG_MSG("Found %d Networks...\r\n", networks_Available);
  Connect_Secure_Auth_AP();
  // if (_config.dhcp)
  // {
  //   // DEBUG_MSG("\rDHCP connect to %s with pw: %s\r\n", _config.ssid.c_str(),
  //   //           _config.password.c_str());
  DEBUG_MSG("returned from Connect_Secure_Auth_AP with status of %d\r\n", WiFi.status());
  // }
  if (!_config.dhcp)
  {
    DEBUG_MSG("starting with NO DHCP\r\n");
    WiFi.config(_config.ip, _config.gateway, _config.netmask, _config.dns);
  }
  // Connect_Secure_Auth_AP();
  // status = WiFi.begin(_config.ssid.c_str(), _config.password.c_str(), WIFI_CHANNEL);
  DEBUG_MSG("returned from WiFi.begin with status of %d\r\n", WiFi.status());
  // #endif

  wifiDisconnectedTime = millis(); // start the timer

  DEBUG_MSG("Start of wifi attempt at millis(): %lu\n\r", wifiDisconnectedTime);
  //
  // if (WiFi.waitForConnectResult() != WL_CONNECTED)
  // {
  //   DEBUG_MSG("WiFi Failed!\r\n");
  //   return;
  // }

  // //  If haven't connected within wifiDisconnected_timeout then go into AP mode
  // //

  long wifiTimer = 0;
  // while (!WiFi.isConnected())
  // { // this loop will causes a WDT if not successful with about 5 seconds
  //   // delay(5000);
  //   // wifiTimer=millis() - wifiAttemptedSince;
  //   DEBUG_MSG(".");
  // }
  DEBUG_MSG("\r\n");
  while (!WiFi.isConnected() && (millis() - wifiDisconnectedTime) < wifiDisconnected_timeout)
  {
    // how is this different or in conflict with WiFi.waitforConnectResult ?
    // #ifndef RELEASED
    DEBUG_MSG(" Current millis() is %lu and wifi attempt timer %lu with WiFI.status of  %d\r", millis(), wifiTimer, WiFi.status());
    //repeat the .begin
    // Try the configured SSID first
    // int status = WiFi.begin(_config.ssid.c_str(), _config.password.c_str(), WIFI_CHANNEL);
    // if (!status){
    Connect_Secure_Auth_AP(); // try connect to strongest "Authorized" AP
    // }
    // #endif
  }
  DEBUG_MSG(" >Time attempting to connect: %lu and current millis() is:%lu\n\r", (millis() - wifiDisconnectedTime), millis());

  // this should be already handled by onWiFiConnected and therefore not needed
  // if (WiFi.isConnected()) {
  //       // currentWifiStatus = WIFI_STA_CONNECTED;
  wifiDisconnectedTime = 0; // reset the timer
                            //   }
                            //
  // if (!WiFi.isConnected()) {// if still not connect then
  //   return;
  // }
  //
  DEBUG_MSG("WIFI_STA connection status = %d\r\n", WiFi.isConnected());
  DEBUG_MSG("IP Address: %s\r\n", WiFi.localIP().toString().c_str());
  DEBUG_MSG("Gateway:    %s\r\n", WiFi.gatewayIP().toString().c_str());
  DEBUG_MSG("DNS:        %s\r\n", WiFi.dnsIP().toString().c_str());
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::onWiFiConnected(WiFiEventStationModeConnected data)
{
  if (CONNECTION_LED >= 0)
  {
    digitalWrite(CONNECTION_LED, LOW); // Turn LED on
  }
  DEBUG_MSG("\n\rEvent: onWiFiConnected in STA mode -> LED pin %d is now 'ON'\n\r", CONNECTION_LED);
  // If re-connecting from a disconnect, then need to update NTP time
  // if (NTP.getTime() == 0)
  // { // 0 means couldn't connect to NTP server
  //   DEBUG_MSG("Error: Unable to getTime() %n \n\t- timeout or need to try NTP.begin()? \n\t\tHere is getTimeDateString()%s \n", NTP.getTime(), NTP.getTimeDateString().c_str());
  // }
  // else
  // {
  //   DEBUG_MSG("New time is: %s\n", NTP.getTimeDateString().c_str());
  // };

  // // turnLedOn();

  wifiDisconnectedTime = 0;
}

void AsyncFSWebServer::onWiFiDisconnected(
    // const WiFiEventStationModeDisconnected& event)
    WiFiEventStationModeDisconnected data)
{
  DEBUG_MSG("case onWiFiDisconnected >> STA disconnected for %d seconds \r\n", (int)((millis() - wifiDisconnectedTime) / 1000));

  if (CONNECTION_LED >= 0)
  {
    digitalWrite(CONNECTION_LED, HIGH); // Turn off LED
  }
  // DEBUG_MSG("Led %s off\n", CONNECTION_LED);
  flashLED(CONNECTION_LED, 2, 100);
  if (wifiDisconnectedTime == 0)
  {
    wifiDisconnectedTime = millis(); // start the timer for STA mode attempt
    // DEBUG_MSG("\r\nDisconnected for %d seconds\r\n",(int)((millis() - wifiDisconnectedTime) / 1000);
    DEBUG_MSG("Initial detection of STA disconnected...%d millseconds. \r\n", (int)((millis() - wifiDisconnectedTime)));
  }
}

void AsyncFSWebServer::handleFileList(AsyncWebServerRequest *request)
{
  if (!request->hasArg("dir"))
  {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = request->arg("dir");
  DEBUG_MSG("handleFileList: %s\r\n", path.c_str());
  Dir dir = _fs->openDir(path);
  path = String();

  String output = "[";
  while (dir.next())
  {
    File entry = dir.openFile("r");
    if (true) // entry.name()!="secret.json") // Do not show secrets
    {
      if (output != "[")
        output += ',';
      bool isDir = false;
      output += "{\"type\":\"";
      output += (isDir) ? "dir" : "file";
      output += "\",\"name\":\"";
      output += String(entry.name()).substring(1);
      output += "\"}";
    }
    entry.close();
  }

  output += "]";
  DEBUG_MSG("%s\r\n", output.c_str());
  request->send(200, "text/json", output);
}

String getContentType(String filename, AsyncWebServerRequest *request)
{
  if (request->hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".json"))
    return "application/json";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  else if (filename.endsWith(".jgz"))
    return "application/x-gzip";
  return "text/plain";
}

bool AsyncFSWebServer::handleFileRead(String path,
                                      AsyncWebServerRequest *request)
{
  DEBUG_MSG("handleFileRead: %s\r\n", path.c_str());
  if (CONNECTION_LED >= 0)
  {
    // CANNOT RUN DELAY() INSIDE CALLBACK
    flashLED(CONNECTION_LED, 2, 100); // Show activity on LED
  }
  if (wifiAP_timer > 0)
  {
    wifiAP_timer = millis();
  } // if wifiAP_timer is running, then update the timer to show activity

  if (path.endsWith("/"))
    path += "index.htm";
  String contentType = getContentType(path, request);
  String pathWithGz = path + ".gz";
  String pathWithJGz = path + ".jgz";
  if (_fs->exists(pathWithGz) || _fs->exists(pathWithJGz) ||
      _fs->exists(path))
  {   
  //   <!--local stored files-->
  //   <!-- <link rel="stylesheet" href="./css/pure-min.css"> -->
  //   <!-- <link rel="stylesheet" href="./css/gridsresponsivemin.css"> -->
  // 	<!-- <link rel="stylesheet" href="./css/CustomStyles.min.css"> -->

	// <link rel="stylesheet" href="./css/PaGoI-global.css">

      path += ".jgz";
    }
    DEBUG_MSG("Content type: %s\r\n", contentType.c_str());
    AsyncWebServerResponse *response =
        request->beginResponse(*_fs, path, contentType);
    if (path.endsWith(".gz") || path.endsWith(".jgz"))
    {
      response->addHeader("Content-Encoding", "gzip");
    }
    File file = SPIFFS.open(
        path,
        "r"); // why was this commented out - no file opened/read without it?
    DEBUG_MSG("File %s exists\r\n", path.c_str());
    request->send(response);

    return true;
  }
  else
    DEBUG_MSG("Cannot find %s\r\n", path.c_str());
  return false;
}

void AsyncFSWebServer::handleFileCreate(AsyncWebServerRequest *request)
{
  if (!checkAuth(request))
    return request->requestAuthentication();
  if (request->args() == 0)
    return request->send(500, "text/plain", "BAD ARGS");
  String path = request->arg(0U);
  DEBUG_MSG("handleFileCreate: %s\r\n", path.c_str());
  if (path == "/")
    return request->send(500, "text/plain", "BAD PATH");
  if (_fs->exists(path))
    return request->send(500, "text/plain", "FILE EXISTS");
  File file = _fs->open(path, "w");
  if (file)
    file.close();
  else
    return request->send(500, "text/plain", "CREATE FAILED");
  request->send(200, "text/plain", "");
  path = String(); // Remove? Useless statement?
}

void AsyncFSWebServer::handleFileDelete(AsyncWebServerRequest *request)
{
  if (!checkAuth(request))
    return request->requestAuthentication();
  if (request->args() == 0)
    return request->send(500, "text/plain", "BAD ARGS");
  String path = request->arg(0U);
  DEBUG_MSG("handleFileDelete: %s\r\n", path.c_str());
  if (path == "/")
    return request->send(500, "text/plain", "BAD PATH");
  if (!_fs->exists(path))
    return request->send(404, "text/plain", "FileNotFound");
  _fs->remove(path);
  request->send(200, "text/plain", "");
  path = String(); // Remove? Useless statement?
}

void AsyncFSWebServer::handleFileUpload(AsyncWebServerRequest *request,
                                        String filename, size_t index,
                                        uint8_t *data, size_t len, bool final)
{
  static File fsUploadFile;
  static size_t fileSize = 0;
  DEBUG_MSG("handleFileUpload Name: %s\r\n", filename.c_str());
  if (!index)
  { // Start

    if (!filename.startsWith("/"))
      filename = "/" + filename;
    fsUploadFile = _fs->open(filename, "w");
    DEBUG_MSG("First upload part.\r\n");
  }
  // Continue
  if (fsUploadFile)
  {
    DEBUG_MSG("Continue upload part. Size = %u\r\n", len);
    if (fsUploadFile.write(data, len) != len)
    {
      DEBUG_MSG("Write error during upload\n\r");
    }
    else
      fileSize += len;
  }
  for (size_t i = 0; i < len; i++)
  {
    if (fsUploadFile)
      fsUploadFile.write(data[i]);
  }
  if (final)
  { // End
    if (fsUploadFile)
    {
      fsUploadFile.flush();
      fsUploadFile.close();
    }
    DEBUG_MSG("handleFileUpload Size: %u\r\n", fileSize);
    fileSize = 0;
  }
}

// Base FSBrowserNG handlers
void AsyncFSWebServer::send_network_configuration_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");

  if (request->args() > 0) // Save Settings
  {
    // String temp = "";
    bool oldDHCP =
        _config.dhcp; // Save status to avoid general.htm clearing it
    _config.dhcp = false;

    storedCreds = loadCredentials();

    for (uint8_t i = 0; i < request->args(); i++)
    {
      DEBUG_MSG("Network configuration Arg %d: %s\r\n", i,
                request->arg(i).c_str());
      if (request->argName(i) == "devicename")
      {
        _config.deviceName = urldecode(request->arg(i));
        _config.dhcp = oldDHCP;
        continue;
      }
      if (request->argName(i) == "ssid")
      {
        _config.ssid = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "password")
      {
        _config.password = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "ip_0")
      {
        if (checkRange(request->arg(i)))
          _config.ip[0] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "ip_1")
      {
        if (checkRange(request->arg(i)))
          _config.ip[1] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "ip_2")
      {
        if (checkRange(request->arg(i)))
          _config.ip[2] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "ip_3")
      {
        if (checkRange(request->arg(i)))
          _config.ip[3] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "nm_0")
      {
        if (checkRange(request->arg(i)))
          _config.netmask[0] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "nm_1")
      {
        if (checkRange(request->arg(i)))
          _config.netmask[1] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "nm_2")
      {
        if (checkRange(request->arg(i)))
          _config.netmask[2] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "nm_3")
      {
        if (checkRange(request->arg(i)))
          _config.netmask[3] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "gw_0")
      {
        if (checkRange(request->arg(i)))
          _config.gateway[0] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "gw_1")
      {
        if (checkRange(request->arg(i)))
          _config.gateway[1] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "gw_2")
      {
        if (checkRange(request->arg(i)))
          _config.gateway[2] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "gw_3")
      {
        if (checkRange(request->arg(i)))
          _config.gateway[3] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "dns_0")
      {
        if (checkRange(request->arg(i)))
          _config.dns[0] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "dns_1")
      {
        if (checkRange(request->arg(i)))
          _config.dns[1] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "dns_2")
      {
        if (checkRange(request->arg(i)))
          _config.dns[2] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "dns_3")
      {
        if (checkRange(request->arg(i)))
          _config.dns[3] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "dhcp")
      {
        _config.dhcp = true;
        continue;
      }
    }
    bool config_saved = save_config();
    DEBUG_MSG("Configuration saved? %d\r\n", config_saved);
    // Update the credentials stored in EEPROM if they have changed
    DEBUG_MSG("SSID '%s', storedSSID '%s', size of _config.ssid %d storedCreds.ssid %d\n ", _config.ssid.c_str(), storedCreds.ssid, sizeof(_config.ssid), sizeof(storedCreds.ssid));
    if (memcmp(_config.ssid.c_str(), storedCreds.ssid, sizeof(_config.ssid)) != 0 || memcmp(_config.password.c_str(), storedCreds.password, sizeof(_config.password)) != 0)
    {
      memcpy(&storedCreds.ssid, &_config.ssid, sizeof(_config.ssid));
      memcpy(&storedCreds.password, &_config.password, sizeof(_config.password));
      saveCredentials(storedCreds);
    }
    // restart_esp(request);
    // TODO - enable on a restart so that it goes back into AP mode (or STA_AP)
    // ConfigureWifi();
    // AdminTimeOutCounter = 0;
  }
  else
  {
    DEBUG_MSG("url:%s\r\n", request->url().c_str());
    handleFileRead(request->url(), request);
  }
  handleFileRead(request->url(), request); // now reload the values on the page
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_general_configuration_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
  if (!checkAuth(request))
    return request->requestAuthentication();

  if (request->args() > 0) // Save Settings
  {
    for (uint8_t i = 0; i < request->args(); i++)
    {
      DEBUG_MSG("Arg %d: %s\r\n", i, request->arg(i).c_str());
      if (request->argName(i) == "devicename")
      {
        _config.deviceName = urldecode(request->arg(i));
        continue;
      }
    }
    save_config();
    // request->send(200, "text/html", Page_Restart);

    // _fs->end();
    // restart_esp(request);
    // ESP.restart();
    // ConfigureWifi();
    // AdminTimeOutCounter = 0;
  }
  else
  {
    handleFileRead(request->url(), request);
  }
  handleFileRead(request->url(), request); // now reload the values on the page
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_module_configuration_html(
    AsyncWebServerRequest *request)
{
  // var elementID = ["Model", "Manufacturer", "Acquired", "TargetCT", "Radio_ID"];
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
  if (!checkAuth(request))
    return request->requestAuthentication();

  if (request->args() > 0) // Save Settings -<<< this approach might work - but better to spend time getting into JSON
                           // for (int i = 0;  i < element_length; i++) {
                           // 	switch (request->argName(i)) {
                           // 		case 'Model': // Model
                           //       _config.Model = request->arg(i);
                           // 		case 'Manufacturer': // Manufacturer
                           //       _config.Model = request->arg(i);
                           // 		case 'Acquired': // Acquired Date
                           // 			_config.Acquired = request->arg(i);
                           // }
  {
    for (uint8_t i = 0; i < request->args(); i++)
    {
      DEBUG_MSG("Arg %d: %s\r\n", i, request->arg(i).c_str());
      if (request->argName(i) == "Radio_ID")
      {
        _config.deviceName = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "Model")
      {
        _config.Model = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "Manufacturer")
      {
        _config.Manufacturer = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "Acquired")
      {
        _config.Acquired = urldecode(request->arg(i));
        continue;
      }
      if (request->argName(i) == "TargetCT")
      {
        _config.TargetCT = request->arg(i).toFloat();
        continue;
      }
    }
    save_config();
  }
  // else
  // {
  //   handleFileRead(request->url(), request);
  // }
  // handleFileRead("/ntp.htm", request);
  handleFileRead(request->url(), request); // now reload the values on the page
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_NTP_configuration_values_html(
    AsyncWebServerRequest *request)
{
  // Put the values into the html form
  String values = "";
  values += "ntpserver|" + (String)_config.ntpServerName + "|input\n";
  values += "update|" + (String)_config.updateNTPTimeEvery + "|input\n";
  values += "tz|" + (String)_config.timezone + "|input\n";
  values += "dst|" + (String)(_config.daylight ? "checked" : "") + "|chk\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";
  request->send(200, "text/plain", values);
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_general_configuration_values_html(
    AsyncWebServerRequest *request)
{
  String values = "";
  values += "devicename|" + (String)_config.deviceName + "|input\n";
  values += "firmwareVersion|" + (String)APP_VERSION + "|div\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";
  request->send(200, "text/plain", values);
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_host_configuration_values_html(
    AsyncWebServerRequest *request)
{
  String values = "";
  values += "devicename| Device name: " + (String)_config.deviceName + "|div\n";
  values += "firmwareVersion| Firmware Version: " + (String)APP_VERSION + "|div\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";
  request->send(200, "text/plain", values);
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_module_configuration_values_html(
    AsyncWebServerRequest *request)
{
  String values = "";
  values += "Radio_ID|" + (String)_config.deviceName + "|input\n";
  // values += "firmwareVersion| Version:" + (String)APP_VERSION + "|div\n";
  values += "Model|" + _config.Model + "|input\n";
  values += "Manufacturer|" + _config.Manufacturer + "|input\n";
  values += "Temperature|" + LastTemp + "|input\n";
  values += "Humidity|" + LastHumid + "|input\n";
  values += "Acquired|" + _config.Acquired + "|input\n";
  values += "TargetCT|" + (String)_config.TargetCT + "|input\n";
  values += "LastCT|" + String(CycleTime) + "|input\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";
  request->send(200, "text/plain", values);
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_network_configuration_values_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG("\n\rstarting network configuration for output\n");
  String values = "";

  values += "ssid|" + (String)_config.ssid + "|input\n";
  values += "password|" + (String)_config.password + "|input\n";
  values += "ip_0|" + (String)_config.ip[0] + "|input\n";
  values += "ip_1|" + (String)_config.ip[1] + "|input\n";
  values += "ip_2|" + (String)_config.ip[2] + "|input\n";
  values += "ip_3|" + (String)_config.ip[3] + "|input\n";
  values += "nm_0|" + (String)_config.netmask[0] + "|input\n";
  values += "nm_1|" + (String)_config.netmask[1] + "|input\n";
  values += "nm_2|" + (String)_config.netmask[2] + "|input\n";
  values += "nm_3|" + (String)_config.netmask[3] + "|input\n";
  values += "gw_0|" + (String)_config.gateway[0] + "|input\n";
  values += "gw_1|" + (String)_config.gateway[1] + "|input\n";
  values += "gw_2|" + (String)_config.gateway[2] + "|input\n";
  values += "gw_3|" + (String)_config.gateway[3] + "|input\n";
  values += "dns_0|" + (String)_config.dns[0] + "|input\n";
  values += "dns_1|" + (String)_config.dns[1] + "|input\n";
  values += "dns_2|" + (String)_config.dns[2] + "|input\n";
  values += "dns_3|" + (String)_config.dns[3] + "|input\n";
  values += "dhcp|" + (String)(_config.dhcp ? "checked" : "") + "|chk\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";

  request->send(200, "text/plain", values);
  values = "";

  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_connection_state_values_html(
    AsyncWebServerRequest *request)
{

  String state = "N/A";
  String Networks = "";
  if (WiFi.status() == 0)
    state = "Idle";
  else if (WiFi.status() == 1)
    state = "NO SSID AVAILBLE";
  else if (WiFi.status() == 2)
    state = "SCAN COMPLETED";
  else if (WiFi.status() == 3)
    state = "CONNECTED";
  else if (WiFi.status() == 4)
    state = "CONNECT FAILED";
  else if (WiFi.status() == 5)
    state = "CONNECTION LOST";
  else if (WiFi.status() == 6)
    state = "DISCONNECTED";

  WiFi.scanNetworks(true);

  String values = "";
  values += "connectionstate|" + state + "|div\n";
  // values += "networks|Scanning networks ...|div\n";
  request->send(200, "text/plain", values);
  state = "";
  values = "";
  Networks = "";
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_information_values_html(
    AsyncWebServerRequest *request)
{

  String values = "";

  values += "x_ssid|" + (String)WiFi.SSID() + "|div\n";
  values += "x_ip|" + (String)WiFi.localIP()[0] + "." +
            (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." +
            (String)WiFi.localIP()[3] + "|div\n";
  values += "x_gateway|" + (String)WiFi.gatewayIP()[0] + "." +
            (String)WiFi.gatewayIP()[1] + "." + (String)WiFi.gatewayIP()[2] +
            "." + (String)WiFi.gatewayIP()[3] + "|div\n";
  values += "x_netmask|" + (String)WiFi.subnetMask()[0] + "." +
            (String)WiFi.subnetMask()[1] + "." + (String)WiFi.subnetMask()[2] +
            "." + (String)WiFi.subnetMask()[3] + "|div\n";
  values += "x_mac|" + getMac_address_s() + "|div\n";
  values += "x_dns|" + (String)WiFi.dnsIP()[0] + "." + (String)WiFi.dnsIP()[1] +
            "." + (String)WiFi.dnsIP()[2] + "." + (String)WiFi.dnsIP()[3] +
            "|div\n";
  values +=
      "x_ntp_sync|" + NTP.getTimeDateString(NTP.getLastNTPSync()) + "|div\n";
  values += "x_ntp_time|" + NTP.getTimeStr() + "|div\n";
  values += "x_ntp_date|" + NTP.getDateStr() + "|div\n";
  values += "x_uptime|" + NTP.getUptimeString() + "|div\n";
  values +=
      "x_last_boot|" + NTP.getTimeDateString(NTP.getLastBootTime()) + "|div\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";      

  request->send(200, "text/plain", values);
  // delete &values;
  values = "";
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

String AsyncFSWebServer::getMac_address_s()
{
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
  return String(macStr);
}

// convert a single hex digit character to its integer value (from
// https://code.google.com/p/avr-netino/)
unsigned char AsyncFSWebServer::h2int(char c)
{
  if (c >= '0' && c <= '9')
  {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f')
  {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F')
  {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

String AsyncFSWebServer::urldecode(
    String input) // (based on https://code.google.com/p/avr-netino/)
{
  char c;
  String ret = "";

  for (byte t = 0; t < input.length(); t++)
  {
    c = input[t];
    if (c == '+')
      c = ' ';
    if (c == '%')
    {

      t++;
      c = input[t];
      t++;
      c = (h2int(c) << 4) | h2int(input[t]);
    }

    ret.concat(c);
  }
  return ret;
}

//
// Check the Values is between 0-255
//
boolean AsyncFSWebServer::checkRange(String Value)
{
  if (Value.toInt() < 0 || Value.toInt() > 255)
  {
    return false;
  }
  else
  {
    return true;
  }
}

// Marin Digital added page handling:

void AsyncFSWebServer::send_mqtt_configuration_values_html(
    AsyncWebServerRequest *request)
{

  String values = "";

  values +=
      "mqtt_broker_ip_0|" + (String)_config.mqtt_broker_ip[0] + "|input\n";
  values +=
      "mqtt_broker_ip_1|" + (String)_config.mqtt_broker_ip[1] + "|input\n";
  values +=
      "mqtt_broker_ip_2|" + (String)_config.mqtt_broker_ip[2] + "|input\n";
  values +=
      "mqtt_broker_ip_3|" + (String)_config.mqtt_broker_ip[3] + "|input\n";
  values += "mqtt_QoS|" + (String)(_config.mqtt_QoS) + "|input\n";
  values += "mqtt_port|" + (String)(_config.mqtt_port) + "|input\n";
  values += "mqtt_retain|" + (String)(_config.mqtt_retain ? "checked" : "") +
            "|chk\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";
  request->send(200, "text/plain", values);

  values = "";
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
};

void AsyncFSWebServer::send_mqtt_configuration_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG(__FUNCTION__);

  if (!checkAuth(request))
    return request->requestAuthentication();

  if (request->args() > 0) // Save Settings
  {
    _config.mqtt_retain = false; // default value unless present in string
    for (uint8_t i = 0; i < request->args(); i++)
    {
      DEBUG_MSG("Arg %d: Name: %s with Value: %s\r\n", i,
                request->argName(i).c_str(), request->arg(i).c_str());

      if (request->argName(i) == "mqtt_broker_ip_0")
      {
        if (checkRange(request->arg(i)))
          _config.mqtt_broker_ip[0] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_broker_ip_1")
      {
        if (checkRange(request->arg(i)))
          _config.mqtt_broker_ip[1] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_broker_ip_2")
      {
        if (checkRange(request->arg(i)))
          _config.mqtt_broker_ip[2] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_broker_ip_3")
      {
        if (checkRange(request->arg(i)))
          _config.mqtt_broker_ip[3] = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_QoS")
      {
        _config.mqtt_QoS = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_port")
      {
        _config.mqtt_port = request->arg(i).toInt();
        continue;
      }
      if (request->argName(i) == "mqtt_retain")
      {
        _config.mqtt_retain = true;
        continue;
      } // parameter only appears if set
    }
    save_config();
    // request->send(200, "text/html", Page_WaitAndReload);
    DEBUG_MSG(__FUNCTION__);
    DEBUG_MSG(" - end \r\n");
    // restart_esp(request);
    // yield();
    // delay(3000);
    // // _fs->end();
    // ESP.restart();
    // ConfigureWifi();
    // AdminTimeOutCounter = 0;
  }
  else
  {
    handleFileRead(request->url(), request);

    DEBUG_MSG(__PRETTY_FUNCTION__);
    DEBUG_MSG("\r\n");
  }
  handleFileRead(request->url(), request); // now reload the values on the page
}

void AsyncFSWebServer::send_NTP_configuration_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG("NTP Update Processing ...\r\n");
  if (!checkAuth(request))
    return request->requestAuthentication();
  if (request->args() > 0) // Save Settings
  {                        // update NTP settings - no need to restart though
    DEBUG_MSG("NTP process parms ...\r\n");
    _config.daylight = false;
    // String temp = "";
    for (uint8_t i = 0; i < request->args(); i++)
    {
      if (request->argName(i) == "ntpserver")
      {
        _config.ntpServerName = urldecode(request->arg(i));
        NTP.setNtpServerName(_config.ntpServerName);
        continue;
      }
      if (request->argName(i) == "update")
      {
        _config.updateNTPTimeEvery = request->arg(i).toInt();
        NTP.setInterval(_config.updateNTPTimeEvery * 60);
        continue;
      }
      if (request->argName(i) == "tz")
      {
        _config.timezone = request->arg(i).toInt();
        NTP.setTimeZone(_config.timezone / 10);
        continue;
      }
      if (request->argName(i) == "dst")
      {
        _config.daylight = true;
        DEBUG_MSG("Daylight Saving: %d\r\n", _config.daylight);
        continue;
      }
    }

    NTP.setDayLight(_config.daylight);
    save_config();
    // firstStart = true;
    // DEBUG_MSG("time before reset: %d", )
    DEBUG_MSG("time of last boot:%s and last sync: %s\r\n", NTP.getTimeDateString(NTP.getLastBootTime()).c_str(), NTP.getTimeDateString(NTP.getLastNTPSync()).c_str());
    setTime(NTP.getTime()); // set time
    DEBUG_MSG("time after set time:%s and last sync: %s\r\n", NTP.getTimeDateString().c_str(), NTP.getTimeDateString(NTP.getLastNTPSync()).c_str());
  }
  // handleFileRead("/ntp.htm", request);
  handleFileRead(request->url(), request); // now reload the values on the page
  // server.send(200, "text/html", PAGE_NTPConfiguration);
  DEBUG_MSG(__PRETTY_FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::restart_esp(AsyncWebServerRequest *request)
{
  // request->send(200, "text/html", Page_Restart);
  // request->send(200, "text/html", Page_WaitAndReload);

  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
  _fs->end(); // SPIFFS.end();
  request->send(200, "text/html", Page_Restart);
  DEBUG_MSG("ESP restarting....");
  // ESP.restart();
}

void AsyncFSWebServer::send_wwwauth_configuration_values_html(
    AsyncWebServerRequest *request)
{
  String values = "";

  values += "wwwauth|" + (String)(_httpAuth.auth ? "checked" : "") + "|chk\n";
  values += "wwwuser|" + (String)_httpAuth.wwwUsername + "|input\n";
  values += "wwwpass|" + (String)_httpAuth.wwwPassword + "|input\n";
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";

  request->send(200, "text/plain", values);

  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::send_wwwauth_configuration_html(
    AsyncWebServerRequest *request)
{
  DEBUG_MSG("%s %d\n", __FUNCTION__, request->args());
  if (request->args() > 0) // Save Settings
  {
    _httpAuth.auth = false;
    // String temp = "";
    for (uint8_t i = 0; i < request->args(); i++)
    {
      if (request->argName(i) == "wwwuser")
      {
        _httpAuth.wwwUsername = urldecode(request->arg(i));
        DEBUG_MSG("User: %s\r\n", _httpAuth.wwwUsername.c_str());
        continue;
      }
      if (request->argName(i) == "wwwpass")
      {
        _httpAuth.wwwPassword = urldecode(request->arg(i));
        DEBUG_MSG("Pass: %s\r\n", _httpAuth.wwwPassword.c_str());
        continue;
      }
      if (request->argName(i) == "wwwauth")
      {
        _httpAuth.auth = true;
        DEBUG_MSG("HTTP Auth enabled\r\n");
        continue;
      }
    }

    saveHTTPAuth();
  }
  handleFileRead("/system.htm", request);

  // DEBUG_MSG(__PRETTY_FUNCTION__);
  // DEBUG_MSG("\r\n");
}

bool AsyncFSWebServer::saveHTTPAuth()
{
  // flag_config = false;
  DEBUG_MSG("Save secret\r\n");
  DynamicJsonDocument jsonDoc(256);
  // StaticJsonBuffer<256> jsonBuffer;
  //JsonObject &json = jsonBuffer.createObject();
  jsonDoc["auth"] = _httpAuth.auth;
  jsonDoc["user"] = _httpAuth.wwwUsername;
  jsonDoc["pass"] = _httpAuth.wwwPassword;

  // TODO add AP data to html
  File configFile = _fs->open(SECRET_FILE, "w");
  if (!configFile)
  {
    DEBUG_MSG("Failed to open secret file for writing\r\n");
    configFile.close();
    return false;
  }

#ifndef RELEASE
  String temp;
  serializeJsonPretty(jsonDoc, temp);
  Serial.println(temp);
#endif // RELEASE

  serializeJsonPretty(jsonDoc, configFile);
  configFile.flush();
  configFile.close();
  return true;
}

void AsyncFSWebServer::send_update_firmware_values_html(
    AsyncWebServerRequest *request)
{
  String values = "";
#ifndef RELEASED
  uint32_t maxSketchSpace = (ESP.getSketchSize() - 0x1000) & 0xFFFFF000;
#endif
  // bool updateOK = Update.begin(maxSketchSpace);
  // actually want to only check if there is room for the new sketch
  // bool updateOK = maxSketchSpace < ESP.getFreeSketchSpace();
  bool updateOK = true;
  StreamString result;
  Update.printError(result);
  DEBUG_MSG("--MaxSketchSpace: %d\n", maxSketchSpace);
  DEBUG_MSG("--getSketchSize: %d\n", ESP.getSketchSize());
  DEBUG_MSG("--Update error = %s\r\n", result.c_str());
  values += "remupd|" + (String)((updateOK) ? "OK" : "Not enough free space!") + "|div\n";

  if (Update.hasError() > 0)
  {
    result.trim();
    values += "remupdResult|" + result + "|div\n";
  }
  else
  {
    values += "remupdResult|Current build: ";
    values += buildtimestamp_s.c_str();
    values += "\nBrowse and select new firmware file.|div\n";
  }
  values += "brand| " + (String)PAGOI_CLIENT + "|div\n";

  request->send(200, "text/plain", values);
  DEBUG_MSG(__FUNCTION__);
  DEBUG_MSG("\r\n");
}

void AsyncFSWebServer::setUpdateMD5(AsyncWebServerRequest *request)
{
  _browserMD5 = "";
  DEBUG_MSG("Arg number: %d\r\n", request->args());
  if (request->args() > 0) // Read hash
  {
    for (uint8_t i = 0; i < request->args(); i++)
    {
      DEBUG_MSG("Arg %s: %s\r\n", request->argName(i).c_str(),
                request->arg(i).c_str());
      if (request->argName(i) == "md5")
      {
        _browserMD5 = urldecode(request->arg(i));
        Update.setMD5(_browserMD5.c_str());
        continue;
      }
      if (request->argName(i) == "size")
      {
        _updateSize = request->arg(i).toInt();
        DEBUG_MSG("Update size: %d\r\n", _updateSize);
        continue;
      }
    }
    request->send(200, "text/html", "OK --> MD5: " + _browserMD5);
  }
}

void AsyncFSWebServer::updateFirmware(AsyncWebServerRequest *request,
                                      String filename, size_t index,
                                      uint8_t *data, size_t len, bool final)
{
  // handler for the file upload, get's the sketch bytes, and writes
  // them through the Update object
  static long totalSize = 0;
  if (!index)
  { // UPLOAD_FILE_START
    SPIFFS.end();
    Update.runAsync(true);
    DEBUG_MSG("Update starting for filename: %s\r\n", filename.c_str());
#ifndef RELEASED
    uint32_t maxSketchSpace = ESP.getSketchSize();
    DEBUG_MSG("Max free program space: %u\r\n", maxSketchSpace);
    DEBUG_MSG("New program size: %u\r\n", _updateSize);
#endif
    if (_browserMD5 != NULL && _browserMD5 != "")
    {
      Update.setMD5(_browserMD5.c_str());
      DEBUG_MSG("Hash from client: %s\r\n", _browserMD5.c_str());
    }
    if (!Update.begin(_updateSize))
    { // start with max available size
      // Update.printError(DEBUG_PORT);
      Update.printError(Serial);
    }
  }

  // Get upload file, continue if not start
  totalSize += len;
  DEBUG_MSG(".");
  digitalWrite(LED5, !digitalRead(LED5));
  size_t written = Update.write(data, len);
  if (written != len)
  {
    DEBUG_MSG("len = %d, written = %zu, totalSize = %lu", len, written, totalSize);
    Update.printError(Serial);
    // Update.printError(DBG_OUTPUT);
    // return;
  }
  if (final)
  { // UPLOAD_FILE_END
    String updateHash;
    digitalWrite(LED5, 0); // if LED5 (15 is 1, the ESP will go into a boot from SD loop)
    DEBUG_MSG("\r\nApplying update...\n\r");
    if (Update.end(true))
    { // true to set the size to the current progress
      updateHash = Update.md5String();
      DEBUG_MSG("Upload finished. Calculated MD5: %s\r\n", updateHash.c_str());
      DEBUG_MSG("Update Success: %u\nRebooting...\n", request->contentLength());
    }
    else
    {
      updateHash = Update.md5String();
      DEBUG_MSG("Upload failed. Calculated MD5: %s\r\n", updateHash.c_str());
      // Update.printError(DEBUG_PORT);
      Update.printError(Serial);
    }
  }

  // delay(2);
}

void AsyncFSWebServer::serverInit()
{
  // SERVER INIT
  //  initialize the listeners for processes
  DEBUG_MSG("initialize server listeners....\n");
  serveStatic("/", SPIFFS, "/", "max-age=600");
  // list directory
  on("/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->handleFileList(request);
  });
  // load editor
  on("/edit", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    if (!this->handleFileRead("/edit.htm", request))
      request->send(404, "text/plain", "FileNotFound");
  });
  // create file
  on("/edit", HTTP_PUT, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->handleFileCreate(request);
  }); //delete file
  on("/edit", HTTP_DELETE, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->handleFileDelete(request);
  });
  // first callback is called after the request has ended with all parsed
  // arguments  second callback handles file uploads at that location
  on("/edit", HTTP_POST,
     [](AsyncWebServerRequest *request) {
       request->send(200, "text/plain", "");
     },
     [this](AsyncWebServerRequest *request, String filename, size_t index,
            uint8_t *data, size_t len, bool final) {
       this->handleFileUpload(request, filename, index, data, len, final);
     });
  on("/admin/generalvalues", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_general_configuration_values_html(request);
  });
  on("/admin/hostvalues", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_host_configuration_values_html(request);
  });
  on("/admin/networkvalues", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_network_configuration_values_html(request);
  });
  on("/admin/connectionstate", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_connection_state_values_html(request);
  });
  on("/admin/infovalues", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_information_values_html(request);
  });
  on("/admin/ntpvalues", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_NTP_configuration_values_html(request);
  });
  on("/admin/mqttvalues", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_mqtt_configuration_values_html(request);
  });
  on("/admin/modulevalues", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_module_configuration_values_html(request);
  });
  on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "[";
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED)
    {
      WiFi.scanNetworks(true);
    }
    else if (n)
    {
      for (int i = 0; i < n; ++i)
      {
        if (i)
          json += ",";
        json += "{";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
        json += ",\"channel\":" + String(WiFi.channel(i));
        json += ",\"secure\":" + String(WiFi.encryptionType(i));
        json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
        json += "}";
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == WIFI_SCAN_FAILED)
      {
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
    request->send(200, "text/json", json);
    json = "";
  });
  on("/general.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_general_configuration_html(request);
  });
  on("/config_network.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_network_configuration_html(request);
  });
  on("/config_mqtt.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_mqtt_configuration_html(request);
  });
  on("/ntp.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    DEBUG_MSG("NTP process should happen\n\r");
    this->send_NTP_configuration_html(request);
  });
  on("/config_module.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_module_configuration_html(request);

  });
  on("/admin/restart", [this](AsyncWebServerRequest *request) {
    DEBUG_MSG("%s\r\n", request->url().c_str());
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    handleFileRead("/Rebooting.htm", request);
    // delay(3000);
    // if (!this->handleFileRead("/Rebooting.htm", request))
    //   request->send(404, "text/plain", "FileNotFound");
    // this->restart_esp(request);
    // ESP.restart();
    requestRestartMillis=millis();
  });
  on("/admin/wwwauth", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_wwwauth_configuration_values_html(request);
  });
  on("/admin", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    if (!this->handleFileRead("/admin.htm", request))
      request->send(404, "text/plain", "FileNotFound");
  });
  on("/system.htm", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_wwwauth_configuration_html(request);
  });
  on("/update/updatepossible", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    this->send_update_firmware_values_html(request);
  });
  on("/setmd5", [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    // DBG_OUTPUT.println("md5?");
    this->setUpdateMD5(request);
  });
  on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    if (!this->handleFileRead("/OTA_update.htm", request))
      request->send(404, "text/plain", "FileNotFound");
  });
  on("/update", HTTP_POST,
     [this](AsyncWebServerRequest *request) {
       if (!this->checkAuth(request))
         return request->requestAuthentication();
       AsyncWebServerResponse *response = request->beginResponse(
           200, "text/html",
           (Update.hasError()) ? "FAIL"
                               : "<META http-equiv=\"refresh\" "
                                 "content=\"10;URL=/Rebooting.htm\">Update correct. "
                                 "Restarting...");
       response->addHeader("Connection", "close");
       response->addHeader("Access-Control-Allow-Origin", "*");
       request->send(response);
       this->_fs->end();
      //  ESP.restart();
      requestRestartMillis=millis();

     },
     [this](AsyncWebServerRequest *request, String filename, size_t index,
            uint8_t *data, size_t len, bool final) {
       this->updateFirmware(request, filename, index, data, len, final);
     });

  // // called when the url is not defined here
  // use it to load content from SPIFFS
  onNotFound([this](AsyncWebServerRequest *request) {
    DEBUG_MSG("URL processing not defined: %s\r\n", request->url().c_str());
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    // if it is file then reply with file contents, otherwise respond with this
    // FileNotFound message
    if (!this->handleFileRead(request->url(), request))
      request->send(404, "text/plain", "FileNotFound");
    delete response; // Free up memory!
  });

  _evs.onConnect([](AsyncEventSourceClient *client) {
    DEBUG_MSG("Event source wifiClient connected from %s\r\n",
              client->client()->remoteIP().toString().c_str());
  });
  addHandler(&_evs);

#define HIDE_SECRET
#ifdef HIDE_SECRET
  on(SECRET_FILE, HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    AsyncWebServerResponse *response =
        request->beginResponse(403, "text/plain", "Forbidden");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
#endif // HIDE_SECRET

#ifdef HIDE_CONFIG
  on(CONFIG_FILE, HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!this->checkAuth(request))
      return request->requestAuthentication();
    AsyncWebServerResponse *response =
        request->beginResponse(403, "text/plain", "Forbidden");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
#endif // HIDE_CONFIG

  // get heap status, analog input value and all GPIO statuses in one json call
  on("/all", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" +
            String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    request->send(200, "text/json", json);
    json = String();
  });
  // server.begin(); --> Not here
  DEBUG_MSG("HTTP server started\r\n");
}

bool AsyncFSWebServer::checkAuth(AsyncWebServerRequest *request)
{
  if (!_httpAuth.auth)
  {
    return true;
  }
  else
  {
    return request->authenticate(_httpAuth.wwwUsername.c_str(),
                                 _httpAuth.wwwPassword.c_str());
  }
}

// Support functions
void AsyncFSWebServer::printWifiData()
{
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(subnet);

  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway);
}

void AsyncFSWebServer::printCurrentNet()
{
  // print the SSID of the network you're attached to:
  Serial.print("Currently connected to SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  Serial.println(WiFi.BSSIDstr());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // // print the encryption type:
  // byte encryption = WiFi.encryptionType();// not supported in ESP8266 SDK API
  // Serial.print("Encryption Type:");
  // Serial.println(encryption,HEX);
}

void AsyncFSWebServer::Connect_Secure_Auth_AP()
{
  /*
  Goal: 
     1. Disconnect from network and scan for availalbe AP/Gateway 
        - only connect to Authorized AP
        - connect to strongest Authorized AP
          - "Authorized" means includes MILwrightAP in SSID - isApprovedAP(j_th_AP)
  Problem trying to solve: 
    - otherwise reconnect will connect to the last connected AP even if it is in the "other" room

*/
  WiFi.disconnect(); // disconnect from current AP (WiFi_STA mode)
  delay(100);
  // Sets currently configured SSID and password to null values and disconnects the station from an access point.
  WiFi.scanNetworks(true); // this also takes out of WiFi_AP mode
  DEBUG_MSG("\nLooking for approved AP's. Scan start for AP's with prefix: '%s'\n\r ", WIFI_APPROVED_AP_PREFIX);

  // print out Wi-Fi network scan result upon completion
  while (WiFi.scanComplete() < 0)
  {
    DEBUG_MSG(".");
    delay(500);
  }
  int n = WiFi.scanComplete();
  if (n >= 0)
  {
    DEBUG_MSG("%d network(s) found\n", n);
    //      int i=0;
    int SigStrength[n][n]; //maximum of 10 access points.
    for (int i = 0; i < n; i++)
    {
      //initialise array
      //2 columms column 0 is the signal Strength, column 1 is index
      SigStrength[1][i] = i;
      SigStrength[0][i] = WiFi.RSSI(i);
      DEBUG_MSG("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }

    for (int i = 1; i < n; ++i)
    {                            // insert sort into strongest signal
      int j = SigStrength[0][i]; // hold value for signal strength
      int l = SigStrength[1][i]; // index of the AP
      int k;
      for (k = i - 1; (k >= 0) && (j > SigStrength[0][k]); k--)
      {
        SigStrength[0][k + 1] = SigStrength[0][k];
        l = SigStrength[1][k + 1];
        SigStrength[1][k + 1] = SigStrength[1][k];
        SigStrength[1][k] = l; //swap index values here.
      }
      SigStrength[0][k + 1] = j;
      SigStrength[1][k + 1] = l; //swap index values here to re-order.
    }
    DEBUG_MSG("List of Access Points by Strength\n");
    for (int i = 0; i < n; i++)
    {
      int l = SigStrength[1][i];
      DEBUG_MSG("%d: '%s', Ch:%d (%ddBm)AP_mac: %s %s\n", i + 1, WiFi.SSID(l).c_str(), WiFi.channel(l), WiFi.RSSI(l), WiFi.BSSIDstr(l).c_str(), WiFi.encryptionType(l) == ENC_TYPE_NONE ? "open" : "secure");
      //            Serial.printf("SigStrength 0: %d, SigStrength 1: %d\n", SigStrength[0][i], SigStrength[1][i]);
    }
    int j = 0;
    while ((j < n) && (WiFi.status() != WL_CONNECTED)) // try to connect to an approved AP until successful
    {
      // DEBUG_MSG("Connecting: %s\n\r", WiFi.SSID(SigStrength[1][j])); //debugging
      int k = 0;
      String j_th_AP = WiFi.SSID(SigStrength[1][j]);
      DEBUG_MSG("Access Point %s Approved? %d or Selected: %d\n\r", j_th_AP.c_str(), isApprovedAP(j_th_AP), _config.ssid == j_th_AP.c_str());
      //skip AP if not "Approved" - approved means has correct prefix in SSID and uses the default password
      if (isApprovedAP(j_th_AP) || _config.ssid == j_th_AP.c_str())
      {
        // WiFi.begin(j_th_AP.c_str(), _config.password.c_str()); // Note: may need to accept channel that access point is set to, hopefully, AP should be configured to WIFI_CHANNEL
        WiFi.begin(j_th_AP.c_str(), _config.password.c_str(), WIFI_CHANNEL); //connect to the j'th value in AP array on our desired WIFI_CHANNEL - part of the "Authorized" logic
        // WiFi.begin(_config.ssid.c_str(), _config.password.c_str());
        while ((WiFi.status() != WL_CONNECTED) && k < 15)
        {
          DEBUG_MSG("k");
          delay(500);
          k++; //counter for up to 8 seconds - why 8 ? who knows
        }
        //         if (WiFi.waitForConnectResult() != WL_CONNECTED) // alternative method for doing the timeout
        // {
        //   DEBUG_MSG("WiFi Failed!\r\n");
        //   return;
        // }
      }
      DEBUG_MSG("\n\r");
      if (k > 15)
        DEBUG_MSG("Timeout so, try NEXT Wifi Accest Point\n\r");
      j++; //choose the next SSID
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      DEBUG_MSG("Connected: %s IP: %s ", WiFi.SSID().c_str(), IpAddress2String(WiFi.localIP()).c_str());
    }
    else
    {
      DEBUG_MSG("No Connection :( (Sad face) - enabling AP mode\n\r");
      configureWifiAP(); // change to AP mode

    }

    WiFi.scanDelete();
  }
}