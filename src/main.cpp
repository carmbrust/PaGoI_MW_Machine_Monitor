/*
Project: Machine Monitor WiFi
Filename: main.cpp
Client: PaGoI  .
Author: Chris Armbrust, Marin Digital, (c)2017-2020
*/
#include "main.h"

// TODO These values should be read from the SPIFF (so delete when SPIFF added)
// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
bool Heartbeat_state = true;
bool OTA_In_Process = false;

/* Note:  RFC 952 https://tools.ietf.org/html/rfc952
    ... only the ASCII letters 'a' through 'z' (in a case-insensitive manner), the digits '0' through '9', and the hyphen ('-').
 */
String host_Name = "xxxxxx-1234";
String localIP_s = "xxx.xxx.xxx.xxx";
// // set default IPAddress and NodeIP;
// String ipAddr = "255.255.255.255";
// String MacAddr="xx:xx:xx:xx:xx:xx";
// String MacAddr = "aabbccddeeffgg";
// const int FW_VERSION = 1903; // Initial firmware version - TODO: reconcile with APP_VERSION

/******* Initialize (instantiate) global instances **********/

// WiFiClient espClient;
// PubSubClient client(espClient);
// ConfigManager configManager;
String L_Squig_Bracket = "\u007B"; // "{"
String R_Squig_Bracket = "\u007D"; // "}"
std::string nist_time = "";

WiFiClient wifiClient; // used globally and only one connection, so declare here
HTTPClient httpClient;

elapsedMillis timeElapsed; // declare global if you don't want it reset every
                           // time loop runs

// Ticker timers
Ticker LED_Upload;
Ticker LED_StationModeConnected;
Ticker LED_SoftAPModeConnected;
Ticker LED_Webserver;
Ticker LED_MQTT_Message;
Ticker LED_Booting;
Ticker LED_MQTT_connected_to_broker;
// Ticker Firmware_Update_Check;

// OLED display SSD1306 128x64
#ifdef OLED_DISPLAY
//TODO: Eliminate use of 'delay' in this dependency - will crash ESP for other functions

#include <Wire.h>    // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Include the UI lib
#include "OLEDDisplayUi.h"
#include "font.h" // these fonts for Tinos

// Include custom images
#include "images.h"
#define SDA 2 // i2c SDA - D4
#define SCL 0 // i2c SCL - D3
#define i2c_address 0x3c
SSD1306 display(i2c_address, SDA, SCL);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui(&display);

int screenW = 128;
int screenH = 64;
// int screenW = 64;
// int screenH = 48;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int clockRadius = 23;
int frameStart = 1;     // set to 0 when a frame has started
time_t prevDisplay = 0; // when the digital clock was displayed

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an
// int.
int intervalMillis_Cycle = 5 * 60 * 1000; // interval between sensor readings
int lastMsg = -intervalMillis_Cycle;      // set so that the sensors will be read on
                                          // initialization (first pass)

long TimeRefresh4NIST = 24 * 60 * 60; // check in once per x seconds (1 day is
                                      // 24*60*60) but need to do it at 2:01am
                                      // to catch PDT<>PST changeover

// utility function for digital clock display: prints leading 0
String twoDigits(int digits)
{
  if (digits < 10)
  {
    String i = '0' + String(digits);
    return i;
  }
  else
  {
    return String(digits);
  }
}

void init_Wifi()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");
}

String Wifi_scan()
{
  Serial.println("\nScan start");
  String SSID_List = "\nno networks found\n";
  ;
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println(SSID_List);
  }
  else
  {
    SSID_List = String(n);
    SSID_List = SSID_List + " networks found \r\n";
    // Serial.print(SSID_List);
    // Serial.print(n);
    // Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      SSID_List = SSID_List + String(i + 1) + ": " + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + ")\n";
      // Serial.print(i + 1);
      // Serial.print(": ");
      // Serial.print(WiFi.SSID(i));
      // Serial.print(" (");
      // Serial.print(WiFi.RSSI(i));
      // Serial.print(")");
      // Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      // Serial.println(WiFi.encryptionType(i));
      // delay(10);
    }
  }
  Serial.print(SSID_List);
  Serial.println("");
  return SSID_List;
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Arial 10");

  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "Arial 16");

  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 34 + y, "Arial 24");
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // Text alignment demo
  display->setFont(Tinos_Regular_8);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33 + y, "Right aligned (128,33)");
}

void ModuleIDFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  String msg = "MAC: " + Read_Mac_Address_s();
  display->drawString(0 + x, 14 + y, msg);
  // localIP_s = "IP: " + IpAddress2String(WiFi.localIP());
  display->drawString(0 + x, 26 + y, "IP: " + IpAddress2String(WiFi.localIP()));
  display->drawString(0 + x, 37 + y, "Name: " + host_Name);
  display->drawString(0 + x, 48 + y, "Version: " + String(APP_VERSION));
  // display->drawString(0 + x, 55 + y, "AccessPoint: " + String(WiFi.SSID()));
  // display->drawStringMaxWidth(0 + x, 10 + y, 128, msg);
}

void ModuleReadingsFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  //TODO: display current Relay2Gnd State
  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(8) + 30;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, LastHumid);
  String LastHumidityValue = doc["V"];
  String Humid_UoM = doc["U"];
  deserializeJson(doc, LastTemp);
  String LastTempValue = doc["V"];
  String Temp_UoM = doc["U"];
  int UpTime = millis64() / 1000;

  String msg = "Humidity: " + LastHumidityValue + Humid_UoM + "     Temperature: " + LastTempValue + Temp_UoM + "     CycleTime: " + CycleTime + "        UpTime: " + String(UpTime);
  display->drawStringMaxWidth(0 + x, 14 + y, 128, msg);
  // display IP and hostName
}

void NetworkList(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // another frame content goes here
  if (frameStart == 1)
  {
    Serial.print(millis());
    // String Wifi_list = Wifi_scan();// modify to return a string or pointer to array with the network information to output on the display
    frameStart = 0;
  }
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  // display->drawStringMaxWidth(0 + x, 10 + y, 128, "Goal is to update this with a list of the available networks.");
  // display->drawStringMaxWidth(0 + x, 10 + y, 128, Wifi_scan());
  display->drawString(0 + x, 11 + y, Wifi_scan());
  delay(500);
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  //  ui.disableIndicator();

  // Draw the clock face
  //  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for (int z = 0; z < 360; z = z + 30)
  {
    //Begin at 0° and stop at 360°
    float angle = z;
    angle = (angle / 57.29577951); //Convert degrees to radians
    int x2 = (clockCenterX + (sin(angle) * clockRadius));
    int y2 = (clockCenterY - (cos(angle) * clockRadius));
    int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 8))));
    int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 8))));
    display->drawLine(x2 + x, y2 + y, x3 + x, y3 + y);
  }

  // display second hand
  float angle = second() * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 5))));
  int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 5))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display minute hand
  angle = minute() * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 4))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 4))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int((minute() / 12) * 6);
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 2))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 2))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  String timenow = String(hour()) + ":" + twoDigits(minute()) + ":" + twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x, clockCenterY + y, timenow);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(10 + x, 20 + y, "Network: " + String(WiFi.SSID()));
}

void PutaGaugeOnIt()
{
  // This is a frame version
  int x = 0;
  int y = 0;
  String timenow = String(hour()) + ":" + twoDigits(minute()) + ":" + twoDigits(second());
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(clockCenterX + x, clockCenterY + y, "PaGoI");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(10 + x, 20 + y, "Put a Gauge on It!");
}

void PutaGaugeOnItMsg(String msg)
{
  // This is a generalized message version
  int x = 0;
  int y = 0;
  // String timenow = String(hour()) + ":" + twoDigits(minute()) + ":" + twoDigits(second());
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(clockCenterX + x, clockCenterY + y, "PaGoI");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(10 + x, 20 + y, msg);
  display.display();
}
// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {analogClockFrame, digitalClockFrame, ModuleIDFrame, ModuleReadingsFrame};

// how many frames are there?
int frameCount = 4;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {clockOverlay};
int overlaysCount = 1;

void OLED_setup()
{
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(10);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_DOWN);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();
}
void OLED_loop()
{
  int remainingTimeBudget = ui.update();
  frameStart = 0;
  if (remainingTimeBudget > 0)
  {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    // Wifi_scan();
    delay(remainingTimeBudget);
  }
  else
  { //must be out of time for this "frame" and will now move to the next
    frameStart = 1;
  }
}

#endif

// declarations of the serialRead object from the Serial_read class
Serial_read serialRead;

#ifdef USE_SW_SER
SoftwareSerial SERIAL_OTHER_DEVICE_PORT(14, 12, false); //- only for ESP platform  // RX, TX
// SoftwareSerial swSer(10, 11); //  for Arduino platform
//(RX, TX where D5=GPIO14=RX and D6=GPIO12=TX)
// extern SoftwareSerial swSer(14, 12, false, 256);
// extern allows swSert to be shared and used by the other modules (software
// compile units) D5  IO, SCK  (aka: CLK)           GPIO14 RX D6  IO, MISO (aka:
// CS)            GPIO12 TX
#endif
// The CAPPED variables are defined in defaults.h
String Host_Port = "SERIAL_HOST_PORT";                 // port of the USB serial interface
String Other_Device_Port = "SERIAL_OTHER_DEVICE_PORT"; // port of the ESP serial interface
String Debug_Port = "DEBUG_PORT";
String MQTT_pub_topic = MQTT_PUB_TOPIC;
String MQTT_sub_topic = MQTT_LISTEN_TOPIC; // final formwat will be the MQTT_LISTEN_TOPIC + hostname (MILwright-xxxx) + /#
vector<string> mqtt_SUB_topics_array;

int CycleCounter = 0;
bool LED_ON = false;
int current_LED_State = HIGH;

const char *wifi_mode_to_string(WiFiMode_t mode)
{
  switch (mode)
  {
  case WIFI_STA:
    return "WiFi Station Mode";
  case WIFI_AP:
    return "WiFi AP Mode";
  case WIFI_OFF:
    return "WiFi is OFF";
  case WIFI_AP_STA:
    return "WiFi is in AP and Station mode";
  default:
    return "Unknown WiFi mode.";
  }
}

