#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class BuzzerManager {
public:
  void begin(uint8_t pin);
  void beep();
  void startup();
  void wifiOn();
  void wifiOff();
  void uploadSuccess();
  void modeAuto();    // Tambahkan
  void modeManual();  // Tambahkan
private:
  uint8_t _pin;
};

extern BuzzerManager buzzer;

#endif
