#ifndef SPOTIFY_AUTH_H
#define SPOTIFY_AUTH_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "secrets.h"

#define REDIRECT_URL "http://localhost:8888/callback"
#define SERVER_NAME "https://accounts.spotify.com/api/token"

struct TokenInfo {
  String access_token;
  String token_type;
  int expires_in;
};

String getRefreshToken();
TokenInfo refreshAccessToken(String refreshToken);

#endif