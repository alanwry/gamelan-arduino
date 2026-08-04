#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class WiFiManager {
public:
  WiFiManager();
  void begin();

  // Mode Control
  void startAP();
  void startSTA();
  void stopAll();

  // Settings
  void saveSettings(String ssid, String password, bool enableSTA);
  void getSettings(String &ssid, String &password, bool &enableSTA);
  String getSSID() const { return ssid; }
  String getPassword() const { return password; }

  bool isSTAEnabled();

private:
  Preferences prefs;
  String ssid;
  String password;
  bool enableSTA;

  void loadFromPrefs();
};

extern WiFiManager wifiManager;

#endif
