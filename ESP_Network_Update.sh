cd '/var/www/html/OTA_Images'
echo Usage: xxx of ip, name of binary, name of filesystem
IP_BASE="192.168.100"
echo Updating IP:$IP_BASE.$1, new binary: $2, new spiffs: $3
# echo "Updating $IP_BASE.$1"
python /home/pi/Documents/MachineMonitor/Tools/espota.py -i $IP_BASE.$1 -p 8266 --auth=1234 -f $2 -d -r
echo Sleeping and then upload the file system $3
sleep 45
python	/home/pi/Documents/MachineMonitor/Tools/espota.py -i $IP_BASE.$1 -p 8266 --auth=1234 -s -f $3 -d -r
sleep 15
echo $1 is now Updated
