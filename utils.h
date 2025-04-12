#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "secrets.h"

#define EEPROM_SIZE 300 
#define MARKER_ADDR 0
#define TOKEN_ADDR 1

String connectToWifi();
void writeEEPROM(String toWrite);
String readEEPROM();

#endif