#include "button.h"
#include "pins.h"
#include "config.h"
#include <Adafruit_PCF8574.h>

ButtonManager button;
Adafruit_PCF8574 pcf;

// Mapping array: P0=PREV, P1=PLAY_PAUSE, P2=NEXT, P3=MODE
static const uint8_t buttonPin[4] = { PIN_PREV, PIN_PLAY_PAUSE, PIN_NEXT, PIN_MODE };

void ButtonManager::begin() {
  // Initializing PCF8574 silently
  if (!pcf.begin(PCF8574_ADDRESS, &Wire)) {
    Serial.println("[SYSTEM]: Error: PCF8574 gagal terhubung!");
    initialized = false;
    return;
  }
  
  Serial.println("[SYSTEM]: PCF8574 terhubung.");
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

// Helper untuk direct buzzer control di PCF
void triggerBuzzer(uint16_t duration) {
  pcf.digitalWrite(PIN_BUZZER, HIGH);
  vTaskDelay(duration / portTICK_PERIOD_MS);
  pcf.digitalWrite(PIN_BUZZER, LOW);
}

void ButtonManager::update() {
  if (!initialized) return;

  event = BTN_NONE;
  uint32_t now = millis();

  for (int i = 0; i < 4; i++) {
    bool state = pcf.digitalRead(buttonPin[i]);

    if (state != lastState[i]) {
      if ((now - lastTime[i]) > BUTTON_DEBOUNCE) {
        lastState[i] = state;
        lastTime[i] = now;

        if (state == LOW) {
          pressedState[i] = true;
          triggerBuzzer(50); // Panggil beep untuk semua tombol

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
        } else {
          pressedState[i] = false;
          if (i == 3) {
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