const char *wl_status_to_string(wl_status_t status)
{
  switch (status)
  {
  case WL_NO_SHIELD:
    return "WL_NO_SHIELD";
  case WL_IDLE_STATUS:
    return "WL_IDLE_STATUS";
  case WL_NO_SSID_AVAIL:
    return "WL_NO_SSID_AVAIL";
  case WL_SCAN_COMPLETED:
    return "WL_SCAN_COMPLETED";
  case WL_CONNECTED:
    return "WL_CONNECTED";
  case WL_CONNECT_FAILED:
    return "WL_CONNECT_FAILED";
  case WL_CONNECTION_LOST:
    return "WL_CONNECTION_LOST";
  case WL_DISCONNECTED:
    return "WL_DISCONNECTED";
  // case -1:
  // return "WL_Timeout";
  default:
    return "Unknown issue.";
  }
}

// String IpAddress2String(const IPAddress& ipAddress)
String IpAddress2String(IPAddress ipAddress)
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}

String calculated_Hostname()
{
  char macStr[18] = {0};
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(macStr, "%s-%02X%02X", PAGOI_CLIENT, mac[4], mac[5]);
  // host_Name = String(macStr);
  return String(macStr);
}
String Read_Mac_Address_s()
{
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
  // ensure that MAC Address is always returned as upper case by using %02X vs %02x
  return String(macStr);
}

void safeRestart()
{
  //********** CHANGE PIN FUNCTION  TO TX/RX **********
  //GPIO 1 (TX) swap the pin to a TX.
  pinMode(1, FUNCTION_0);
  //GPIO 3 (RX) swap the pin to a RX.
  pinMode(3, FUNCTION_0);
  //***********
  digitalWrite(0, 1); // must be 1
  digitalWrite(1, 1); // This is TX pin
  digitalWrite(2, 1); // must be 1
  digitalWrite(3, 1); // This is RX pin
  digitalWrite(9, 1);
  digitalWrite(10, 1);
  digitalWrite(15, 0); // just in case GPIO15 was on before the reboot, GPIO15 means boot from SD-Card
  digitalWrite(16, 0);
  PutaGaugeOnItMsg("Rebooting . . . "); // reset the display TODO: update so that it says "Rebooting"

  delay(5000);
  ESP.restart();
}

// Both of these should be overridden from values in config file
String mqtt_broker = MQTT_BROKER; // should this be a char ?
int mqtt_port = MQTT_PORT;

String mqtt_Pub_message;
String lastTopic;
String lastMessage;

//TODO default values should be set in defaults (except those that can be reconfigured which should be in the config file)
unsigned long start_time = millis();
unsigned long config_previous_ms = 0;          // will store last time config was updated
unsigned long config_Interval = 30000;         // config_Interval at which to check request config status
int Blink_count = 0;                           // number of times to blink the LED - //TODO if needed make this and following an array or struct
unsigned long AP_Gateway_Blink_Interval = 100; // ms for blinking Interval at which to show Gateway activity
unsigned long AP_Gateway_previous_ms = 0;      // indicates last AP_Gateway activity/interaction??
unsigned long Client_Blink_Interval = 1000;    // ms for blink Interval at which to show web client activity status
unsigned long Client_previous_ms = 0;          // indicates the last client interaction (i.e. when in AP mode)
int CheckforUpdates_Interval = 30;             // check for updates using ticker at this interval in seconds
boolean flashing = false;                      // state of whether LED should be blinking
int Packet_ctr = 0;
unsigned long requestRestartMillis = 0; // gets set to current millis() when a restart has been requested

// strBSMdata BSMdata;
// strBSMdata BSMdefault;

char mqtt_Pub_serial[MQTT_MAX_MESSAGE_LENGTH];

// int64_t millis64() {
//     // Handling long durations (e.g. uptime of the ESP/BSM)
//     //  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
//     static uint32_t low32, high32;
//     uint32_t new_low32 = millis();
//     if (new_low32 < low32) high32++;
//     low32 = new_low32;
//     return (uint64_t) high32 << 32 | low32;
// }

