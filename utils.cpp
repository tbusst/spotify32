#include "utils.h"

String connectToWifi() {
  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(SSID, PASSWORD);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  return WiFi.localIP().toString();
}

void writeEEPROM(String toWrite) {
  int maxLen = EEPROM_SIZE - TOKEN_ADDR - 1; 
  if (maxLen < toWrite.length()) {
    Serial.println("Warning, overflow prevented");
  } else {
    EEPROM.write(MARKER_ADDR, 0x42);
    for (int i = 0; i < toWrite.length(); ++i) {
      EEPROM.write(TOKEN_ADDR + i, toWrite[i]);
    }
    EEPROM.write(TOKEN_ADDR + toWrite.length(), '\0');
    EEPROM.commit();
  }
} 

String readEEPROM() {
  if (EEPROM.read(MARKER_ADDR) != 0x42) {
    Serial.println("No valid token in EEPROM.");
    return "";
  }

  char storedToken[EEPROM_SIZE];  // Make sure this is big enough
  for (int i = 0; i < EEPROM_SIZE - TOKEN_ADDR; ++i) {
    storedToken[i] = EEPROM.read(TOKEN_ADDR + i);
    if (storedToken[i] == '\0') break;
  }
  return String(storedToken);
}