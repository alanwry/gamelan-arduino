#include "ota_wifi.h"
#include "config.h"

OtaWifiManager otaWifi;
static bool otaStarted = false;

void OtaWifiManager::begin() {
  prefs.begin("ota_wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  staEnabled = prefs.getBool("staEnabled", false);
  prefs.end();
  Serial.printf("[OTA_WIFI] Loaded: SSID='%s', STA_ENABLED=%d\n", ssid.c_str(), staEnabled);
}

void OtaWifiManager::beginSTA() {
  if (ssid.length() == 0 || password.length() == 0) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  otaStarted = false; 
}

void OtaWifiManager::stopSTA() {
  ArduinoOTA.end();
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_OFF);
  otaStarted = false;
}

void OtaWifiManager::update() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!otaStarted) {
      ArduinoOTA.setHostname(OTA_HOSTNAME);
      ArduinoOTA.setPassword(OTA_PASSWORD);
      ArduinoOTA.begin();
      otaStarted = true;
    }
    ArduinoOTA.handle();
  }
}

void OtaWifiManager::saveConfig(String _ssid, String _pass, bool _staEnabled) {
  ssid = _ssid;
  password = _pass;
  staEnabled = _staEnabled;
  prefs.begin("ota_wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  prefs.putBool("staEnabled", staEnabled);
  prefs.end();
}

void OtaWifiManager::getConfig(String &_ssid, String &_pass, bool &_staEnabled) {
  _ssid = ssid;
  _pass = password;
  _staEnabled = staEnabled;
}

bool OtaWifiManager::isStaEnabled() {
  return staEnabled;
}