string digitalClockDisplay(int UTS = 0, int time_offset_hrs_10x = -80, int daylight = 0)
{ // UTS is the UnixTimeStamp (seconds since 1970)
  // ASSUME:  UTS has already been adjusted to local time and that this routine is displaying the local time and offset from UTS
  // UTS = UTS + (time_offset_hrs_10x/10 + daylight) * 60 * 60; //read offset from timezone setting and daylight saving time (e.g. PDT)
  // String currentTime;
  // string offsetHours;
  // offsetHours<< << std::setw(5) << std::setfill('0') << std::internal <<  100*(time_offset_hrs_10x/10 + daylight);
  char buffer[25] = "";
  // sprintf(buffer, "%04d%02d%02dT%02d%02d%02d%+04d", year(UTS), month(UTS), day(UTS), hour(UTS), minute(UTS), second(UTS), 100 * (time_offset_hrs_10x / 10 + daylight));
  // currentTime = String(buffer);
  int offsetHoursMinutes = 100 * (int(time_offset_hrs_10x / 10) + daylight);
  if (UTS)
  {
    sprintf(buffer, "%04d%02d%02dT%02d%02d%02d", year(UTS), month(UTS), day(UTS), hour(UTS), minute(UTS), second(UTS));
  }
  else
  {
    sprintf(buffer, "%04d%02d%02dT%02d%02d%02d", year(), month(), day(), hour(), minute(), second());
  }
  char offsetTime[6]; // need to leave room for \0x at end
  // Add the offset - or +
  DEBUG_MSG("timestamp: %s, offsetHoursMinutes: %d\n\r", buffer, offsetHoursMinutes);
  if (offsetHoursMinutes < 0)
  {
    strcat(buffer, "-");
  }
  else
  {
    strcat(buffer, "+");
  }
  sprintf(offsetTime, "%04d", abs(offsetHoursMinutes));
  DEBUG_MSG("timestamp: %s, offsetHoursMinutes: %d, zero padded: %s\n\r", buffer, offsetHoursMinutes, offsetTime);
  // put the whole thing together
  strcat(buffer, offsetTime);

  string currentTime = string(buffer);
  return currentTime; // ISO 8601 basic format yyyymmddThhmmss-hhmm
}

std::string buildtimestamp_s = digitalClockDisplay(BUILD_TIMESTAMP + (NTP_TIME_OFFSET / 10 + NTP_DAY_LIGHT) * 60 * 60, NTP_TIME_OFFSET, int(NTP_DAY_LIGHT));

// TODO: Consider moving this to a MD_OTA_Suite.h/.cpp so that main is only main structure
// void AsyncFSWebServer::ConfigureOTA(String password) //  - this was the routing used in the original FSWebServerLib
void OTA_Setup(String password)
{
  // Port default from defaults.h
  ArduinoOTA.setPort(OTA_PORT);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname(ESPHTTPServer._config.deviceName.c_str()); //the OTA hostname must be diff from WebServer
  // ArduinoOTA.setHostname("UpdateMe");

  // No authentication by default
  if (password != "")
  {
    ArduinoOTA.setPassword(password.c_str());
    DEBUG_MSG("OTA password set to %s\r\n ", password.c_str());
  }
  else
  { // set password to at least something
    ArduinoOTA.setPassword((const char *)"1234");
  }
  DEBUG_MSG("Define ArduinoOTA routines....\r\n");
  ArduinoOTA.onStart([]() {
    DEBUG_MSG("StartOTA\r\n");
    DEBUG_MSG("Sketch size: %u\r\n", ESP.getSketchSize());
    DEBUG_MSG("Free size: %u\r\n", ESP.getFreeSketchSpace());
    DEBUG_MSG("Free Heap size: %u\r\n", ESP.getFreeHeap());
    DEBUG_MSG("Chip memory size: %u\r\n", ESP.getFlashChipSize());
    String updateType;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      updateType = "sketch";
    }
    else
    { // U_FS
      updateType = "filesystem";
    }
    DEBUG_MSG("Update type: %s\r\n", updateType.c_str());
    OTA_In_Process = true;

    // DEBUG_MSG("OTA password set as ");
    // Serial.println( password);
    // DEBUG_MSG("Hostname set %s\r\n", ESPHTTPServer._config.deviceName.c_str());
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_MSG("\n OTA End");
    OTA_In_Process = false;
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_MSG("OTA Progress: >%u%%   \r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_MSG("\r\n");
    DEBUG_MSG("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      DEBUG_MSG("Auth Failed\r\n");
    else if (error == OTA_BEGIN_ERROR)
      DEBUG_MSG("Begin Failed\r\n");
    else if (error == OTA_CONNECT_ERROR)
      DEBUG_MSG("\r\n >>> Connect Failed\r\n");
    else if (error == OTA_RECEIVE_ERROR)
      DEBUG_MSG("Receive Failed\r\n");
    else if (error == OTA_END_ERROR)
      DEBUG_MSG("OTA Ended with Error\r\n");
    OTA_In_Process = false;
  });
  DEBUG_MSG("ArduinoOTA routines defined. Now start 'ArduinoOTA.begin()' ...\r\n");

  ArduinoOTA.begin();
  DEBUG_MSG("\r\nOTA Ready\r\n");
}

void flashLED(int pin, int times, unsigned long delayTime)
{
  // modification by CArmbrust on 8Dec2018 - need to calc time in loop
  //TODO - digitalWrite doesn't seem to be working - is something else setting this pin?

  bool oldState = digitalRead(pin);
  // bool oldState = LOW;
  Client_previous_ms = millis(); // start the blink timer for main.cpp
  DEBUG_MSG("---Flash LED on pin %d during %lu ms %d times. Old state = %d\r\n", pin, delayTime, times, oldState);
  // return; // disables flash

  //TODO: this is a blocking loop and will cause soft reset if >~1 second - need to put in thread or other or dump
  if (delayTime * times > 1000)
  {
    times = int(delayTime / times);
  }
  unsigned long startblink = 0;
  unsigned long timeblink = 0;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH); // turns LED off

  // for (int i = 0; i=1; i++)
  for (int i = 0; i < times; i++)
  {

    for (int LED_State = 0; LED_State < 2; LED_State++)
    {
      startblink = millis(); // use for duration of the on and off state
      if (LED_State == 0)
      {
        digitalWrite(pin, LOW);
        DEBUG_MSG("---Flash LED_State is %d", LED_State);
      } // Turn on LED
      else
      {
        digitalWrite(pin, HIGH);
        DEBUG_MSG("---Flash LED_State is %d", LED_State);
      }

      timeblink = 0;
      int numLoops = 0;
      while (timeblink < delayTime)
      {
        timeblink = millis() - startblink;
        numLoops++;
        // could do stuff here or not
      }
      DEBUG_MSG("---Flashed LED %lu ms %d times in %d loops. \r\n", timeblink, i, numLoops);
    }
  }
  digitalWrite(pin, oldState); // return LED
  DEBUG_MSG("---Flash LED returned to old state %d\r\n", oldState);
}

