#include "button.h"
#include "pins.h"
#include "config.h"
#include "buzzer.h"
#include <Adafruit_PCF8574.h>

ButtonManager button;
Adafruit_PCF8574 pcf;

// Mapping array: P0=PLAY_PAUSE, P1=STOP, P2=PREV, P3=NEXT, P4=MODE
static const uint8_t buttonPin[5] = { PIN_PLAY_PAUSE, PIN_STOP, PIN_PREV, PIN_NEXT, PIN_MODE };

void ButtonManager::begin() {
  if (!pcf.begin(PCF8574_ADDRESS, &Wire)) {
    Serial.println("[BUTTON]: Error: PCF8574 gagal terhubung!");
  } else {
    Serial.println("[BUTTON]: PCF8574 terhubung.");
  }
  for (int i = 0; i < 5; i++) {
    pcf.pinMode(buttonPin[i], INPUT_PULLUP);
    lastState[i] = pcf.digitalRead(buttonPin[i]);
    pressedState[i] = false;
    lastTime[i] = 0;
  }
}

uint32_t ButtonManager::getStopHoldDuration() {
  if (stopHeld) return millis() - stopPressStart;
  return 0;
}

void ButtonManager::update() {
  event = BTN_NONE;
  uint32_t now = millis();

  for (int i = 0; i < 5; i++) {
    bool state = pcf.digitalRead(buttonPin[i]);

    // Debounce logic
    if (state != lastState[i]) {
      if ((now - lastTime[i]) > BUTTON_DEBOUNCE) {
        lastState[i] = state;
        lastTime[i] = now;

        if (state == LOW) {  // Button Pressed
          pressedState[i] = true;
          if (i != 4) buzzer.beep();  // Beep if not MODE button

          switch (i) {
            case 0:
              event = BTN_START;
              Serial.println("[BUTTON]: PLAY/PAUSE ditekan");
              break;
            case 1:
              stopHeld = true;
              stopPressStart = now;
              break;
            case 2:
              event = BTN_PREV;
              Serial.println("[BUTTON]: PREVIOUS ditekan");
              break;
            case 3:
              event = BTN_NEXT;
              Serial.println("[BUTTON]: NEXT ditekan");
              break;
            case 4:
              event = BTN_MODE;
              Serial.println("[BUTTON]: MODE ditekan");
              break;
          }
        } else {  // Button Released
          pressedState[i] = false;
          if (i == 1) {  // STOP button logic
            if (now - stopPressStart < WIFI_ENABLE_MS) {
              event = BTN_STOP;
              Serial.println("[BUTTON]: STOP ditekan");
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
