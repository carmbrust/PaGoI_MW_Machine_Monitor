#ifndef MQTT_ASYNC_CLIENT_H
#define MQTT_ASYNC_CLIENT_H

#include "main.h"
// #include <string>
// #include <ESP8266WiFi.h> // already declared in main.h
//
#ifdef USE_SW_SER
extern SoftwareSerial SERIAL_BSM_PORT;
#endif

// #include <ESP8266WiFi.h>
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>
// #include <ESP8266WiFiMulti.h>
// #include <ESP8266HTTPClient.h>
// #include <ESP8266httpUpdate.h>
// #include <Ticker.h>


// #include <AsyncMqttClient.h> // (http://platformio.org/lib/show/346/AsyncMqttClient) built on [me-no-dev/ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
extern AsyncMqttClient mqttClient;
extern HTTPClient httpClient;
extern WiFiClient wifiClient;

extern Ticker mqttReconnectTimer;
extern WiFiEventHandler wifiConnectHandler;
extern WiFiEventHandler wifiDisconnectHandler;
// extern Ticker wifiReconnectTimer;

extern AsyncFSWebServer ESPHTTPServer; // why extern - because handled by AsyncServer - is this used wintin this scope? 

// #include <ESP8266WiFi.h> // included in base platform distribution
#include <ESP8266HTTPClient.h> // also included in ESP8266httpUpdate.h

#include <ESP8266httpUpdate.h> // This allows pull udpate via HTTP protocol where ESP is a httpClient. Initiate request either by checking if an update or in response to MQTT_OTA topic
// These should be defined and used by the ESPAsync and ArduinoOTA libraries
// #include <ESP8266HTTPUpdateServer.h>
// ESP8266HTTPUpdateServer httpUpdater; //<<< used to create an object of the update server

// #include <ESP8266mDNS.h>
// MQTT_MAX_PACKET_SIZE : Maximum packet size
// #ifndef MQTT_MAX_PACKET_SIZE
// #define MQTT_MAX_PACKET_SIZE 256
// #endif
// #include <PubSubClient.h> // the above block should override the value in the base library
// #include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>

// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h> // included in ESP8266httpUpdate.h

// ESP8266WebServer httpServer(80);
// #include <ESPAsyncWebServer.h>
//
// #ifndef HTTPserver_DEF
// #define HTTPserver_DEF
// AsyncWebServer HTTPserver(80);
// #endif


extern void onWiFiConnected(WiFiEventStationModeConnected data);
extern void onWiFiDisconnected(WiFiEventStationModeDisconnected data);
//  WiFiClient wifiClient;
// WiFiClient espClient;
// PubSubClient MQTT_client(espClient);
// any includes that are unique to this modules
// #include  ...
#include <TimeLib.h>

//---------------------------------------------------
// Declarations for functions used by this module:
//
void setup_MQTT();
// void loop_MQTT(); // check MQTT and will activate callback if needed - not used for Async
bool MQTT_pub(char *topic, char *Message, uint8_t qos);
// void MQTT_callback(char *topic, byte *payload, unsigned int length); //TODO find if OTA can be rec'd via MQTT message
// void MQTT_receive_ota(const MQTT::Publish& pub); //TODO find if OTA can be rec'd via MQTT message
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttConnect(bool sessionPresent);
void MQTT_process_Relay(char *ptr2payload);
void MQTT_echoPub(char * msg2Pub);
// void MQTT_echoPub(String str_payload);
String Put_Serial(char chars2output[], size_t length);
boolean MQTT_reconnect();
void connectToMqtt();


// Variables used in this module and defined elsewhere: 
// extern String R_Squig_Bracket;
// extern String L_Squig_Bracket;
extern int mqtt_port;
extern int Relay_State[];
extern int Relay_Pin[];
// extern char *MQTT_pub_topic;
extern String MQTT_pub_topic;
extern String MQTT_sub_topic;
extern String mqtt_broker;
extern vector<string> mqtt_SUB_topics_array;
extern String lastTopic;
extern String lastMessage;
extern String host_Name;
extern std::string buildtimestamp_s;
extern unsigned long AP_Gateway_previous_ms;
extern unsigned long start_time;

// functions defined elsewhere
extern String Read_Mac_Address_s();
extern void checkForUpdates();
extern const char* wl_status_to_string(wl_status_t status);
extern const char* wifi_mode_to_string(WiFiMode_t mode);


// Declaration for variables defined in this module (optional but nice to help identify where they came from)
// extern char LastPubReading[MQTT_MAX_MESSAGE_LENGTH];     // used to compare last published reading
// extern char MQTT_Buffer[MQTT_MAX_MESSAGE_LENGTH]; // this needs to be longer than your longest packet.<< Note:  Longest packet transmitted is 110
// ISSUE: where are these used? seems like for OTA
// extern const char *update_password ;
// extern const char *update_path;
// extern const char *update_username ;
extern long MQTT_lastReconnectAttempt;
// extern String Mqtt_Message;
char mqtt_announce[MQTT_MAX_MESSAGE_LENGTH];
extern void MQTT_Status_msg ();

// #if UseSWser
// extern SoftwareSerial serial_B;
// #elif UseHWser
// extern Serial serial_B;
// #endif

#endif
