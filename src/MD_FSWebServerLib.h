//MD_FSWebServerLib.h

#ifndef _MDFSWEBSERVERLIB_h
#define _MDFSWEBSERVERLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#include "main.h"
// the following may have already been included via main.h, but are explicitly included here. 
// #include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <TimeLib.h>
// #include <NtpClientLib.h>
// #include <ESPAsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <ESP8266mDNS.h>
// // #include <ESPmDNS.h>
// #include <FS.h>
// #include <Ticker.h>

// #include <ArduinoOTA.h>

// #include <ArduinoJson.h>

 // #include <regex>
  #include <cstring>
// using namespace std;


#define CONNECTION_LED LED_BUILTIN  // Connection LED pin (LED_BUILTIN). -1 to disable
// 0 is GPIO0 which can be configured in Hardware_Pinout.h and platformio.ini to Relay_pin[]
// -1 results in never server a page
// 1 - generates garbage (may be a  wrong/different BAUDRATE)
// 2 - gets a serve of the net_config page
// LED_BUILTIN - use built in LED - causes no messages on the ESP-01
#define AP_ENABLE_BUTTON 0 // Button pin # used to enable AP during startup for configuration.
// -1 to disable,
// 0 = read from config file (_config.WiFiMode which can be "STA" or "AP")
// 2 = GPIO2 or other available pin depending avail of process (ESP01 only has 2 pin available)


#define CONFIG_FILE_DEFAULT "/Config_default.json"
#define CONFIG_FILE "/config.json"
#define SECRET_FILE "/secret.json"

// extern struct strBSMdata BSMdata; //to hold the serial number and other values read from the MCC/BSM

extern Ticker LED_Upload;
extern Ticker LED_StationModeConnected;
extern Ticker LED_SoftAPModeConnected;
extern Ticker LED_Webserver;
extern Ticker LED_MQTT_Message;
extern Ticker LED_Booting;
extern Ticker LED_MQTT_connected_to_broker;
extern Ticker Firmware_Update_Check;
extern String mqtt_broker;

// extern String R_Squig_Bracket;
// extern String L_Squig_Bracket;

extern void changeState(int LED);
extern int last_Sensor2Gnd_state; // True is open, False is closed

#define SCAN_PERIOD 60000
    extern long lastScanMillis;

#define BLINK_PERIOD 250
    extern long lastBlinkMillis;
    extern bool ledState;

#include "credentials_EEPROM.h"
extern struct Credentials eepromVar;
extern Credentials loadCredentials();
extern void saveCredentials(Credentials &newAP);
extern void clearCredentials();
extern void safeRestart();
extern String calculated_Hostname();
extern unsigned long requestRestartMillis;

struct strConfig{
    String ssid;
    String password;
    IPAddress  ip;
    IPAddress  netmask;
    IPAddress  gateway;
    IPAddress  dns;
    bool dhcp;
    String ntpServerName;
    long updateNTPTimeEvery;
    long timezone;
    bool daylight;
    String deviceName;
	IPAddress mqtt_broker_ip;
	long mqtt_port;
	bool mqtt_retain;
    long mqtt_QoS;
    String WiFiMode;
    String Model;
    String Manufacturer;
    String Acquired;
    float TargetCT;
    unsigned long debounceDelay;
    String wwwUser;
    String wwwAuth;
    // String ueia;
    // String meia;
} ;

struct strApConfig{
    String APssid = "PaGoI-"; // ChipID is appended to this name
    String APpassword = "12345678";
    bool APenable = true; // AP enabled (true) means allow ESP to go into AP mode
    // 
} ;

struct strHTTPAuth{
    bool auth;
    String wwwUsername;
    String wwwPassword;
} ;

