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

  buzzer.begin(BUZZER_PIN);
  buzzer.startup();

  display.begin();
  display.splash();

  button.begin();
  wifiManager.begin();

  Serial.printf("[SYSTEM]: STA Enabled status: %s\n", wifiManager.isSTAEnabled() ? "ON" : "OFF");
  if (wifiManager.isSTAEnabled()) {
    wifiManager.startSTA();
  }
  if (!sdcard.begin()) {
    display.error("SD CARD");
    while (true) {
      if (digitalRead(PIN_SD_DET) == LOW) {
        if (sdcard.begin()) {
          Serial.println("[SDCARD]: Kartu SD berhasil dibaca, melanjutkan...");
          break;
        }
      }
      delay(500);
    }
  }

  solenoid.begin();

  led.begin(PIN_LED);

  sdcard.scan();

  player.begin();

  display.ready();
}
void loop() {

  button.update();
  sdcard.update();

  static uint32_t lastBlink = 0;
  static bool blinkState = false;
  uint32_t now = millis();

  if (digitalRead(PIN_SD_DET) == HIGH) {
    // SD Lepas: Orange blink (Orange = R:255, G:165, B:0)
    if (now - lastBlink > 500) {
      lastBlink = now;
      blinkState = !blinkState;
      if (blinkState)
        led.setColor(255, 165, 0);
      else
        led.setColor(0, 0, 0);
    }
  } else if (webServer.isActive()) {
    led.setColor(0, 0, 255); // Blue
  } else if (player.isPlaying()) {
    // Playing: Green blink
    if (now - lastBlink > 500) {
      lastBlink = now;
      blinkState = !blinkState;
      if (blinkState)
        led.setColor(0, 255, 0);
      else
        led.setColor(0, 0, 0);
    }
  } else {
    led.setColor(255, 0, 0); // Red (Idle)
  }

  static bool lastSTAEstate = false;
  static bool firstRun = true;
  bool currentSTAEstate = wifiManager.isSTAEnabled();

  // Jika status toggle STA berubah secara real-time via dashboard (atau saat boot)
  if (currentSTAEstate != lastSTAEstate || firstRun) {
    if (!firstRun) Serial.printf("[SYSTEM]: STA Toggle changed in real-time to: %s\n", currentSTAEstate ? "ON" : "OFF");
    lastSTAEstate = currentSTAEstate;
    firstRun = false;
    
    // Hanya apply jika TIDAK sedang dalam mode AP
    if (WiFi.getMode() != WIFI_AP) {
        if (currentSTAEstate) {
          wifiManager.startSTA();
          if (WiFi.status() == WL_CONNECTED) {
             webServer.begin();
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
            
            // Setelah AP mati, jika STA toggle ON, maka coba konek STA
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
            wifiManager.stopAll(); // Matikan STA jika ada
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
  // ... rest unchanged

  solenoid.update();
  display.update();
  led.update();
}