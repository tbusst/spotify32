#include "spotify_auth.h"
#include "utils.h"
#include "spotify_api.h"
#include "display.h"


unsigned long startTime;
unsigned long lastApiCallTime = 0;
unsigned long lastPlayerCall = 0;
const unsigned long requestInterval = 5000;

TrackInfo trackInfo;
TokenInfo tokenInfo;

String refreshToken;

bool currentlyActive = false;
bool trackUpdated = false;

bool lastNextState = LOW; 
bool lastPausePlayState = LOW; 
bool lastPrevState = LOW; 

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
    vTaskDelay(requestInterval / portTICK_PERIOD_MS);
  }
}

int getVolume(int pin) {
  float pot = analogRead(pin);
  int volume = (pot / 4095.0) * 100;
  return constrain(volume, 0, 100);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(18, INPUT);
  pinMode(17, INPUT);
  pinMode(16, INPUT);
  startTime = millis();

  initScreen();
  String ip = connectToWifi();
  displayWiFi(ip, SSID);

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
    1            // core 
  );
}

void loop() {
  displayTrack(trackInfo, currentlyActive, trackUpdated);

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

  if ((millis() - lastApiCallTime) >= ((tokenInfo.expires_in*1000) - 60000)) {
    lastApiCallTime = millis();
    tokenInfo = refreshAccessToken(refreshToken);
    Serial.println("Refreshed access token.");
  }
}