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

  for (int attempt = 0; attempt < 2; attempt++) {  // Try twice max
    http.begin("https://api.spotify.com/v1/me/player");
    http.addHeader("Authorization", token_type + " " + access_token);
    http.addHeader("Connection", "keep-alive");

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      response = http.getString();
      trackInfo = parseTrackInfo(response);
    } else if (httpResponseCode == 204) {
      Serial.println("No content in response (204)");
      response = "{}";
      break;  // No track playing — valid response
    } else{
      Serial.print("Error getting playback (attempt ");
      Serial.print(attempt + 1);
      Serial.print("): ");
      Serial.println(httpResponseCode);
      http.end();  // Clean up before retry
      delay(500);
    }

  }
  http.end();
  return trackInfo;
}

// void changeVolume(String access_token, String token_type, int volume) {
//   HTTPClient http;
//   http.setTimeout(10000);
//   for (int attempt = 0; attempt < 2; attempt++) {
//     http.begin("https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(volume));
//     http.addHeader("Authorization", token_type + " " + access_token);
//     http.addHeader("Connection", "keep-alive");
//     http.addHeader("Content-Length", "0");

//     int httpResponseCode = http.PUT("");  // Spotify API expects no body
//     if (httpResponseCode == 204) {  // 204 No Content is expected on success
//       http.end();
//       return;
//     } else {
//       Serial.print("Error setting volume (attempt ");
//       Serial.print(attempt + 1);
//       Serial.print("): ");
//       Serial.println(httpResponseCode);
//     }

//     http.end();  // Clean up
//     delay(500);
//   }

//   Serial.println("Failed to set volume after 2 attempts.");
// }

// void togglePlayState(String access_token, String token_type) {
//   // Step 1: Fetch current playback state
//   HTTPClient getHttp;
//   getHttp.setTimeout(10000);
//   getHttp.begin("https://api.spotify.com/v1/me/player");
//   getHttp.addHeader("Authorization", token_type + " " + access_token);

//   int code = getHttp.GET();
//   if (code == 200) {
//     String payload = getHttp.getString();
//     getHttp.end();

//     bool isCurrentlyPlaying = payload.indexOf("\"is_playing\":true") != -1;

//     // Step 2: Toggle playback
//     HTTPClient toggleHttp;
//     toggleHttp.setTimeout(10000);
//     if (isCurrentlyPlaying) {
//       toggleHttp.begin("https://api.spotify.com/v1/me/player/pause");
//     } else {
//       toggleHttp.begin("https://api.spotify.com/v1/me/player/play");
//     }

//     toggleHttp.addHeader("Authorization", token_type + " " + access_token);
//     toggleHttp.addHeader("Content-Length", "0");

//     int toggleCode = toggleHttp.PUT("");
//     if (toggleCode == 204 || toggleCode == 200) {
//       Serial.println("Toggled play state successfully.");
//     } else {
//       Serial.print("Toggle play state failed. Code: ");
//       Serial.println(toggleCode);

//       String responseBody = toggleHttp.getString();  // show body if something went wrong
//       Serial.println("Response body: " + responseBody);
//     }

//     toggleHttp.end();

//   } else {
//     Serial.print("Failed to fetch play state: ");
//     Serial.println(code);
//     String errorBody = getHttp.getString();  // Show if there's any body in the error
//     Serial.println("Error response: " + errorBody);
//     getHttp.end();
//   }
// }

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