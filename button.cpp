#include "button.h"
#include "pins.h"
#include "config.h"
#include "buzzer.h"
#include <Adafruit_PCF8574.h>

ButtonManager button;
Adafruit_PCF8574 pcf;

// Mapping array: P0=PREV, P1=PLAY_PAUSE, P2=NEXT, P3=MODE
static const uint8_t buttonPin[4] = { PIN_PREV, PIN_PLAY_PAUSE, PIN_NEXT, PIN_MODE };

void ButtonManager::begin() {
  Serial.println("[BUTTON]: Initializing PCF8574...");
  if (!pcf.begin(PCF8574_ADDRESS, &Wire)) {
    Serial.println("[BUTTON]: Error: PCF8574 gagal terhubung!");
    initialized = false;
    return;
  }
  
  Serial.println("[BUTTON]: PCF8574 terhubung.");
  initialized = true;

  for (int i = 0; i < 4; i++) {
    pcf.pinMode(buttonPin[i], INPUT_PULLUP);
    lastState[i] = pcf.digitalRead(buttonPin[i]);
    pressedState[i] = false;
    lastTime[i] = 0;
  }
}

bool ButtonManager::isInitialized() {
  return initialized;
}

uint32_t ButtonManager::getStopHoldDuration() {
  if (stopHeld) return millis() - stopPressStart;
  return 0;
}

void ButtonManager::update() {
  if (!initialized) return;

  event = BTN_NONE;
  uint32_t now = millis();

  for (int i = 0; i < 4; i++) {
    bool state = pcf.digitalRead(buttonPin[i]);

    // Debounce logic
    if (state != lastState[i]) {
      if ((now - lastTime[i]) > BUTTON_DEBOUNCE) {
        lastState[i] = state;
        lastTime[i] = now;

        if (state == LOW) {  // Button Pressed
          pressedState[i] = true;
          // MODE button is index 3 (last one), so check i != 3
          if (i != 3) buzzer.beep();

          switch (i) {
            case 0:
              event = BTN_PREV;
              Serial.println("[BUTTON]: PREVIOUS ditekan");
              break;
            case 1:
              event = BTN_START;
              Serial.println("[BUTTON]: PLAY/PAUSE ditekan");
              break;
            case 2:
              event = BTN_NEXT;
              Serial.println("[BUTTON]: NEXT ditekan");
              break;
            case 3:
              // For MODE button, handle long press for AP mode entrance
              stopHeld = true;
              stopPressStart = now;
              Serial.printf("[BUTTON]: MODE hold started at %lu\n", stopPressStart);
              break;
          }
        } else {  // Button Released
          pressedState[i] = false;
          if (i == 3) {  // MODE button logic
            Serial.printf("[BUTTON]: MODE released. Duration: %lu ms\n", now - stopPressStart);
            if (now - stopPressStart < WIFI_ENABLE_MS) {
              event = BTN_MODE;
              Serial.println("[BUTTON]: MODE (short press) event generated");
            }
            stopHeld = false;
            stopPressStart = 0;
          }
        }
      }
    }
  }
}

ButtonID ButtonManager::getEvent() {
  ButtonID e = event;
  event = BTN_NONE;
  return e;
}

bool ButtonManager::getWifiEnableLongPress() {
  return false;
}
bool ButtonManager::getWifiDisableLongPress() {
  return false;
}
