/*
Filename: defaults.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.1
Date: 22Dec2019
Updates: 
Version 1.1 
  Update to generic client/agnostic
  Variables for client, application are now set in platformio.ini
*/

#ifndef Defaults_H
#define Defaults_H//------------------------------------------------------------------------------

#include "main.h"


/*! \file defaults..h
    \brief A Documented file.
    These are the default substitution values at compile
    Details.
*/
// Modules to load
// #define Serial_Read 1
// #define MQTT_client 1
// #define OTA_Listen 1
// #define UDP_to_Serial
// #define HTTP_Service 1
//
//

// ----------------------------------------------------------------------------
// RELEASE is also defined in FSWebServerlib.h to turn off debugging - But, set it below - works globally
// -----------------------------------------------------------------------------
/// <<<<<<< To turn on debugging SET in platformio.ini
// #define RELEASED  //  debug output
// -------------------------------------------------- RELEASE defined or not
//
// #define Config_BSM // used to detect MCC_BSM

#ifndef RELEASED //
  #define DEBUG true // flag to turn on/off debugging for library debugging
  // Comment out or change mapping for the DEBUG_PORT
  //      mapping could be any "Serial" type class (I think)
  #define DEBUG_PORT Serial
// NOTE: long input Strings (>~30 characters) with DEBUG_PORT set will cause ESP to crash
#else
  #define DEBUG false // if RELEASE then turn off all debugging
#endif



// TODO: Rationalize with what is stored in SPIFF and naming convention of ESP_Generic
// TODO move variables to SPIFFS with the _config.variable style of MD_FSWebServerLib
//------------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HARDWARE
//     NOTE: Pin numbers are defined in Hardware_Pinout.h
// -----------------------------------------------------------------------------
// UseSWSer and UseHWSer are defined in build_flags
#ifdef USE_SW_SER
    #define SERIAL_OTHER_DEVICE_PORT Serial_B
extern SoftwareSerial Serial_B;
#endif

#ifdef USE_HW_SER
    #define SERIAL_OTHER_DEVICE_PORT Serial
#endif

#define SERIAL_HOST_PORT Serial
#define SERIAL_BAUDRATE         9600
//#define LED_PIN                 13

// These String names are defined in main.cpp and are used to test the port in Get_Serial_Port
extern String Host_Port; // port of the USB serial interface
extern String Other_Device_Port; // port  to use to talk to BSM (e.g. the ESP serial interface
extern String Debug_Port;

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------
// #define WIFI_APPROVED_AP_PREFIX "Liberty"
// #define WIFI_APPROVED_AP_PREFIX "MW_MM_AP"
#define WIFI_RECONNECT_INTERVAL 300000
#define WIFI_MAX_NETWORKS       3
// #define AP_PASS                 "12345678"
#define WIFI_APCONFIGMODE true
//if false - then device will connect to AP and not use the ESP_AP mode.
//if true - then device will try to connect and if it fails then use ESP_AP mode.
// RPi and ESP's must be on same channel to allow concurrent STA+AP access
#define WIFI_CHANNEL 6
#define WIFI_DHCP true
// only use these as default if DHCP is false
// #define WIFI_DHCP false
#define WIFI_STATIC_IP  "192.168.100.151"
#define WIFI_NETMASK "255.255.255.0"
#define WIFI_GATEWAY "192.168.100.1"
#define WIFI_DNS "192.168.100.1"

// -----------------------------------------------------------------------------
// HTTP web server
// -----------------------------------------------------------------------------
//
#define HTTP_SERVER_PORT 80
#define HTTP_Server_USER_NAME "admin"
#define HTTP_Server_USER_PASSWORD "admin"

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------
// #define MQTT_BROKER             "54.190.195.131"
#define MQTT_BROKER             "192.168.100.1"
// #define MQTT_BROKER             "MILwright.pagoi.com"
#define MQTT_PORT               1883
#define MQTT_RETAIN             true
#define MQTT_RECONNECT_DELAY    10000
#define MQTT_QOS              0 // Quality of Service for Pubs (maybe need to add one for each Sub)
// There are 3 QoS levels in MQTT:
// At most once (0)
// At least once (1)
// Exactly once (2).
// #define MQTT_IP_TOPIC           "/device/rentalito/ip"
// #define MQTT_VERSION_TOPIC      "/device/rentalito/version"
// #define MQTT_FSVERSION_TOPIC    "/device/rentalito/fsversion"
// #define MQTT_HEARTBEAT_TOPIC    "/device/rentalito/heartbeat"
#define MQTT_PUB_TOPIC         "Sensor/from_device"  // the publish topic prefix (appended with /host_Name)
#define MQTT_PUB_TOPIC_1       "/Status"
// #define MQTT_BRIGHTNESS_TOPIC   "/device/rentalito/brightness"
//TODO: rethink how the topics are used - this may be too complicated or not enough -
//           hard to listen | validate topics that are subscribed with #
#define MQTT_MAX_SUB_TOPICS 5 // include LISTEN and OTA topices and the following:
#define MQTT_LISTEN_TOPIC         "Sensor/to_device"  //This is the base listen topic
#define MQTT_OTA_TOPIC_MODIFIER    "ota" // listen for OTA, this will be a subtopic of MQTT_LISTEN_TOPIC/deviceName
#define MQTT_SUB_TOPIC_1 "intoSensor/Relay"
#define MQTT_SUB_TOPIC_2 "MSG"
#define MQTT_SUB_TOPIC_3 "intoSensor/"
#define MQTT_MAX_TOPIC_LENGTH 30
#define MQTT_MAX_MESSAGE_LENGTH 250 // this is hanging loading the webpages when >120