void checkForUpdates_default_Host()
{
  checkForUpdates();
}

void checkForUpdates()
{
  if (IpAddress2String(WiFi.localIP()) > "0.0.0.0")
  {
    DEBUG_MSG("\n--------------------------------------------Checking for firmware updates on OTA Server.>>>>>>>>>>>>>\n");

    String mac = Read_Mac_Address_s();
    String fwURL = "http://";
    fwURL.concat(OTA_SERVER);
    fwURL.concat(OTA_PATH);
    fwURL.concat("/");
    fwURL.concat(PAGOI_CLIENT);
    // fwURL.concat("_");
    // String fwURL = "http://" + OTA_SERVER + OTA_PATH + "/" + PAGOI_CLIENT + "_";

    // "http://" + OTA_SERVER + OTA_PATH + "/" + PAGOI_CLIENT + "_";
    String fwVersionCheckURL = fwURL;
    fwVersionCheckURL.concat("_firmware.");
    fwVersionCheckURL.concat(mac);
    fwVersionCheckURL.concat(".version");

    // String board = env.board;
    String fw_model;
    if (ESP.getFlashChipSize() < 4194304)
    {
      fw_model = "_esp01_1m_160";
    }
    else
    {
      fw_model = "_d1_mini";
    }

    DEBUG_MSG("Sketch size: %u\r\n", ESP.getSketchSize());
    DEBUG_MSG("Free size: %u\r\n", ESP.getFreeSketchSpace());
    DEBUG_MSG("Free Heap size: %u\r\n", ESP.getFreeHeap());
    DEBUG_MSG("Chip memory size: %u\r\n", ESP.getFlashChipSize());
    DEBUG_MSG("Chip memory real size: %u\r\n", ESP.getFlashChipRealSize());
    DEBUG_MSG("\nCurrent NTP clock time: %s\n", digitalClockDisplay(now(), ESPHTTPServer._config.timezone, ESPHTTPServer._config.daylight).c_str());
    DEBUG_MSG("MAC address: %s\n", mac.c_str());
    DEBUG_MSG("Firmware version URL: %s\n", fwVersionCheckURL.c_str());

    // String fwURL_generic = fwURL;

    // DEBUG_MSG("Firmware URL: %s\n", fwURL_generic.c_str());

#ifndef RELEASED
    if (!wifiClient.connect(OTA_SERVER, 80))
    {
      DEBUG_MSG("'wifiClient' was not able to connect to %s. Boo.\n ", OTA_SERVER);
    }
    else
    {
      DEBUG_MSG("'wifiClient' was able to connect to %s. Yea!.\n", OTA_SERVER);
    }
#endif
    // DEBUG_MSG("WiFi Status: %s\n", WiFi.status());
    // fwVersionCheckURL format example:  http://192.168.100.1/OTA_Images/MILwright_firmware.A4CF12D9C4F2.version
    DEBUG_MSG("Checking for new Firmware version at %s \n", fwVersionCheckURL.c_str());
    httpClient.begin(wifiClient, fwVersionCheckURL);
    int httpCode = httpClient.GET();
    DEBUG_MSG("HTTP code: %d\n", httpCode);
    if (httpCode == 200)
    {
      String newFWVersion = httpClient.getString();

      DEBUG_MSG("Current firmware version: %d\n", APP_VERSION);
      DEBUG_MSG("Available firmware version: %s\n", newFWVersion.c_str());

      int newVersion = newFWVersion.toInt(); //TODO: what if the version is non-numberic (e.g. 21012a)
      // newVersion is the "desired" or target version that you want this device to be running.
      // Looking for file of format:   'http://192.168.100.1/OTA_Images/MILwright_d1_mini_newVersion_firmware.bin'
      
      // add board/environmnet to fwURL
      fwURL.concat(fw_model);

      if (newVersion != APP_VERSION)
      {
        fwURL.concat("_" + newFWVersion);

        if ((ESP.getSketchSize() > ESP.getFreeSketchSpace()))
        {
          // if the current sketch (.bin) is larger than the freespace, then there is not enough room for a new image, so use the bootstrap
          fwURL.concat("_bootstrap");
        }

        fwURL.concat("_firmware.bin");
        DEBUG_MSG("Preparing to update from:\n\t '%s'\n", fwURL.c_str());
        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Begin OTA Update...");
        display.display();
        String fwImageURL = fwURL;
        t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, fwImageURL);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          DEBUG_MSG("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          DEBUG_MSG("HTTP_UPDATE_NO_UPDATES\n");
          break;

        case HTTP_UPDATE_OK:
          DEBUG_MSG("HTTP_UPDATE_OK\n");
          break;

        default:
          DEBUG_MSG("Undefined HTTP_UPDATE Code: %d\n", ret);
        }
      }
      else
      {
        DEBUG_MSG("Already on latest version\n");
      }
    }
    else
    {
      DEBUG_MSG("Firmware version %s check failed, got HTTP response code %d\n", fwVersionCheckURL.c_str(), httpCode);
    }
    httpClient.end();
  }
  else
  {
    DEBUG_MSG("Firmware version check - not performed. No Network connection\n\r");
  }
}

