#include "led.h"
#include "button.h" // Contains 'extern Adafruit_PCF8574 pcf;'
#include "pins.h"
#include "player.h"
#include "webserver.h"
#include <WiFi.h>

LedController led;

void LedController::begin() {
  if (!button.isInitialized()) return;
  
  pcf.pinMode(PIN_LED_NET, OUTPUT);
  pcf.pinMode(PIN_LED_RUN, OUTPUT);
  pcf.pinMode(PIN_LED_ERR, OUTPUT);
  
  // Set all LEDs to OFF initially
  pcf.digitalWrite(PIN_LED_NET, LOW);
  pcf.digitalWrite(PIN_LED_RUN, LOW);
  pcf.digitalWrite(PIN_LED_ERR, LOW);
}

void LedController::update() {
  if (!button.isInitialized()) return;

  // P5 (Net): ON if AP mode active or STA connected
  bool netOn = (WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED);
  pcf.digitalWrite(PIN_LED_NET, netOn ? HIGH : LOW);

  // P6 (Run): ON constant when Pause; Blinking when Play.
  if (player.isPlaying()) {
    static uint32_t lastPlayBlink = 0;
    static bool playBlinkState = false;
    uint32_t now = millis();
    if (now - lastPlayBlink >= 500) {
      lastPlayBlink = now;
      playBlinkState = !playBlinkState;
    }
    pcf.digitalWrite(PIN_LED_RUN, playBlinkState ? HIGH : LOW);
  } else if (player.isPaused()) {
    pcf.digitalWrite(PIN_LED_RUN, HIGH);
  } else {
    pcf.digitalWrite(PIN_LED_RUN, LOW);
  }

  // P7 (Err): ON if there is an error
  bool errorState = (digitalRead(PIN_SD_DET) == HIGH); 
  pcf.digitalWrite(PIN_LED_ERR, errorState ? HIGH : LOW);
}