// -----------------------------------------------------------------------------
// OTA Server
// -----------------------------------------------------------------------------
// defaults. These values are used when OTA initiated from MQTT or from a PUSH (i.e. where the program looks on OTA server for updates)
// TODO: implement PUSH where a new firmware.bin or spiff.bin is pushed to a host_Name (by Gateway/site/location)
// ASSUMPTION: that having this defined in the code is more secure than putting in the configuration file (Node_Config_default.json)
// WORKAROUND/ENHANCEMENT: change/allow the MQTT message to include the entire URL with filename
// // #define OTA_SERVER             "192.168.100.1"
// #define OTA_SERVER             "demo.pagoi.com"
// // #define OTA_SERVER              "MILwright-bcn.local"
// #define OTA_PASS                "1234"
// #define OTA_PORT                8266
// #define OTA_PATH                "/OTA_Images/pagoi_"
// #define OTA_update_username "admin" // can be changed by user from system.html
// #define OTA_update_password "admin"

// -----------------------------------------------------------------------------
// NTP
// -----------------------------------------------------------------------------
//
#define NTP_SERVER              "pool.ntp.org"
#define NTP_TIME_OFFSET         -80
#define NTP_DAY_LIGHT           true
#define NTP_UPDATE_INTERVAL     1800

// -----------------------------------------------------------------------------
// DHT
// -----------------------------------------------------------------------------
//
//#define DHT_PIN                 5
#define DHT_UPDATE_INTERVAL     300000
#define DHT_TYPE                DHT22
#define DHT_TIMING              11
#define DHT_TEMPERATURE_TOPIC   "/home/studio/temperature"
#define DHT_HUMIDITY_TOPIC      "/home/studio/humidity"

// -----------------------------------------------------------------------------
// IR
// -----------------------------------------------------------------------------

//#define IR_PIN                  4

// -----------------------------------------------------------------------------
// DEFAULTS
// -----------------------------------------------------------------------------

#define HEARTBEAT_INTERVAL      300000
#define HOSTNAME                APP_NAME  // APP_NAME defined  main.h
#define DEVICE                  APP_NAME
#define DEFAULT_WORKING_MODE    1
#define MAX_TOPICS              20
#define MAX_VALUE_LENGTH        20
#define DISPLAY_UPDATE_TIME     5000
#define DISPLAY_TICK_TIME       1000
// #define WIFI_APPROVED_AP_PREFIX "PAGOI"

// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------
//
const char SOH = '\x01';      // indicate start of data file (use for XModem)
const char STX = '\x02';      // indicate start of text message 
const char ETX = '\x03';      // indicate end of text message
const char NL = '\x0A';       // indicate end of text message dec=10
const char CR = '\x0D';       // indicate end of text message dec=13
const char CAN = '\x18';     // indicate cancel of the message (dec=23)
const char startMarker = '<'; // indicate start of Command message
const char startMarkerALT = '['; // indicate start of Command message
const char endMarker = '>';   // indicate end of Command message
const char endMarkerALT = ']';   // indicate end of Command message
// String L_Squig_Bracket = "\u007B"; // "{"
// String R_Squig_Bracket = "\u007D"; // "}"

#endif

//
//  For reference (from wl_definitions.h)
//
// typedef enum {
//       WL_NO_SHIELD = 255,
//         WL_IDLE_STATUS = 0,
//         WL_NO_SSID_AVAIL,
//         WL_SCAN_COMPLETED,
//         WL_CONNECTED,
//         WL_CONNECT_FAILED,
//         WL_CONNECTION_LOST,
//         WL_DISCONNECTED
// } wl_status_t;
//
// That means
// WL_NO_SHIELD = 255,
// WL_IDLE_STATUS = 0,
// WL_NO_SSID_AVAIL = 1
// WL_SCAN_COMPLETED = 2
// WL_CONNECTED = 3
// WL_CONNECT_FAILED = 4
// WL_CONNECTION_LOST = 5
// WL_DISCONNECTED = 6
