#!/bin/bash
cd '/var/www/html/OTA_Images'
# echo Usage: xxx of ip, name of binary, name of filesystem
IP_BASE="192.168.100"
echo >>>>>>>>>>>>>>>>>> Updating IP:$IP_BASE.$1, new binary: $2, new spiffs: $3
# echo "Updating $IP_BASE.$1"
ping $IP_BASE.$1 -c 1 -w 1000
echo Updating firmware . . . 
python3 /home/pi/Documents/MachineMonitor/Tools/espota.py -i $IP_BASE.$1 -p 8266 --auth=1234 -f $2 > /dev/null 2>> error.log
# give the ESP time to start reboot
sleep 3
echo Waiting for upload to finish . . . | ts
# listen for when reconnected to WiFi
ping $IP_BASE.$1 -c 1 -w 40000
echo Updated. Waiting for reboot to finish . . . | ts
# wait for reboot to complete
sleep 15
echo  Now Updating the file system with spiffs.bin of $3 ... | ts
python3	/home/pi/Documents/MachineMonitor/Tools/espota.py -i $IP_BASE.$1 -p 8266 --auth=1234 -s -f $3 > /dev/null 2>> error.log
# wait for reboot to start
sleep 3
# listen for when reconnected to WiFi
ping $IP_BASE.$1 -c 1 -w 30000 | ts
echo Press Monitor Module at $IP_BASE.$1 is rebooting. Uploads are finished.  <<<<<<<<<<<<<<<<| ts

