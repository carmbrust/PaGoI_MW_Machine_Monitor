echo off
rem build and upload program and filesystem
rem python u:\Arduino\tool_spiffgenpy\spiffsgen.py 3125248 data spiffs.bin
REM C:\Users\carmb\.platformio\packages\tool-mkspiffs\mkspiffs -c data -b 4096 -s 1028096 image.spiffs
REM pio run --environment d1_mini -t buildfs
REM pio run --environment d1_mini

REM python C:\Users\carmb\.platformio\packages\tool-esptoolpy\esptool.py --port com11  write_flash -fm dio -fs 4MB 0x000000 C:\Users\carmb\CloudStation\Arduino\User_Sketches\PaGoI-MILwright\.pio\build\d1_mini\firmware.bin 0x300000 C:\Users\carmb\CloudStation\Arduino\User_Sketches\PaGoI-MILwright\.pio\build\d1_mini\spiffs.bin
REM pio device monitor -p com11

REM Initialize ESP: 
cd .pio/build/d1_mini
esptool.py --port COM6 erase_flash  
@REM esptool.py --port COM10 --baud 921600  write_flash -fm dio -fs 4MB 0x00000 firmware.bin 0x4050000 spiffs_test2.bin
esptool.py --port COM6 --baud 921600  write_flash -fm dio -fs 4MB 0x00000 firmware.bin 0x300000 spiffs.bin
pio device monitor -p COM6 -b 9600
cd ../../..