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
  if (ssid.length() == 0 || password.length() == 0) {
    Serial.println("[OTA_WIFI]: SSID/Password kosong, STA tidak dimulai.");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.printf("[OTA_WIFI]: Menghubungkan ke %s...\n", ssid.c_str());
  otaStarted = false; 
}

void OtaWifiManager::stopSTA() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  otaStarted = false;
  Serial.println("[OTA_WIFI]: WiFi STA dimatikan.");
}

void OtaWifiManager::update() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!otaStarted) {
      ArduinoOTA.setHostname(OTA_HOSTNAME);
      ArduinoOTA.setPassword(OTA_PASSWORD);
      ArduinoOTA.begin();
      otaStarted = true;
      Serial.print("[OTA_WIFI]: Terhubung! IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("[OTA_WIFI]: ArduinoOTA siap.");
    }
    ArduinoOTA.handle();
  } else {
    if (otaStarted) {
      Serial.println("[OTA_WIFI]: Koneksi STA terputus.");
      otaStarted = false;
    }
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
  
  // Real-time Control: Hanya aktifkan/nonaktifkan jika TIDAK sedang dalam mode AP
  // Karena saat AP, WiFi mode diatur oleh WebServerManager
  if (WiFi.getMode() != WIFI_AP) {
      if (staEnabled) {
          Serial.println("[OTA_WIFI]: Real-time activation...");
          beginSTA();
      } else {
          Serial.println("[OTA_WIFI]: Real-time deactivation...");
          stopSTA();
      }
  }
}

void OtaWifiManager::getConfig(String &_ssid, String &_pass, bool &_staEnabled) {
  _ssid = ssid;
  _pass = password;
  _staEnabled = staEnabled;
}

bool OtaWifiManager::isStaEnabled() {
  return staEnabled;
}
