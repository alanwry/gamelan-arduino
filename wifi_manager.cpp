#include "wifi_manager.h"
#include "config.h"
#include "display.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include "esp_wifi.h"

WiFiManager wifiManager;

WiFiManager::WiFiManager() : ssid(""), password(""), enableSTA(false) {}

void WiFiManager::begin() { 
  loadFromPrefs(); 
}

void WiFiManager::loadFromPrefs() {
  // Try read-only first
  if (prefs.begin("gamelan_wifi", true)) {
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("password", "");
    enableSTA = prefs.getBool("enableSTA", false);
    prefs.end();
    Serial.printf("[WIFI]: Loaded settings - SSID: '%s', STA Enabled: %s\n", ssid.c_str(), enableSTA ? "ON" : "OFF");
  } else {
    // If failed, it might not exist. Try opening in write mode to initialize it.
    Serial.println("[WIFI]: Namespace not found, initializing...");
    if (prefs.begin("gamelan_wifi", false)) {
        ssid = "";
        password = "";
        enableSTA = false;
        prefs.end();
        Serial.println("[WIFI]: Preferences initialized successfully.");
    } else {
        Serial.println("[WIFI]: ERROR: Preferences hardware failure!");
    }
  }
}

void WiFiManager::saveSettings(String newSsid, String newPassword, bool newEnableSTA) {
  ssid = newSsid;
  password = newPassword;
  enableSTA = newEnableSTA;

  Serial.printf("[WIFI]: Saving to Preferences - SSID: '%s', Enable: %s\n", ssid.c_str(), enableSTA ? "ON" : "OFF");
  
  if (prefs.begin("gamelan_wifi", false)) {
    size_t sLen = prefs.putString("ssid", ssid);
    size_t pLen = prefs.putString("password", password);
    size_t eLen = prefs.putBool("enableSTA", enableSTA);
    prefs.end();
    Serial.printf("[WIFI]: Save complete (Wrote SSID: %d, Pass: %d, Enable: %d)\n", sLen, pLen, eLen);
  } else {
    Serial.println("[WIFI]: Error: Failed to open Preferences for writing");
  }
}

void WiFiManager::getSettings(String &outSsid, String &outPassword, bool &outEnableSTA) {
  outSsid = ssid;
  outPassword = password;
  outEnableSTA = enableSTA;
}

bool WiFiManager::isSTAEnabled() { 
  return enableSTA; 
}
void WiFiManager::stopAll() {
  Serial.println("[WIFI]: Cleaning up WiFi stack...");
  
  // Gentler disconnect to avoid netstack cb registration errors
  WiFi.disconnect(false, false); 
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_OFF);

  delay(200); 
  Serial.println("[WIFI]: WiFi stack cleaned.");
}

void WiFiManager::startAP() {
  Serial.println("[WIFI]: Attempting to start AP Mode...");
  stopAll();
  delay(100);

  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  if (WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
      Serial.println("[WIFI]: AP Mode Started successfully");
      Serial.print("[WIFI]: AP IP Address: ");
      Serial.println(WiFi.softAPIP());
      display.showStatus("AP: 192.168.4.1");
  } else {
      Serial.println("[WIFI]: AP Mode Failed to start");
      display.showStatus("AP START FAIL");
  }
}

void WiFiManager::startSTA() {
  if (ssid.length() == 0) {
      Serial.println("[WIFI]: No SSID configured, cannot start STA");
      return;
  }

  Serial.printf("[WIFI]: Attempting to start STA. SSID: %s\n", ssid.c_str());
  stopAll();
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int timeout = 0;
  Serial.print("[WIFI]: Connecting to STA");
  display.showStatus("CONNECTING...");
  while (WiFi.status() != WL_CONNECTED && timeout < 30) { // 15 seconds timeout
    delay(500);
    timeout++;
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WIFI]: STA Mode Connected");
    Serial.print("[WIFI]: IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Display IP on LCD Screen
    char statusBuf[17];
    snprintf(statusBuf, sizeof(statusBuf), "IP: %s", WiFi.localIP().toString().c_str());
    display.showStatus(statusBuf);
  } else {
    Serial.println("[WIFI]: STA Mode Failed to connect");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    display.showStatus("WIFI CONN FAIL");
  }
}