class AsyncFSWebServer : public AsyncWebServer {
public:
    AsyncFSWebServer(uint16_t port);
    void begin(FS* fs);
    void handle();
    strConfig _config; // General and WiFi configuration
    void onWiFiConnected(WiFiEventStationModeConnected data);
    // void onWiFiConnected(const WiFiEventStationModeConnected& event) ;
    void onWiFiDisconnected(WiFiEventStationModeDisconnected data);
    // void onWiFiDisconnected(const WiFiEventStationModeDisconnected& event);

protected:
    // strConfig _config; // General and WiFi configuration
    strApConfig _apConfig; // Static AP config settings
    strHTTPAuth _httpAuth;
    FS* _fs;
    unsigned long  wifiDisconnectedTime = 0;
    unsigned long  wifiDisconnected_timeout = 60*1000*1; // timeout for when in STA mode and haven't connected as a Client - maybe the SSID is not in range or off-line
    unsigned long wifiAP_timer=0;
    unsigned long wifiAP_timeout=60*1000*3;// timeout after x minutes of inactivity in AP mode, i.e. time allowed for someone to configure ESP while connected to ESP as an AP
    String _browserMD5 = "";
    uint32_t _updateSize = 0;

    //TODO investigate whether need to add onAP_Mode_StartedHandler??
    WiFiEventHandler onStationModeConnectedHandler, onStationModeDisconnectedHandler;

    //uint currentWifiStatus;

    Ticker _secondTk;
    bool _secondFlag;

    AsyncEventSource _evs = AsyncEventSource("/events");

    void sendTimeData();
    bool load_config();
    bool defaultConfig(); 
    bool save_config(); // how is this different than the AsyncFSWebServer:save_config()
    bool loadHTTPAuth();
    bool saveHTTPAuth();
    void configureWifiAP();
    void configureWifi();
    // void ConfigureOTA(String password);
    void serverInit();



    static void s_secondTick(void* arg);

    String getMac_address_s();

    //supporting functions
    void printWifiData();
    void printCurrentNet();

    //added function to support picking strongest AP
    void Connect_Secure_Auth_AP();

#ifndef RELEASE
    void handleFileRead_edit_html(AsyncWebServerRequest *request); // comment out to allow only listing files but not modifying - for better security when released disable this ability
#endif
    bool checkAuth(AsyncWebServerRequest *request);
    bool handleFileRead(String path, AsyncWebServerRequest *request);

	void handleFileCreate(AsyncWebServerRequest *request);
    void handleFileDelete(AsyncWebServerRequest *request);
    void handleFileList(AsyncWebServerRequest *request);
    void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void restart_esp(AsyncWebServerRequest *request);
    void send_host_configuration_values_html(AsyncWebServerRequest *request);
    void send_connection_state_values_html(AsyncWebServerRequest *request);
    void send_general_configuration_html(AsyncWebServerRequest *request);
    void send_general_configuration_values_html(AsyncWebServerRequest *request);
    void send_information_values_html(AsyncWebServerRequest *request);
    void send_network_configuration_html(AsyncWebServerRequest *request);
    void send_network_configuration_values_html(AsyncWebServerRequest *request);
    void send_update_firmware_values_html(AsyncWebServerRequest *request);
    void send_wwwauth_configuration_html(AsyncWebServerRequest *request);
    void send_wwwauth_configuration_values_html(AsyncWebServerRequest *request);
    void setUpdateMD5(AsyncWebServerRequest *request);
    void updateFirmware(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

	//Marin Digital added page handling:
	void send_NTP_configuration_html(AsyncWebServerRequest *request);
	void send_NTP_configuration_values_html(AsyncWebServerRequest *request);
	void send_module_configuration_html(AsyncWebServerRequest *request);
	void send_module_configuration_values_html(AsyncWebServerRequest *request);
	void send_mqtt_configuration_html(AsyncWebServerRequest *request);
	void send_mqtt_configuration_values_html(AsyncWebServerRequest *request);

    static String urldecode(String input); // (based on https://code.google.com/p/avr-netino/)
    static unsigned char h2int(char c);
    static boolean checkRange(String Value);
};

extern AsyncFSWebServer ESPHTTPServer;
// extern void OTA_Setup(String password);
extern AsyncMqttClient mqttClient; 

// extern Ticker mqttReconnectTimer;
// extern WiFiEventHandler wifiConnectHandler;
// extern WiFiEventHandler wifiDisconnectHandler;
// extern Ticker wifiReconnectTimer;



extern unsigned long Client_previous_ms;
extern void connectToMqtt();
extern void checkForUpdates();
extern void flashLED(int pin, int times, unsigned long delayTime);
// extern String LastTemp;
// extern String LastHumid;
extern float CycleTime;

#endif // _FSWEBSERVERLIB_h
