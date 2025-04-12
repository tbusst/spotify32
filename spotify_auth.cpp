#include "spotify_auth.h"
#include "utils.h"

String getRefreshToken() {
  String access_token = "";
  String refreshToken = "";
  String token_type = "";
  int expires_in = -1;

  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    client.setInsecure(); // ⚠️ Use only for testing

    HTTPClient http;
    http.begin(client, SERVER_NAME);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = String("grant_type=authorization_code&code=")+AUTH_TOKEN+"&redirect_uri="+REDIRECT_URL+"&client_id="+CLIENT_ID+"&client_secret="+CLIENT_SECRET;           

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      JSONVar doc = JSON.parse(response);

      if (JSON.typeof(doc) == "undefined") {
        Serial.print("JSON Parsing failed!");
      } else {
        access_token = (const char*) doc["access_token"];
        token_type = (const char*) doc["token_type"];
        expires_in = (int) doc["expires_in"];
        refreshToken = (const char*) doc["refresh_token"];
        if (refreshToken.length() > 0) {
          Serial.println("Retrieving refresh token:");
          Serial.println(refreshToken);
          writeEEPROM(refreshToken);
        } else {
          Serial.println("Failed to retrieve refresh token");
        }
      }

    } else {
      Serial.println("\nError on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  }

  return refreshToken;
}

TokenInfo refreshAccessToken(String refreshToken) {
  String access_token = "";
  String token_type = "";
  int expires_in = -1;
  String scope = "";

  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    client.setInsecure(); // ⚠️ Use only for testing

    HTTPClient http;
    http.begin(client, SERVER_NAME);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "grant_type=refresh_token&refresh_token="+refreshToken+"&client_id="+CLIENT_ID+"&client_secret="+CLIENT_SECRET;           

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();

      JSONVar doc = JSON.parse(response);

      if (JSON.typeof(doc) == "undefined") {
        Serial.print("JSON Parsing failed!");
      } else {
        Serial.println("\Refreshed access token.");
        access_token = (const char*) doc["access_token"];
        token_type = (const char*) doc["token_type"];
        expires_in = (int) doc["expires_in"];
      }

    } else {
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  }

  TokenInfo tokenInfo;
  tokenInfo.access_token = access_token;
  tokenInfo.token_type = token_type;
  tokenInfo.expires_in = expires_in;

  return tokenInfo;
}