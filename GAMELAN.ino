#include "config.h"
#include "pins.h"

#include "button.h"
#include "display.h"
#include "solenoid.h"
#include "sdcard.h"
#include "midi.h"
#include "player.h"
#include "webserver.h"
#include "buzzer.h"
#include "led.h"
#include "ota_wifi.h"

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

  otaWifi.begin();
  if (otaWifi.isStaEnabled()) {
    Serial.println("[MAIN]: STA aktif otomatis dari boot.");
    otaWifi.beginSTA();
  } else {
    // Jika STA tidak aktif, pastikan web server TIDAK berjalan otomatis di sini,
    // atau jika ingin tetap bisa diakses melalui AP, panggil webServer.startAP().
    // Karena tujuan utamanya adalah STA, kita biarkan saja.
  }
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
      if (blinkState) led.setColor(255, 165, 0);
      else led.setColor(0, 0, 0);
    }
  } else if (webServer.isActive()) {
    led.setColor(0, 0, 255);  // Blue
  } else if (player.isPlaying()) {
    // Playing: Green blink
    if (now - lastBlink > 500) {
      lastBlink = now;
      blinkState = !blinkState;
      if (blinkState) led.setColor(0, 255, 0);
      else led.setColor(0, 0, 0);
    }
  } else {
    led.setColor(255, 0, 0);  // Red (Idle)
  }

  uint32_t holdTime = button.getStopHoldDuration();
  static bool wifiActionTaken = false;

  if (holdTime > 0) {
    if (holdTime >= WIFI_DISABLE_MS && !wifiActionTaken) {
      // Keluar dari Dasbor (AP -> STA)
      if (webServer.isActive()) {
        Serial.println("[MAIN]: Menghentikan WebServer...");
        webServer.stop();
        buzzer.wifiOff(); // Buzzer dibunyikan di sini, saat AP mati (Keluar Dasbor)
        
        // Jeda 2 detik untuk memastikan stack WiFi benar-benar bersih
        delay(2000); 
        
        // PENTING: Reload config agar yakin membaca data terbaru dari Flash
        otaWifi.begin(); 
        
        Serial.printf("[MAIN] Transisi AP->STA. STA_ENABLED=%d\n", otaWifi.isStaEnabled());

        if (otaWifi.isStaEnabled()) {
           Serial.println("[MAIN]: STA Aktif, mencoba terhubung...");
           otaWifi.beginSTA();
        } else {
           Serial.println("[MAIN]: WiFi STA dinonaktifkan.");
        }
        wifiActionTaken = true;
      }
    } else if (holdTime >= WIFI_ENABLE_MS && holdTime < WIFI_DISABLE_MS && !wifiActionTaken) {
      // Masuk ke Dasbor (STA -> AP)
      if (!webServer.isActive()) {
        otaWifi.stopSTA(); // Matikan STA dulu
        delay(2000); // Jeda transisi
        webServer.startAP(); // Baru hidupkan AP + Server
        buzzer.wifiOn(); // Kembalikan buzzer
        wifiActionTaken = true;
      }
    }
  } else {
    wifiActionTaken = false;
  }

  webServer.update();
  otaWifi.update();

  player.update();
  // ... rest unchanged

  solenoid.update();
  display.update();
  led.update();
}
