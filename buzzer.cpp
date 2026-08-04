// buzzer.cpp uses extern pcf from button.h
#include "buzzer.h"
#include "button.h"
#include "pins.h"

BuzzerManager buzzer;

void BuzzerManager::begin(uint8_t pin) {
  _pin = pin;
  if (button.isInitialized()) {
    pcf.pinMode(_pin, OUTPUT);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::beep() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(50);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::startup() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
    delay(150);
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::wifiOn() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
    delay(150);
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::wifiOff() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
    delay(150);
    pcf.digitalWrite(_pin, HIGH);
    delay(200);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::uploadSuccess() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
    delay(150);
    pcf.digitalWrite(_pin, HIGH);
    delay(200);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::modeAuto() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
    delay(100);
    pcf.digitalWrite(_pin, HIGH);
    delay(100);
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::modeManual() {
  if (button.isInitialized()) {
    pcf.digitalWrite(_pin, HIGH);
    delay(200);
    pcf.digitalWrite(_pin, LOW);
  }
}
