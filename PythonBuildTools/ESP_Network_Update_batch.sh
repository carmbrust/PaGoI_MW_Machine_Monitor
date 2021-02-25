#!/bin/bash
# This script clears the terminal, displays a greeting and gives information
# about currently connected users.  The two example variables are set and displayed.

clear				# clear terminal window

echo "The script starts now."

echo "Hi, $USER!"		# dollar sign is used to get content of variable
echo

echo "I will now fetch you a list of connected users:"
echo							
w				# show who is logged on and
echo				# what they are doing

echo "Update Battery Monitor Nodes"
echo
echo "These devices are on the network"

nmap -sP 192.168.100.0/24| grep scan

<<COMMENT
Should report the following IP's (or more/less)
IP 192.168.100.1
IP 192.168.100.102
IP 192.168.100.105
IP 192.168.100.106
IP 192.168.100.115
IP 192.168.100.116
IP 192.168.100.122
IP 192.168.100.125
COMMENT

#cd '/home/pi/Downloads/Builds'

echo "Updating firmware"
python espota.py -i 192.168.100.102 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.105 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.106 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.115 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.116 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.122 -p 8266 --auth=1234  -f firmware.bin -d -r
python espota.py -i 192.168.100.125 -p 8266 --auth=1234  -f firmware.bin -d -r
#python espota.py -i 192.168.100.165 -p 8266 -a1234 -f .pio\build\d1_mini\firmware.bin -d -r
#python espota.py -i 192.168.4.1 -p 8266 -a1234 -f U:\Clients\Arrgh\Builds\esp01_1m_160_bootstrap_firmware.bin -d -r
#python U:\Clients\Arrgh\Builds\Tools\esptool.py 
#python U:\Clients\Arrgh\Builds\Tools\espota.py -i 192.168.100.184 -p 8266 -a1234 -f U:\Clients\Arrgh\Builds\esp01_1m_160_bootstrap_firmware.bin -d -r
#python U:\Clients\Arrgh\Builds\Tools\espota.py -i 192.168.100.133 -p 8266 -a1234 -f U:\Clients\Arrgh\Builds\d1_mini_bootstrap_firmware.bin -d -r
#curl -F "image=@firmware.bin" esp8266-webupdate.local/update
#curl -F "image=@U:\Clients\Arrgh\Builds\d1_mini_bootstrap_firmware.bin" 192.168.100.133/update
# from MBP: 
# on MBP: /Users/chrisarmbrust/CloudStation/Clients/Arrgh/Builds
# chrisarmbrust@Chris-MBP-3 Builds % python Tools/espota.py -i 192.168.4.1 -p 8266 -a1234 -f esp01_1m_160_bootstrap_firmware.bin -d -r


echo "Updating SPIFFS"
#python espota.py -i <ESP_IP_address> -I <Host_IP_address> -p <ESP_port> -P <HOST_port> [-a password] -s -f <spiffs.bin> -d -r
python espota.py -i 192.168.100.102 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.105 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.106 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.115 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.116 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.122 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
python espota.py -i 192.168.100.125 -p 8266 --auth=1234  -s -f spiffs.bin -d -r
# next command is not working - timeout (No response from device)
#python U:\Clients\Arrgh\Builds\Tools\espota.py -i 192.168.100.184 -p 8266 -a1234 -s -f U:\Clients\Arrgh\Builds\esp01_1m_160_spiffs.bin
# this command will work on the RPi on the same LAN/Subnet as the ESP. 
#python2 /home/pi/Desktop/ESP01_Master_Imaging/tool-espotapy/espota.py -i 192.168.100.143 -p 8266 --auth=1234 -s -f /home/pi/Desktop/ESP01_Master_Imaging/Images/esp01_1m_160_spiffs.bin  -d -r


echo "Updates Complete - waiting 15 seconds ..."
sleep 15
echo "Check that 'everyone' has rebooted"
nmap -sP 192.168.100.0/24| grep scan

echo
echo "Done!"

