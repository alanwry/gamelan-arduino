#include "ota_wifi.h"
#include "config.h"

OtaWifiManager otaWifi;

void OtaWifiManager::begin() {
  prefs.begin("ota_wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  staEnabled = prefs.getBool("staEnabled", false);
  prefs.end();
  Serial.printf("[OTA_WIFI] INIT: SSID='%s', STA_ENABLED=%d\n", ssid.c_str(), staEnabled);
}

void OtaWifiManager::beginSTA() {
  if (ssid.length() == 0 || password.length() == 0) {
    Serial.println("[OTA_WIFI]: SSID/Password kosong, STA tidak dimulai.");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.printf("[OTA_WIFI]: Mencoba menghubungkan ke SSID: %s\n", ssid.c_str());

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
  Serial.println("[OTA_WIFI]: Mode STA diinisialisasi.");
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
  Serial.printf("[OTA_WIFI] SAVED: SSID='%s', STA_ENABLED=%d\n", ssid.c_str(), staEnabled);
}

void OtaWifiManager::getConfig(String &_ssid, String &_pass, bool &_staEnabled) {
  _ssid = ssid;
  _pass = password;
  _staEnabled = staEnabled;
}

bool OtaWifiManager::isStaEnabled() {
  return staEnabled;
}
