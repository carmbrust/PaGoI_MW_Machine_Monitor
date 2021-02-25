filename: readme.txt
date: 12Mar2018
<updated:>15Jan2020</updated:>
<Goal:>Keep spiffs.bin file under 160k to fit in the limited 1M memory on the ESP01. </Goal:>
Note - the memory size available for the SPIFFS is a function of 1M-2x(firmware.bin size)

Notes:
1. This directory contains the "master" of files that will end up in the .'.\data' directory
2. Edit these files (and the subdirectories excluding the archive and nppBackup directories)
3. NOTE: file names with directory name must be <=30 characters or won't be found in SPIFFS. May crash on SPIFFS.bin creation also. 
4. To release for download to the ESP, run the prep_data_folder.py
			 - batch file compresses, renames and copies the needed files to the data directory
5. The data directory can then be used to build the spiffs.bin file
	example: "mkspiffs" -c data -p 256 -b 4096 -s 196608 .pioenvs\esp01_1m_192\spiffs.bin

for 192k: 	
\\DISKSTATION\home\CloudStation\Arduino\mkspiffs-0.1.2-windows\mkspiffs-0.1.2-windows\mkspiffs -c data -p 256 -b 4096 -s 196608 .pioenvs\esp01_1m_192\spiffs.bin
for 160k:
U:\Arduino\mkspiffs-0.1.2-windows\mkspiffs-0.1.2-windows\mkspiffs -c data -p 256 -b 4096 -s 163840 .pioenvs\esp01_1m_160\spiffs.bin


