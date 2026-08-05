#include "config.h"
#include "pins.h"

#include "button.h"
#include "buzzer.h"
#include "display.h"
#include "led.h"
#include "midi.h"
#include "player.h"
#include "sdcard.h"
#include "solenoid.h"
#include "webserver.h"
#include "wifi_manager.h"
#include <WiFi.h>

#include <Wire.h>

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("===================================");
  Serial.println("GAMELAN SASAK");
  Serial.println(FW_VERSION);
  Serial.println("===================================");

  Wire.begin(LCD_SDA, LCD_SCL);

  button.begin(); // Initialize PCF after button.begin()
  
  buzzer.begin(PIN_BUZZER);
  buzzer.startup();

  display.begin();
  display.splash();

  wifiManager.begin();

  Serial.printf("[SYSTEM]: STA Enabled status: %s\n", wifiManager.isSTAEnabled() ? "ON" : "OFF");
  if (wifiManager.isSTAEnabled()) {
    wifiManager.startSTA();
  }
  
  // LED depends on PCF8574, initialized after button.begin()
  led.begin();

  if (!sdcard.begin()) {
    display.error("SD CARD");
  }

  solenoid.begin();

  sdcard.scan();

  player.begin();

  display.ready();
}

void loop() {
  button.update();
  sdcard.update();

  static bool firstRun = true;
  bool currentSTAEstate = wifiManager.isSTAEnabled();
  static bool lastSTAEstate = currentSTAEstate;

  // Jika status toggle STA berubah secara real-time via dashboard (atau saat boot)
  if (currentSTAEstate != lastSTAEstate || firstRun) {
    if (!firstRun) Serial.printf("[SYSTEM]: STA Toggle changed in real-time to: %s\n", currentSTAEstate ? "ON" : "OFF");
    lastSTAEstate = currentSTAEstate;
    firstRun = false;

    // Hanya apply jika TIDAK sedang dalam mode AP
    if (WiFi.getMode() != WIFI_AP) {
        if (currentSTAEstate) {
          // Hanya start jika belum konek
          if (WiFi.status() != WL_CONNECTED) {
            wifiManager.startSTA();
            if (WiFi.status() == WL_CONNECTED) {
               webServer.begin();
            }
          }
        } else {
          wifiManager.stopAll();
          webServer.stop();
        }
    }
  }

  uint32_t holdTime = button.getStopHoldDuration();
  static bool wifiActionTaken = false;

  if (holdTime > 0) {
    if (!wifiActionTaken) {
      if (holdTime >= WIFI_DISABLE_MS) {
        // Threshold 5s reached: DISABLE AP and check STA Toggle
        if (WiFi.getMode() == WIFI_AP) {
            Serial.println("[SYSTEM]: Disable WiFi AP triggered (5s)");
            webServer.stop();
            wifiManager.stopAll();
            buzzer.wifiOff();
            delay(500);
            
            // If STA toggle ON, connect STA
            if (wifiManager.isSTAEnabled()) {
                wifiManager.startSTA();
                if (WiFi.status() == WL_CONNECTED) {
                    webServer.begin();
                }
            }
            wifiActionTaken = true; 
        }
      } else if (holdTime >= WIFI_ENABLE_MS) {
        // Threshold 2s reached: ENABLE AP (Force AP, disable STA)
        if (WiFi.getMode() != WIFI_AP) {
            Serial.println("[SYSTEM]: Enable WiFi AP triggered (2s)");
            webServer.stop();
            wifiManager.stopAll(); // Disable STA if active
            delay(500);
            
            wifiManager.startAP();
            webServer.begin();
            buzzer.wifiOn();
            wifiActionTaken = true; 
        }
      }
    }
  } else {
    wifiActionTaken = false;
  }

  webServer.update();

  player.update();
  
  solenoid.update();
  display.update();
  led.update();
  buzzer.update();
}
