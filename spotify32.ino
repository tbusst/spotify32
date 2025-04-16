#include "spotify_auth.h"
#include "utils.h"
#include "spotify_api.h"
#include "display.h"
// #include "freertos/semphr.h"

// #define VOLUME_BLOCKING 500
// SemaphoreHandle_t volumeSemaphore;
// SemaphoreHandle_t playStateSemaphore;
// const int volumeChangeThreshold = 5;  // Only trigger if change is >= 5

unsigned long startTime;
unsigned long lastApiCallTime = 0;
unsigned long lastPlayerCall = 0;
// unsigned long startVolumeBuffer = 0;
const unsigned long requestInterval = 5000;

// const unsigned long debounceDelay = 50;  // milliseconds

// unsigned long lastNextDebounceTime = 0;
// unsigned long lastPausePlayDebounceTime = 0;
// unsigned long lastPrevDebounceTime = 0;

TrackInfo trackInfo;
TokenInfo tokenInfo;

String refreshToken;

bool currentlyActive = false;
bool trackUpdated = false;

// bool lastNextState = LOW; 
// bool lastPausePlayState = LOW; 
// bool lastPrevState = LOW; 

// bool volumeChanged = false;
// int lastVolume = 0;
// int volume = 0;

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

// void setVolumeTask(void* parameter) {
//   while (true) {
//     // Fetch immediately
//     if (xSemaphoreTake(volumeSemaphore, portMAX_DELAY) == pdTRUE) {
//       while ((millis() - startVolumeBuffer) < VOLUME_BLOCKING) {
//         vTaskDelay(50 / portTICK_PERIOD_MS);  // prevent blocking
//       }
//       if (volume != trackInfo.volumePercent) {
//         changeVolume(tokenInfo.access_token, tokenInfo.token_type, volume);
//         volumeChanged = false;
//       }
//     }
//   }
// }

// void togglePlayStateTask(void* parameter) {
//   while (true) {
//     if (xSemaphoreTake(playStateSemaphore, portMAX_DELAY) == pdTRUE) {
//       if (tokenInfo.access_token != "" && tokenInfo.token_type != "") {
//         togglePlayState(tokenInfo.access_token, tokenInfo.token_type);
//       } else {
//         Serial.println("Play state change skipped — invalid token");
//       }
//     }
//   }
// }


// int getVolume(int pin) {
//   static float smoothed = 0;
//   const float alpha = 0.1;

//   int raw = analogRead(pin);
//   smoothed = alpha * raw + (1 - alpha) * smoothed;

//   // Scale to 0–100 and round to nearest multiple of 5
//   int volume = round((smoothed / 4095.0) * 100);
//   volume = round(volume / 5.0) * 5;

//   return constrain(volume, 0, 100);
// }

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // volumeSemaphore = xSemaphoreCreateBinary();
  // playStateSemaphore = xSemaphoreCreateBinary();

  pinMode(18, INPUT);
  pinMode(17, INPUT);
  pinMode(16, INPUT);
  startTime = millis();

  initScreen();
  String ip = connectToWifi();
  //displayWiFi(ip, SSID);

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

  // for (int i = 0; i < 10; i++) getVolume(36);  // warm up smoothing
  // lastVolume = getVolume(36);
  // volume = lastVolume;
  // trackInfo.volumePercent = volume;  // ✅ align volume on startup

  xTaskCreatePinnedToCore(
    fetchSpotifyTask,
    "SpotifyTask",
    8192,        // stack size
    NULL,        // task parameter
    1,           // priority
    NULL,        // task handle
    1            // core 
  );

  // xTaskCreatePinnedToCore(
  //   setVolumeTask,
  //   "VolumeTask",
  //   8192,        // stack size
  //   NULL,        // task parameter
  //   1,           // priority
  //   NULL,        // task handle
  //   0            // core 
  // );

  // xTaskCreatePinnedToCore(
  //   togglePlayStateTask,
  //   "PlayStateTask",
  //   8192,        // stack size
  //   NULL,        // task parameter
  //   1,           // priority
  //   NULL,        // task handle
  //   0            // core 
  // );
}

void loop() {
  // displayTrack(trackInfo, currentlyActive, trackUpdated, volume);
  displayTrack(trackInfo, currentlyActive, trackUpdated);
  unsigned long now = millis();

  // int currentNextState = digitalRead(16);
  // int currentPausePlayState = digitalRead(17);
  // int currentPrevState = digitalRead(18);

  // if(currentNextState && !lastNextState && (now - lastNextDebounceTime > debounceDelay)) { Serial.println("Next song"); }
  // if(currentPausePlayState && !lastPausePlayState && (now - lastPausePlayDebounceTime > debounceDelay)) { 
  //   xSemaphoreGive(playStateSemaphore);
  // }
  // if(currentPrevState && !lastPrevState && (now - lastPrevDebounceTime > debounceDelay)) { Serial.println("Previous song"); }

  // lastNextState = currentNextState; 
  // lastPausePlayState = currentPausePlayState; 
  // lastPrevState = currentPrevState; 

  // volume = getVolume(36);
  // if (abs(volume - trackInfo.volumePercent) >= volumeChangeThreshold && !volumeChanged) {
  //   startVolumeBuffer = now; 
  //   volumeChanged = true;
  //   xSemaphoreGive(volumeSemaphore);
  // }

  if ((now - lastApiCallTime) >= ((tokenInfo.expires_in*1000) - 60000)) {
    lastApiCallTime = now;
    tokenInfo = refreshAccessToken(refreshToken);
    Serial.println("Refreshed access token.");
  }
}