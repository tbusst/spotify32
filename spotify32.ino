#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "spotify_auth.h"
#include "utils.h"
#include "spotify_api.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define CHAR_WIDTH 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long startTime;
unsigned long lastApiCallTime = 0;
unsigned long lastPlayerCall = 0;
unsigned long lastXIncrease = 0;
const unsigned long requestInterval = 5000;

TrackInfo trackInfo;
TokenInfo tokenInfo;

bool currentlyActive = false;
bool trackUpdated = false; // signal when data is fresh

String refreshToken;

bool lastNextState = LOW; 
bool lastPausePlayState = LOW; 
bool lastPrevState = LOW; 


int x = 0;

int getVolume(int pin) {
  float pot = analogRead(pin);
  int volume = (pot / 4095.0) * 100;
  return constrain(volume, 0, 100);
}

String secsToMins(int seconds) {
  int mins = seconds / 60;
  int secs = seconds % 60;

  // Format as MM:SS with leading zero for seconds
  return String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
}


void fetchSpotifyTask(void* parameter) {
  while (true) {
    // Fetch immediately
    TrackInfo result = getCurrentlyPlaying(tokenInfo.access_token, tokenInfo.token_type);

    if (result.trackName != "") {
      currentlyActive = true;
    } else {
      currentlyActive = false;
    }

    trackInfo = result;
    trackUpdated = true;

    // Wait AFTER first fetch
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(18, INPUT);
  pinMode(17, INPUT);
  pinMode(16, INPUT);
  startTime = millis();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.setTextWrap(false);

  // Splash screen
  display.clearDisplay();
  display.setTextSize(2);             
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("Spotify32");
  display.display(); 

  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  String ip = connectToWifi();
  display.println("Connected to");
  display.println(SSID);
  display.println("Local ESP32 IP: ");
  display.println(ip);
  display.display();

  refreshToken = readEEPROM();
  if (refreshToken == "") {
    refreshToken = getRefreshToken();
  } 
  tokenInfo = refreshAccessToken(refreshToken);
  if (tokenInfo.access_token == "") {
    Serial.println("Failed to refresh token. Getting a new one...");
    refreshToken = getRefreshToken();
    tokenInfo = refreshAccessToken(refreshToken);
  }

  xTaskCreatePinnedToCore(
    fetchSpotifyTask,
    "SpotifyTask",
    8192,        // stack size
    NULL,        // task parameter
    1,           // priority
    NULL,        // task handle
    1            // core (1 = core1, leave 0 if you need Wi-Fi time)
  );
}

void loop() {
  // Scroll logic for artist name
  if (millis() - lastXIncrease >= 100) {
    x--;
    lastXIncrease = millis();
  }

  int textWidth = CHAR_WIDTH * trackInfo.trackName.length();
  if (x < -textWidth) {
    x = SCREEN_WIDTH;
  }

  // Check if a track is currently playing
  if (currentlyActive) {
    display.clearDisplay();  // Clear the display for the new frame

    if (trackUpdated) {
      trackUpdated = false;
    }

    // Display volume percentage at the top-right corner
    display.setTextSize(1);
    display.setCursor(115, 0);
    display.print(trackInfo.volumePercent);
    display.setTextSize(1);
    display.setCursor(0, 27);
    display.print(trackInfo.artistName);

    float progress = (float) trackInfo.progress/trackInfo.trackLength * 100;
    String current = secsToMins(trackInfo.progress/1000);
    String end = secsToMins(trackInfo.trackLength / 1000);
    
    display.setCursor(0, 48);
    display.print(current);

    display.setCursor(104, 48);
    display.print(end);

    display.drawRect(0, 37, 128, 10, SSD1306_WHITE); // Outer frame
    int barWidth = map(progress, 0, 100, 0, 128); // Map percentage to width
    display.fillRect(0, 37, barWidth, 10, SSD1306_WHITE); // Fill the progress bar

    display.setCursor(0, 57);
    display.println(trackInfo.deviceName);

    // Display artist name and handle scrolling if it's too long
    display.setTextSize(2);
    if (trackInfo.trackName.length() > 11) { 
      display.setCursor(x, 10); 
      display.print(trackInfo.trackName.substring(0, trackInfo.trackName.length() - x));
    } else {
      display.setCursor(0, 10);
      display.print(trackInfo.trackName);
    }

  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);             
    display.println("Please open Spotify");
  }
  display.display();


  int prevState = digitalRead(18);
  int pausePlayState = digitalRead(17);
  int nextState = digitalRead(16);
  int pot = getVolume(36);

  if(nextState && !lastNextState) { Serial.println("Next song"); }
  if(pausePlayState && !lastPausePlayState) { Serial.println("Pause/Play"); }
  if(prevState && !lastPrevState) { Serial.println("Previous song"); }

  lastNextState = nextState; 
  lastPausePlayState = pausePlayState; 
  lastPrevState = prevState; 

  // Serial.printf("Volume: %d\n", pot);

  if ((millis() - lastApiCallTime) >= ((tokenInfo.expires_in*1000) - 60000)) {
    lastApiCallTime = millis();
    tokenInfo = refreshAccessToken(refreshToken);
    Serial.println("Refreshed access token.");
  }
}