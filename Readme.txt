Client: MILwright - based on PaGoI structure
Author: Chris Armbrust
Date: 20191221

Project: Machine Monitors
Environment: Using WeMOS d1 mini on the Marin Digital Machine Monitor board with RPi Gateway serving DHCP, Apache (support of dashboard and configuration webpages), MQTT, NTP, MQTT, Python for monitor and control apps and MariaDB as well as Access Point
Goals: 
1. Initial turn-on and configuration of a Machine Monitor
2. Directory: data - serves as webpages to allow setup of ESP8266)
3. Directory: src: c++ code that supports webpage and serial r/w to BSM_LT with MQTT messaging via Gateway MQTT server


Notes: 
1. based on project folder: U:\Arduino\User_Sketches\PaGoI-MILwright
2. Access points: 
    "AP_Default_SSID": "Liberty5",
	"AP_Default_PASS": "Gr8Vu0fTam",
    "AP_Default_SSID": "MW_MM_AP",
	"AP_Default_PASS": "MWat1450I",
    "AP_Default_SSID": "PaGoI",
	"AP_Default_PASS": "4154974238",
    "BCN-AP_SSID": "ArrghAP",
	"BCN-AP_PASS": "Arrgh_BCN!",

Monitor Sensors: 
1. Sensor Contact to Ground
2. DHT22 - for temperature and humidity

Customization: 
1. Copy_bin_to_AWS_Server - change server name, keys, passwords and directories
2. defaults.h/.cpp - change pinouts for different board and Sensors
3. add/delete modifiy for different sensor modules

LED assignment/meaning: //TODO - needs updating.....

On Boot: 
1. "Christmas Tree" display with All LED's on to verify wiring and operation of the unit
2. LED1 - Green - Webserver code is actived (On >> Off) and connected to Access Point
                - if it stays lit, then module is in AP mode. Access via SSID: ESPxxxxx 
3. LED2 - Yellow - configuration read from filesystem (On >> Off)
4. LED3 - Red - Sensor is being initialized and has successfully been read (On >> OFF)
5. LED4 - Blue - Communicating with MQTT Broker
6. LED5 - Orange - Check for updates on OTA_SERVER. Note: if OTA_SERVER is on WAN, then AP must allow access to WAN from LAN.  

In run normal run mode: 
1. LED1 - cycle detected OFF>ON>OFF
2. LED2 - flashes ON for 10 seconds and then OFF for 10 seconds - means connected to Gateway
3. LED3 - T/H sensor is being read. 
4. LED4 - heartbeat - code on module is running and connected to AP
5. LED5 - whenever webserver on module is accessed and/or OTA in process
6. BUILTIN_LED - (gpio 2) - Connected to AP 

Issues: 
20200102 - DONE 20200110 - buffer 512>1024 was causing configuration to not be fully read/stored. - not using correct/configured AP (uses last successful connection - even when changed on configuration screen)
20200101 - DONE 20200110 - buffer 512>1024.  configured for MQTT as MILwright.pagoi.com - should be local as this RPi3-MD.local should not be forwarding internet traffic << verify?
20200102 - DONE 20200104 - added new configuration for LED1, LED2, ... LED5 so that alias to Relays
20190131 - DONE 20200113 - updated MQTT topic and Relay handling. Updated index # in string - not responding to MQTT LED toggle for connected - may be related to MQTT broker configuration
20200101 - DONE 20200104 - Updated copyright on all files from -2019 to -2020
20200104 - define client and project attributes as variables that get used by HTML. Change .bat to a python copy which does a regext to replace the attributes on the fly. Also, python to do the .gz compression.
20200113 - Tests: OTA, specific message to a module using to_device topic format, Cycle time reports are correct. 
20200113 - Verify Python on server is correctly parsing and recording responses to/from module(s). Needs to be backward compatible
20200113 - New sensor for vibration using piezo sensor on analog pin - need algorithm to detect peaks 
20200114 - DONE 20200115 Change batch to copy data_uncompressed to data directory with appropriate encoding and copy of files
20200114 - Issue: more than one approach to blinking LEDs in non-blocking method: need to remove/update all DigitalWrite calls  
                1. use Ticker with attach and detach with period to define number of blinks - look like needed by Get_Serial_Data
                2. use JLed library which defines LEDs allows multiple LEDs to be controlled with multiple effects
                3. use Async_Blink array of LEDs and their effects are manually defined
20200117 - Issue: if can't find AP, currently it gets a WDT timeout and then reboots. Needs to be able to go into AP mode from STA mode and then change the valid SSID prefix in the file editor



CLI for upload of binary - add '-e d1_mini' to explicitly just build d1_mini
    pio run -t upload 
CLI for SPIFFS
    pio run -t uploadfs 
        or 
    pio run -t buildfs
    