#ifndef CRED_H
#define CRED_H

#include "main.h"
// EEPROM library uses one sector of flash located just after the SPIFFS.
// Size can be anywhere between 4 and 4096 bytes in even increments of 4 bytes 
// as a read/write block is 4 bytes.

#define EEPROM_SIZE 512
#include <EEPROM.h>

struct Credentials {
    char    ssid[32];
    char    password[32];
} ;

extern Credentials loadCredentials();
extern void saveCredentials(Credentials &newAP);
extern void clearCredentials();




#endif