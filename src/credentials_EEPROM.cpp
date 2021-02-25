/*
Project: Machine Monitor WiFi OTA Bootstrap
Filename: credentials_EEPROM.cpp
Client: PaGoI  
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Purpose: store and retrive WLAN (SSID and PASS) using EEPROM
*/
#include "credentials_EEPROM.h"




/** Load WLAN credentials from EEPROM */
Credentials loadCredentials() {
  Credentials eepromVar;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, eepromVar.ssid);
  EEPROM.get(0+sizeof(eepromVar.ssid), eepromVar.password);
  char ok[2+1];
  EEPROM.get(0+sizeof(eepromVar.ssid)+sizeof(eepromVar.password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    eepromVar.ssid[0] = 0;
    eepromVar.password[0] = 0;
  }
  DEBUG_MSG("Recovered credentials: %s, pw: %s\n",eepromVar.ssid, eepromVar.password );
  // Serial.println("Recovered credentials:");
  // Serial.println(eepromVar.ssid);
  // // Serial.println(strlen(eepromVar.password)>0?"********":"<no eepromVar.password>");
  // Serial.println(strlen(eepromVar.password)>0?eepromVar.password:"<no eepromVar.password>");
  return eepromVar;
}

/** Store WLAN credentials to EEPROM */
void saveCredentials(Credentials &newCredentials) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, newCredentials.ssid);
  EEPROM.put(0+sizeof(newCredentials.ssid), newCredentials.password);
  char ok[2+1] = "OK";
  EEPROM.put(0+sizeof(newCredentials.ssid)+sizeof(newCredentials.password), ok);
  EEPROM.commit();
  EEPROM.end();
  // return eepromVar;
}

void clearCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  // write a 0 to all EEPROM_SIZE bytes of the EEPROM
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}