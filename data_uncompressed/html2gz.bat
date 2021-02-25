echo Creating compressed versions of html files and copy to ..\data directory
rem Note: This file must run from the uncompressed directory
cd C:\Users\carmb\CloudStation\Arduino\User_Sketches\PaGoI-MILwright\data_uncompressed

x:\installs\Utilities\7z1701-extra\7za.exe a admin.htm.gz admin.htm
x:\installs\Utilities\7z1701-extra\7za.exe a config_module.htm.gz config_module.htm
x:\installs\Utilities\7z1701-extra\7za.exe a config_mqtt.htm.gz config_mqtt.htm
x:\installs\Utilities\7z1701-extra\7za.exe a config_network.htm.gz config_network.htm
x:\installs\Utilities\7z1701-extra\7za.exe a edit.htm.gz edit.htm
x:\installs\Utilities\7z1701-extra\7za.exe a general.htm.gz general.htm
x:\installs\Utilities\7z1701-extra\7za.exe a index.htm.gz   index.htm
x:\installs\Utilities\7z1701-extra\7za.exe a info.htm.gz info.htm
x:\installs\Utilities\7z1701-extra\7za.exe a install.htm.gz install.htm
x:\installs\Utilities\7z1701-extra\7za.exe a ntp.htm.gz ntp.htm
x:\installs\Utilities\7z1701-extra\7za.exe a OTA_update.htm.gz  OTA_update.htm
x:\installs\Utilities\7z1701-extra\7za.exe a Rebooting.htm.gz   Rebooting.htm
x:\installs\Utilities\7z1701-extra\7za.exe a Redirect2Cloud.htm.gz  Redirect2Cloud.htm
x:\installs\Utilities\7z1701-extra\7za.exe a Redirect2Gateway.htm.gz Redirect2Gateway.htm
x:\installs\Utilities\7z1701-extra\7za.exe a status_th.htm.gz status_th.htm
x:\installs\Utilities\7z1701-extra\7za.exe a system.htm.gz  system.htm
x:\installs\Utilities\7z1701-extra\7za.exe a WebsocketTest.htm.gz WebsocketTest.htm

ren *.gz *.jgz
del ..\data\*.jgz
del ..\data\*.htm
move /y *.jgz ..\data\.
rem Note: Only files subdirectories listed above are copied. Others which are fairly static, must be manually copied.
REM C:\Users\carmb\CloudStation\Clients\MILwright\ESP01_Master_Imaging\tool-mkspiffs\mkspiffs -c ..\data -p 256 -b 4096 -s 163840 C:\Users\carmb\CloudStation\Clients\MILwright\ESP01_Master_Imaging\Images\esp01_1m_160_spiffs.bin
REM u:\Clients\MILwright\ESP01_Master_Imaging\tool-mkspiffs\mkspiffs -c data -p 256 -b 8192 -s 1028096 C:\Users\carmb\CloudStation\Clients\MILwright\ESP01_Master_Imaging\Images\d1_mini_spiffs.bin
REM ..\pio run --target uploadfs