void changeState(int LED)
{
  digitalWrite(LED, !(digitalRead(LED))); //Invert Current State of LED
  // DEBUG_MSG(">>>changeState on pin %d to %d... ", LED, digitalRead(LED));
}
void initialize_Pin_States()
{
  DEBUG_MSG(">>>>>>>>>  Toggle all the LEDS ... ");
  // Set pinModes and then blink all lights to confirm light/indicators are
  // working
  pinMode(Status_LED, OUTPUT);
  digitalWrite(Status_LED, 0);
  pinMode(HEARTBEAT, OUTPUT);
  digitalWrite(HEARTBEAT, 0);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, 0);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, 0);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, 0);
  pinMode(LED4, OUTPUT);
  digitalWrite(LED4, 0);
  pinMode(LED5, OUTPUT);
  digitalWrite(LED5, 0);
  pinMode(Sensor2Gnd, INPUT_PULLUP);
  delay(1000);
  // Turn OFF

  digitalWrite(LED1, 0);
  digitalWrite(LED2, 0); // close relay
  digitalWrite(LED3, 0);
  digitalWrite(LED4, 0);
  digitalWrite(LED5, 0);
  digitalWrite(Status_LED, 0);
  digitalWrite(HEARTBEAT, 0);
  //  Turn ON
  delay(2000);

  digitalWrite(LED1, 1);
  digitalWrite(LED2, 1); // open relay
  digitalWrite(LED3, 1);
  digitalWrite(LED4, 1);
  digitalWrite(LED5, 1);
  digitalWrite(Status_LED, 1);
  digitalWrite(HEARTBEAT, 1);
  delay(1000);
  DEBUG_MSG(" all light should be ON <<<<<<<<<<<<<<<<<<<<\n\r ");
}

int main()
{
  setup();
  loop();
  return 0;
}

