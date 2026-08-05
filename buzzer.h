#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class BuzzerManager {
public:
  void begin(uint8_t pin);
  void update();
  void beep();
  void startup();
  void wifiOn();
  void wifiOff();
  void uploadSuccess();
  void modeAuto();
  void modeManual();
private:
  uint8_t _pin;
  uint32_t _startTime = 0;
  uint32_t _duration = 0;
  bool _isBuzzing = false;
  
  // State machine untuk pola bunyi
  struct PatternStep { uint32_t onTime; uint32_t offTime; };
  const PatternStep* _currentPattern = nullptr;
  int _patternLength = 0;
  int _currentStep = 0;
  uint32_t _stepStartTime = 0;
  bool _isPatternPlaying = false;
};

extern BuzzerManager buzzer;

#endif
