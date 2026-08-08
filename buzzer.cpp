#include "buzzer.h"
#include "button.h"
#include "pins.h"

BuzzerManager buzzer;

// Menambahkan state untuk pola agar non-blocking
struct Pattern {
  uint16_t onDuration;
  uint16_t offDuration;
  uint8_t repeats;
  uint8_t currentRepeat;
  bool isSecondBeep;
};
static Pattern activePattern;
static bool patternActive = false;

void BuzzerManager::begin(uint8_t pin) {
  _pin = pin;
  if (button.isInitialized()) {
    pcf.pinMode(_pin, OUTPUT);
  }
  pcf.digitalWrite(_pin, LOW);
  _isBuzzing = false;
  patternActive = false;
}

void BuzzerManager::update() {
  uint32_t now = millis();
  
  if (_isBuzzing) {
    if (now >= _beepEndTime) {
      pcf.digitalWrite(_pin, LOW);
      _isBuzzing = false;
      
      if (patternActive) {
        // Prepare for the second beep (the off phase)
        _beepEndTime = now + activePattern.offDuration;
        // Kita gunakan _isBuzzing sebagai tanda 'tunggu off duration'
        _isBuzzing = true;
      }
    }
  } else if (patternActive) {
    // If not buzzing, but pattern is active, we are in the 'off' phase
    if (now >= _beepEndTime) {
      // Off phase complete, trigger second beep
      pcf.digitalWrite(_pin, HIGH);
      _beepEndTime = now + activePattern.onDuration;
      _isBuzzing = true;
      patternActive = false; // Pattern complete
    }
  }
}

void BuzzerManager::_startBuzz(uint16_t duration) {
  if (!button.isInitialized()) return;
  pcf.digitalWrite(_pin, HIGH);
  _beepEndTime = millis() + duration;
  _isBuzzing = true;
}

void BuzzerManager::beep(uint16_t duration) { _startBuzz(duration); }

void BuzzerManager::startup() { _startBuzz(150); }

void BuzzerManager::wifiOn() {
  _startBuzz(100);
  activePattern = {100, 150, 1, 0, false};
  patternActive = true;
}

void BuzzerManager::wifiOff() {
  _startBuzz(100);
  activePattern = {100, 150, 1, 0, false};
  patternActive = true;
}

void BuzzerManager::wifiSaved() { _startBuzz(300); }

void BuzzerManager::uploadSuccess() { _startBuzz(300); }
