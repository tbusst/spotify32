#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <EEPROM.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define EEPROM_SIZE 300 
#define MARKER_ADDR 0
#define TOKEN_ADDR 1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "The Whitehouse";
const char* password = "ChrisLuxon";

unsigned long startTime;
unsigned long lastApiCallTime = 0;
unsigned long lastPlayerCall = 0;
const unsigned long requestInterval = 5000;

const String redirectUrl = "http://localhost:8888/callback";

String serverName = "https://accounts.spotify.com/api/token";
String authToken = "AQCY2eCMwPDPupBRwIJYB1iuJ3nZAcqQGBIY7RVbDWrRVdE0L1jiUqLCtT2CTcMPZuLzzkqexAz8PzVW_DX31_9JNNwcSz273rvdIOhvbNYpIYAWPt6lqERvQkZfa3PwNKvxyynIrBR994zBNMod09t6_yUsqrhBV31NPJQ5rLRBNDOAqgboKxIryojfzKmgVnN9L6tHF02HgMU9RosK5v6tC7t8p0N0wTj_jHql7GImn4mADdNBaXc";
String clientID = "27eb9faa532c4f3e9a543ac0004c3d4c";
String clientSecret = "34b6f9eb5c394456a1de213ee50f7720";

String refreshToken;

struct TokenInfo {
  String access_token;
  String token_type;
  int expires_in;
};
TokenInfo tokenInfo;

struct TrackInfo {
  String trackName;
  String artistName;
  String albumName;
  int trackLength;
  int progress;
  bool isPlaying;
  String deviceName;
  int volumePercent;
};
TrackInfo trackInfo;

bool currentlyActive = false;

TokenInfo getRefreshToken(String authCode) {
  String access_token = "";
  String token_type = "";
  int expires_in = -1;

  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    client.setInsecure(); // ⚠️ Use only for testing

    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "grant_type=authorization_code&code="+authCode+"&redirect_uri="+redirectUrl+"&client_id="+clientID+"&client_secret="+clientSecret;           

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

  TokenInfo tokenInfo;
  tokenInfo.access_token = access_token;
  tokenInfo.token_type = token_type;
  tokenInfo.expires_in = expires_in;

  return tokenInfo;
}

TokenInfo refreshAccessToken() {
  String access_token = "";
  String token_type = "";
  int expires_in = -1;
  String scope = "";

  if(WiFi.status()== WL_CONNECTED){
    WiFiClientSecure client;
    client.setInsecure(); // ⚠️ Use only for testing

    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "grant_type=refresh_token&refresh_token="+refreshToken+"&client_id="+clientID+"&client_secret="+clientSecret;           

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
      Serial.println("\nError refreshing access token: ");
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

void getCurrentlyPlaying(String access_token, String token_type) {

  HTTPClient http;
  http.setTimeout(5000);  // <--- Add timeout
  http.begin("https://api.spotify.com/v1/me/player");
  http.addHeader("Authorization", token_type + " " + access_token);

  int httpResponseCode = http.GET();
  Serial.print("HTTP response code: ");
  Serial.println(httpResponseCode);

  String response = "{}";
  if (httpResponseCode == 204) {
    Serial.println("No content in response (204)");
  } else if (httpResponseCode > 0) {
    response = http.getString();    
    trackInfo = parseTrackInfo(response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
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

  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  display.println("Connected to");
  display.println(ssid);
  display.println("Local ESP32 IP: ");
  display.println(WiFi.localIP());
  display.display();

  refreshToken = readEEPROM();
  if (refreshToken == "") {
    tokenInfo = getRefreshToken(authToken);
  } else {
    tokenInfo = refreshAccessToken();
  }
}

int getVolume(int pin) {
  float pot = analogRead(pin);
  float volume = (pot/4095) * 100;
  return volume;
}

void loop() {

  display.clearDisplay();
  display.setCursor(0, 0);

  if (millis() - lastPlayerCall >= requestInterval) {
    lastPlayerCall = millis();
    getCurrentlyPlaying(tokenInfo.access_token, tokenInfo.token_type);
    TrackInfo emptyInfo;
    currentlyActive = !memcmp(&trackInfo, &emptyInfo, sizeof(TrackInfo)) == 0;
  }

  if (currentlyActive) {
    display.println(trackInfo.volumePercent);
    display.println(trackInfo.trackName);
    display.println(trackInfo.artistName);
    float progress = (float) trackInfo.progress/trackInfo.trackLength;
    display.println(trackInfo.progress/1000 + String(" ") + progress + String(" ") + trackInfo.trackLength / 1000);
  } else {
    display.println("Please open Spotify");
  }

  int red = digitalRead(18);
  int yellow = digitalRead(17);
  int green = digitalRead(16);
  int pot = getVolume(36);

  if(red == HIGH) {
    display.println("Previous song"); 
  }

  if(yellow == HIGH) {
    display.println("Pause/Play");
  }

  if(green == HIGH) {
    display.println("Next song");
  }

  display.printf("Volume: %d\n", pot);
  display.display(); 

  if ((millis() - lastApiCallTime) >= ((tokenInfo.expires_in*1000) - 60000)) {
    lastApiCallTime = millis();
    tokenInfo = refreshAccessToken();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Access token:");
    display.println(tokenInfo.access_token);
    display.display();
  }
}