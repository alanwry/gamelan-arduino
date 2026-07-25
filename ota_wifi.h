#ifndef OTA_WIFI_H
#define OTA_WIFI_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

class OtaWifiManager {
public:
  void begin();
  void beginSTA();
  void stopSTA();
  void update();
  void saveConfig(String ssid, String pass, bool staEnabled);
  void getConfig(String &ssid, String &pass, bool &staEnabled);
  bool isStaEnabled();
private:
  Preferences prefs;
  String ssid;
  String password;
  bool staEnabled;
};

extern OtaWifiManager otaWifi;

#endif
