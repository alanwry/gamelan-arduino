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

void BuzzerManager::update() {
  if (!button.isInitialized() || !_isPatternPlaying || _currentPattern == nullptr) return;
  uint32_t now = millis();

  if (_currentStep < _patternLength) {
    if (now - _stepStartTime >= (_isBuzzing ? _currentPattern[_currentStep].onTime : _currentPattern[_currentStep].offTime)) {
      _isBuzzing = !_isBuzzing;
      pcf.digitalWrite(_pin, _isBuzzing ? HIGH : LOW);
      _stepStartTime = now;
      if (!_isBuzzing) _currentStep++;
    }
  } else {
    _isPatternPlaying = false;
    pcf.digitalWrite(_pin, LOW);
  }
}

void BuzzerManager::beep() {
  static const PatternStep p[] = {{50, 0}};
  _currentPattern = p; _patternLength = 1; _currentStep = 0;
  _isPatternPlaying = true; _isBuzzing = true;
  _stepStartTime = millis();
  pcf.digitalWrite(_pin, HIGH);
}

void BuzzerManager::startup() {
  static const PatternStep p[] = {{150, 100}, {150, 0}};
  _currentPattern = p; _patternLength = 2; _currentStep = 0;
  _isPatternPlaying = true; _isBuzzing = true;
  _stepStartTime = millis();
  pcf.digitalWrite(_pin, HIGH);
}

void BuzzerManager::wifiOn() {
  static const PatternStep p[] = {{100, 50}, {200, 0}};
  _currentPattern = p; _patternLength = 2; _currentStep = 0;
  _isPatternPlaying = true; _isBuzzing = true;
  _stepStartTime = millis();
  pcf.digitalWrite(_pin, HIGH);
}

void BuzzerManager::wifiOff() {
  static const PatternStep p[] = {{200, 50}, {100, 0}};
  _currentPattern = p; _patternLength = 2; _currentStep = 0;
  _isPatternPlaying = true; _isBuzzing = true;
  _stepStartTime = millis();
  pcf.digitalWrite(_pin, HIGH);
}

void BuzzerManager::uploadSuccess() { beep(); }
void BuzzerManager::modeAuto() {}
void BuzzerManager::modeManual() {}
