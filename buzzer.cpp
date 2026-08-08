#include "buzzer.h"
#include "button.h"
#include "pins.h"

BuzzerManager buzzer;

void BuzzerManager::begin(uint8_t pin) {
  _pin = pin;
  // Memastikan pcf sudah siap sebelum akses pin
  if (button.isInitialized()) {
    pcf.pinMode(_pin, OUTPUT);
  }
  // Paksa mati saat inisialisasi
  pcf.digitalWrite(_pin, LOW);
  _isBuzzing = false;
}

void BuzzerManager::update() {
  if (!_isBuzzing) return;

  if (millis() >= _beepEndTime) {
    pcf.digitalWrite(_pin, LOW);
    _isBuzzing = false;
  }
}

void BuzzerManager::_startBuzz(uint16_t duration) {
  if (!button.isInitialized()) return;
  
  pcf.digitalWrite(_pin, HIGH);
  _beepEndTime = millis() + duration;
  _isBuzzing = true;
}

void BuzzerManager::beep(uint16_t duration) {
  _startBuzz(duration);
}

void BuzzerManager::startup() {
  _startBuzz(150); // 1 beep agak panjang
}

void BuzzerManager::wifiOn() {
  // 2x beep untuk masuk AP
  _startBuzz(100);
  delay(150);
  _startBuzz(100);
}

void BuzzerManager::wifiOff() {
  // 2x beep untuk keluar AP
  _startBuzz(100);
  delay(150);
  _startBuzz(100);
}

void BuzzerManager::wifiSaved() {
  _startBuzz(300); // 1 beep panjang
}

void BuzzerManager::uploadSuccess() {
  _startBuzz(300); // 1 beep panjang
}
