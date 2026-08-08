#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class BuzzerManager {
public:
  void begin(uint8_t pin);
  void update();
  
  // Fungsi untuk trigger bunyi (non-blocking)
  void beep(uint16_t duration = 50); // General key beep
  void startup();
  void wifiOn();
  void wifiOff();
  void uploadSuccess();
  void wifiSaved();
  
  // Fungsi lama yang mungkin masih dipanggil, kita buat dummy atau map ke beep
  void modeAuto() { beep(); }
  void modeManual() { beep(); }

private:
  uint8_t _pin;
  uint32_t _beepEndTime = 0;
  bool _isBuzzing = false;
  
  // Internal helper untuk mulai buzzing
  void _startBuzz(uint16_t duration);
};

extern BuzzerManager buzzer;

#endif
