Version: 20191003 
This directory contains released html, js, and css that supports basic configuration.
The files with the .jgz extention which is compressed reduce space used in the SPIFFS
The uncompressed files are found in ..\data_uncompressed

Notes: 
1. Keep firmware.bin as small as possible - limited by 1M-2x(firmware.bin size)-spiffs.bin where spiffs.bin is 160k. 
2. archive or delete the unused extra files.
    a. in base/root directory: DONE
    b. //TODO subdirectories
3. If compiled version of code to less than 400k and then can use 192k SPIFFS and still get OTA
    a. When firware.bin is more than 400k, use 160k SPIFFS and bootstrap firmware. 
4. Bootstrap .bin which is less than 400k will reload this data directory (_spiffs.bin) and then download the latest _firmware.bin and reboot

Inspired by: https://github.com/gmag11/FSBrowserNG

