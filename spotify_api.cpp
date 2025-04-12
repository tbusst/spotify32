#include "HardwareSerial.h"
#include "spotify_api.h"

TrackInfo parseTrackInfo(String jsonString) {
  JSONVar doc = JSON.parse(jsonString);

  TrackInfo trackInfo;
  if (JSON.typeof(doc) == "undefined") {
    Serial.println("Failed to parse JSON");
    return trackInfo; // Return empty info on failure
  }

  trackInfo.trackName = (const char*) doc["item"]["name"];
  trackInfo.artistName = (const char*) doc["item"]["artists"][0]["name"];
  trackInfo.albumName = (const char*) doc["item"]["album"]["name"];
  trackInfo.trackLength = (int) doc["item"]["duration_ms"];
  trackInfo.progress = (int) doc["progress_ms"];
  trackInfo.isPlaying = (bool) doc["is_playing"];
  trackInfo.deviceName = (const char*) doc["device"]["name"];
  trackInfo.volumePercent = (int) doc["device"]["volume_percent"];

  return trackInfo;
}

TrackInfo getCurrentlyPlaying(String access_token, String token_type) {
  TrackInfo trackInfo;
  HTTPClient http;
  http.setTimeout(10000);  // Longer timeout for flaky WiFi

  String response = "{}";
  int httpResponseCode = -1;

  for (int attempt = 0; attempt < 2; attempt++) {  // Try twice max
    http.begin("https://api.spotify.com/v1/me/player");
    http.addHeader("Authorization", token_type + " " + access_token);
    http.addHeader("Connection", "keep-alive");

    httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      response = http.getString();
      trackInfo = parseTrackInfo(response);
      break;  // Success
    } else if (httpResponseCode == 204) {
      Serial.println("No content in response (204)");
      break;  // No track playing — valid response
    } else if (httpResponseCode <= 0) {
      Serial.print("Error code (attempt ");
      Serial.print(attempt + 1);
      Serial.print("): ");
      Serial.println(httpResponseCode);
      http.end();  // Clean up before retry
      delay(500);
    } else {
      Serial.print("Unhandled response: ");
      Serial.print(httpResponseCode);
      Serial.print(", WiFi Status: ");
      Serial.print(WiFi.status());
      Serial.printf(", RSSI: %d dBm\n", WiFi.RSSI());
      break;  // No retry on other errors (e.g., 401, 403, etc.)
    }

    http.end();  // Clean up after each attempt
  }

  return trackInfo;
}


//  String getArtistData(String access_token, String token_type) {
//   HTTPClient http;
//   http.begin("https://api.spotify.com/v1/artists/4V8LLVI7PbaPR0K2TGSxFF");
//   http.addHeader("Authorization", token_type + String(" ") + access_token);

//   int httpResponseCode = http.GET();
//   String response = "{}";

//   if (httpResponseCode>0) {
//     Serial.print("HTTP Response code: ");
//     Serial.println(httpResponseCode);
//     response = http.getString();
//   }
//   else {
//     Serial.print("Error code: ");
//     Serial.println(httpResponseCode);
//   }

//   http.end();
//   return response;
// }