void setup()
{

/*
TODO: better handle of soft WDT resets
try using one or more the following: 
ESP.wdtDisable(); // stops software wdt, hardware wdt will override in about 6-8 seconds if no activity
ESP.wdtEnable(1000); // time is in milliseconds - not clear if time is actually used, but does restart the wdt
ESP.wdtFeed();// feeds the wdt - note: you can use this in a loop and cause a program to never trigger either software OR hardware wdt (might not be good)

More info: 
https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/doc/faq/a02-my-esp-crashes.md

*/

// Initialize serial output
#ifdef DEBUG_PORT
  DEBUG_PORT.begin(SERIAL_BAUDRATE);
  DEBUG_PORT.setDebugOutput(DEBUG);
  DEBUG_PORT.println();
  DEBUG_PORT.println(Other_Device_Port + " <BSM and " + Host_Port + "<USB " + Debug_Port + "<Debug");
  DEBUG_PORT.print("Serial buffer size is ");
  DEBUG_PORT.println(SERIAL_RX_BUFFER_SIZE);
// DEBUG_MSG("''%s' <BSM,'%s' <USB, '%s'<Debug",SERIAL_OTHER_DEVICE_PORT,SERIAL_HOST_PORT,DEBUG_PORT);
#endif
  // wait for serial port to connect. Needed for native USB
  SERIAL_OTHER_DEVICE_PORT.begin(SERIAL_BAUDRATE);
  SERIAL_OTHER_DEVICE_PORT.println("\n\r[Booting ESP]");

  // Initilize the LED/Relay pins for output mode and set low
  // for (int i = 0; i < NUM_RELAYS; i++)
  // {
  //   DEBUG_MSG(" Relay# %d is on GPIO%d\n\r", i, Relay_Pin[i]);
  //   pinMode(Relay_Pin[i], OUTPUT);
  //   digitalWrite(Relay_Pin[i], LOW);
  // }
  initialize_Pin_States();

  LED_Booting.attach(0.5, changeState, Relay_Pin[0]); // start blink at duration/interval of .5 seconds, callback, parameter for callback

  //
  //   Serial.flush(); // clears output buffer
  // delay(5000);
  DEBUG_MSG("Starting boot-up \r\n");
  // DEBUG_MSG("Release %s\r\n",RELEASE);
  // DEBUG_MSG("Debug mode %s\r\n",DEBUG);
  // DEBUG_MSG("Debug port used %s\r\n",DEBUG_PORT);
  // DEBUG_MSG("Arduino value %s\r\n",ARDUINO);

  DEBUG_MSG("\n\rTimestamp of main.cpp saved version: %s \n", __TIMESTAMP__);
  DEBUG_MSG("\n\rTimestamp of platformio build version: %s \n\r", buildtimestamp_s.c_str());
  DEBUG_MSG("Sketch size: %u\r\n", ESP.getSketchSize());
  DEBUG_MSG("Free size: %u\r\n", ESP.getFreeSketchSpace());
  DEBUG_MSG("Free Heap size: %u\r\n", ESP.getFreeHeap());
  DEBUG_MSG("Chip memory size: %u\r\n", ESP.getFlashChipSize());
  // calculate if there is room for OTA with this program on this Chip
  if (ESP.getFreeSketchSpace() > ESP.getSketchSize())
  {
    DEBUG_MSG("OTA: Possible on this chip with this sketch. \n\r");
  }
  else
  {
    DEBUG_MSG("OTA: WARNING: OTA constrained - current sketch is too big: %u, largest new sketch: %u bytes\n\r", ESP.getSketchSize(), ESP.getFreeSketchSpace());
  }

#ifdef OLED_DISPLAY
  DEBUG_MSG("\r\n<OLED Display .... begin initialization. \r\n");
  OLED_setup();
  DEBUG_MSG("\r\n<OLED Display now initialized. \r\n");
  PutaGaugeOnItMsg("Put a Gauge on It");
  // display.clear();
  // display.setTextAlignment(TEXT_ALIGN_CENTER);
  // display.setFont(ArialMT_Plain_24);
  // display.drawString(clockCenterX, clockCenterY, "PaGoI");
  // display.setTextAlignment(TEXT_ALIGN_LEFT);
  // display.setFont(ArialMT_Plain_10);
  // display.drawString(10, 20, "Put a Gauge on It!");
  // display.display();
  // PutaGaugeOnIt();
  // display.clear();
  // display.setFont(ArialMT_Plain_16);
  // display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  // display.drawString(display.getWidth()/2, display.getHeight()/2 , "PaGoI - Put a Gauge on It!");
  // display.display();
#endif

  DEBUG_MSG("Begin FSWebserver "
            "Setup>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r");
  // SPIFFS.begin();
  SPIFFS.begin();
  digitalWrite(LED1, 0);
  DEBUG_MSG("... Completed SPIFFS"
            "Setup>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r");
  ESPHTTPServer.begin(&SPIFFS);
  digitalWrite(LED2, 0); // close relay
  DEBUG_MSG("Completed FSWebserver "
            "Setup<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Connect to WiFi...");
  display.display();
  setup_DHT(); // start the DHT sensor
  digitalWrite(LED3, 0);
  DEBUG_MSG("Completed DHT Sensors"
            "Setup<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, "DHT Sensors Read...");
  display.display();

  // customize the pub and sub topices
  host_Name = ESPHTTPServer._config.deviceName.c_str(); // this is the configured host_name in the config.json file
  DEBUG_MSG("Hostname calculated: '%s' vs from config: '%s'\n", calculated_Hostname().c_str(), ESPHTTPServer._config.deviceName.c_str());
  // NOTE: Linux is supporting up to a max length of 64, but for the ESP8266, appears to only support 10 charger, so limit length.
  localIP_s = IpAddress2String(WiFi.localIP());
  DEBUG_MSG("Local IP address: %s\n\r", localIP_s.c_str());

  // set the MQTT broker and port
  mqtt_broker = IpAddress2String(ESPHTTPServer._config.mqtt_broker_ip);
  mqtt_port = ESPHTTPServer._config.mqtt_port;
  // Serial.print(ESPHTTPServer._config.mqtt_broker_ip);
  DEBUG_MSG(">>>MQTT mqtt_broker/port: %s/%d\n\r", mqtt_broker.c_str(), mqtt_port);

  if (localIP_s > "0.0.0.0") // if we got an IP address, then create pub and sub topics
  {
    DEBUG_MSG("MQTT Setup\n\r");
    yield();

    // strcat(MQTT_pub_topic, "/");
    // strcat(MQTT_pub_topic, ESPHTTPServer._config.deviceName.c_str());
    MQTT_pub_topic.concat("/");
    MQTT_pub_topic.concat(ESPHTTPServer._config.deviceName);

    // strcat(MQTT_sub_topic, "/");
    // strcat(MQTT_sub_topic, ESPHTTPServer._config.deviceName.c_str());
    // strcat(MQTT_sub_topic, "#"); // e.g. listen to any topices addressed to this deviceName/host_Name
    MQTT_sub_topic.concat("/");
    MQTT_sub_topic.concat(ESPHTTPServer._config.deviceName);
    MQTT_sub_topic.concat("/#"); // e.g. listen to any topic addressed to this deviceName/host_Name

    DEBUG_MSG("MQTT Subscribe Topic: '%s' \n\rand Publish Topic: '%s'\n", MQTT_sub_topic.c_str(), MQTT_pub_topic.c_str());

    // vector<string> mqtt_SUB_topics_array;
    mqtt_SUB_topics_array.push_back(MQTT_sub_topic.c_str());
    mqtt_SUB_topics_array.push_back(MQTT_OTA_TOPIC_MODIFIER);
    mqtt_SUB_topics_array.push_back(MQTT_SUB_TOPIC_1);
    mqtt_SUB_topics_array.push_back(MQTT_SUB_TOPIC_2);
    mqtt_SUB_topics_array.push_back(MQTT_SUB_TOPIC_3);
    for (size_t i = 0; i < mqtt_SUB_topics_array.size(); i++)
    {

      DEBUG_MSG("Topic: %u is: '%s'\n\r", i,
                mqtt_SUB_topics_array[i].c_str());
    }

    MQTT_Async_setup();
    digitalWrite(LED4, 0); // close relay
    DEBUG_MSG("Completed MQTT Broker setup "
              "Setup<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Connected to MQTT Broker.");
    display.display();
    DEBUG_MSG("after MQTT Aync setup the Free Heap size: %u\r\n",
              ESP.getFreeHeap());
    // SERIAL_OTHER_DEVICE_PORT.println("[_Connected]"); // message to BSM saying that now ready <<<< only if wifi is connected else "looking for AP"
  }
  else if (localIP_s == "0.0.0.0")
  {
    DEBUG_MSG("Skipping setting up MQTT because we are in configuration mode! - no IP assigned");
    //  flashLED(LED4,5, 100);// <<< this will crash the ESP with WDT timeout
    digitalWrite(LED4, 1); // leave it ON as indication that not completed
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Try 192.168.4.1");
    display.display();
  }
  DEBUG_MSG("\r\nBoot time: %lu", millis() - start_time);
  DEBUG_MSG("\r\n<Set up complete - waiting for input on serial.  >\r\n");
  DEBUG_MSG("End of setup() and Free Heap size: %u\r\n", ESP.getFreeHeap());

  SERIAL_OTHER_DEVICE_PORT.println(
      "[_Connected]"); // message to BSM saying that now ready <<<< only if wifi is connected else "looking for AP"

  OTA_Setup(OTA_PASS);
  checkForUpdates(); // check and then start ticker object to check periodically
  // Firmware_Update_Check.attach(CheckforUpdates_Interval, checkForUpdates);// Ticker is not working
  digitalWrite(LED5, 0);
  LED_Booting.detach(); // stop blinking
}
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
      // StaticJsonDocument<200> doc;
      const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(8) + 30;
      DynamicJsonDocument doc(capacity);
      deserializeJson(doc, Mqtt_Message);
      String Humidity = doc["H"];
      String Temperature = doc["T"];
      String RID = doc["RID"];
      DEBUG_MSG("%d, %s, %s, %s, %s\n", capacity, Mqtt_Message.c_str(), Humidity.c_str(), Temperature.c_str(), RID.c_str());
      if (LastTemp != Temperature)
      {
        // format and output new reading
        MQTT_pub_calculated_Topic(Mqtt_Message);
        LastTemp = Temperature;
        LastHumid = Humidity;
      }
      else if (LastHumid != Humidity)
      {
        MQTT_pub_calculated_Topic(Mqtt_Message);
        LastHumid = Humidity;
      }
      else
      {
        DEBUG_MSG(" - No H | T change\n"); // so no output
      }
      digitalWrite(Status_LED, LOW);
    }
  }
  else
  { // Blink if doing OTA update (might only work with OTA detected vs directed - from OTA_update.htm)
    digitalWrite(LED5, !digitalRead(LED5));
  }
  // time dependent actions
  // This is redundant to the Ticker attach method but Ticker.attach not working
  if ((int(currentMillis / 1000) % int(HEARTBEAT_INTERVAL / 1000)) == 0) // check for OTA updates every modulo of HEARTBEAT_INTERVAL and try to catch within the same second
  {
    digitalWrite(LED5, 1);
    checkForUpdates();
    digitalWrite(LED5, 0);
  }

  if (currentMillis > 0 && (int(currentMillis % (24 * 60 * 60 * 1000)) == 0)) // let's reboot every 24 hours of uptime
  {
    DEBUG_MSG("Running for a day, so, let's reboot %lu\n", currentMillis);
    safeRestart();
  }

  /* TODO: Restarting timeout logic 
give the other processes some time (5-10 seconds) then do the restart
requestRestartMillis is when restart requested
*/
  if (requestRestartMillis > 0)
  {
    if ((currentMillis - requestRestartMillis) > 10000)
    {
      digitalWrite(15, 0); // just in case GPIO15 was on before the reboot, GPIO15 means boot from SD-Card
      ESP.restart();
    }
  }

#ifdef OLED_DISPLAY
  OLED_loop();
#endif
  ESPHTTPServer.handle(); // this handles webserver for both AP and for STA modes and also OTA Updates
  ArduinoOTA.handle();
}
