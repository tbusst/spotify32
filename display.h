#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "spotify_api.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define MAX_TITLE_LENGTH 10

void initScreen();
void displayWiFi(String ip, String ssid);
void displayTrack(TrackInfo trackInfo, bool currentlyActive, bool trackUpdated);

#endif