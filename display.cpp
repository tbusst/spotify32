#include "display.h"

int x = 0;
unsigned long lastXIncrease = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

String secsToMins(int seconds) {
  int mins = seconds / 60;
  int secs = seconds % 60;
  return String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
}

void initScreen() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  u8g2Fonts.begin(display);

  // Splash screen
  display.clearDisplay();
  display.setTextColor(WHITE);
  u8g2Fonts.setFont(u8g2_font_9x15_mf);
  String splashString = "Spotify32";
  int splashWidth = u8g2Fonts.getUTF8Width(splashString.c_str());
  int xPos = (SCREEN_WIDTH - splashWidth)/2;
  int yPos = (SCREEN_HEIGHT+15)/2;
  u8g2Fonts.setCursor(xPos, yPos);
  u8g2Fonts.println("Spotify32");
  display.display(); 

  delay(2000);
  display.clearDisplay();

  u8g2Fonts.setFont(u8g2_font_5x8_mf);
  u8g2Fonts.setCursor(0, 10);
}

void displayWiFi(String ip, String SSID) {
  u8g2Fonts.setFont(u8g2_font_6x10_mf);
  u8g2Fonts.setCursor(0, 0);
  u8g2Fonts.println("Connected to");
  u8g2Fonts.println(SSID);
  u8g2Fonts.println("Local ESP32 IP: ");
  u8g2Fonts.println(ip);
  display.display();
}

void displayTrack(TrackInfo trackInfo, bool currentlyActive, bool trackUpdated) {
  display.clearDisplay();

  // Scroll logic for artist name
  if (millis() - lastXIncrease >= 75) {
    x--;
    lastXIncrease = millis();
  }

  u8g2Fonts.setFont(u8g2_font_8x13_mf);
  int textWidth = u8g2Fonts.getUTF8Width(trackInfo.trackName.c_str());
  if (x < -textWidth) {
    x = SCREEN_WIDTH;
  }
  u8g2Fonts.setFont(u8g2_font_5x8_mf);

  // Check if a track is currently playing
  if (currentlyActive) {
      // Clear the display for the new frame

    if (trackUpdated) {
      trackUpdated = false;
    }

    // Display volume percentage at the top-right corner
    // u8g2Fonts.setCursor(119, 6);
    // if (volume == 100) { u8g2Fonts.setCursor(113,6); }
    // u8g2Fonts.print(volume);
    u8g2Fonts.setCursor(0, 34);
    u8g2Fonts.print(trackInfo.artistName);

    float progress = (float) trackInfo.progress/trackInfo.trackLength * 100;
    String current = secsToMins(trackInfo.progress/1000);
    String end = secsToMins(trackInfo.trackLength / 1000);
    
    u8g2Fonts.setCursor(0, 54);
    u8g2Fonts.print(current);

    u8g2Fonts.setCursor(109, 54);
    u8g2Fonts.print(end);

    display.drawRect(0, 37, 128, 10, SSD1306_WHITE); // Outer frame
    int barWidth = map(progress, 0, 100, 0, 128); // Map percentage to width
    display.fillRect(0, 37, barWidth, 10, SSD1306_WHITE); // Fill the progress bar

    u8g2Fonts.setCursor(0, 63);
    u8g2Fonts.println(trackInfo.deviceName);

    // Display artist name and handle scrolling if it's too long
    u8g2Fonts.setFont(u8g2_font_8x13_mf);
    if (textWidth > SCREEN_WIDTH) { 
      u8g2Fonts.setCursor(x, 21); 
      u8g2Fonts.print(trackInfo.trackName.substring(0, trackInfo.trackName.length() - x));
    } else {
      u8g2Fonts.setCursor(0, 21);
      u8g2Fonts.print(trackInfo.trackName);
    }
    u8g2Fonts.setFont(u8g2_font_5x8_mf);

  } else {
    u8g2Fonts.setFont(u8g2_font_6x10_mf);
    String errorString = "Please open Spotify";
    int errorWidth = u8g2Fonts.getUTF8Width(errorString.c_str());
    int xPos = (SCREEN_WIDTH - errorWidth)/2;
    int yPos = (SCREEN_HEIGHT+10)/2;
    u8g2Fonts.setCursor(xPos,yPos);
    u8g2Fonts.println(errorString);
    u8g2Fonts.setFont(u8g2_font_5x8_mf);
  }
  display.display();
}