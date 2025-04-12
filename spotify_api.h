#ifndef SPOTIFY_API_H
#define SPOTIFY_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFi.h>

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

TrackInfo parseTrackInfo(String jsonString);
TrackInfo getCurrentlyPlaying(String access_token, String token_type);
//String getArtistData(String access_token, String token_type);

#endif