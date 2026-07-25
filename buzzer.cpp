#include "buzzer.h"

BuzzerManager buzzer;

void BuzzerManager::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void BuzzerManager::beep() {
  tone(_pin, 2000, 50);
}

void BuzzerManager::startup() {
  tone(_pin, 1000, 100);
  delay(150);
  tone(_pin, 1500, 100);
}

void BuzzerManager::wifiOn() {
  tone(_pin, 2000, 100);
  delay(150);
  tone(_pin, 2000, 100);
}

void BuzzerManager::wifiOff() {
  tone(_pin, 1000, 100);
  delay(150);
  tone(_pin, 500, 200);
}

void BuzzerManager::uploadSuccess() {
  tone(_pin, 1500, 100);
  delay(150);
  tone(_pin, 2500, 200);
}

void BuzzerManager::modeAuto() {
  tone(_pin, 2500, 100);
  delay(100);
  tone(_pin, 2500, 100);
}

void BuzzerManager::modeManual() {
  tone(_pin, 1000, 200);
}
