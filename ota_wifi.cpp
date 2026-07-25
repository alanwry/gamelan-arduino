#include "ota_wifi.h"
#include "config.h"

OtaWifiManager otaWifi;

void OtaWifiManager::begin() {
  prefs.begin("ota_wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  staEnabled = prefs.getBool("staEnabled", false);
  prefs.end();
}

void OtaWifiManager::beginSTA() {
  if (ssid.length() == 0 || password.length() == 0) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.printf("[OTA_WIFI]: Menghubungkan ke %s...\n", ssid.c_str());

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
}

void OtaWifiManager::stopSTA() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[OTA_WIFI]: WiFi STA dimatikan.");
}

void OtaWifiManager::update() {
  if (WiFi.status() == WL_CONNECTED) {
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
  Serial.println("[OTA_WIFI]: Konfigurasi disimpan.");
}

void OtaWifiManager::getConfig(String &_ssid, String &_pass, bool &_staEnabled) {
  _ssid = ssid;
  _pass = password;
  _staEnabled = staEnabled;
}

bool OtaWifiManager::isStaEnabled() {
  return staEnabled;
}
