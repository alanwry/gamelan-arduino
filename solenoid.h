#ifndef SOLENOID_H
#define SOLENOID_H

#include <Arduino.h>
#include "config.h"

// Solenoid individual - kontrol satu solenoid
class Solenoid {
public:
  void begin(uint8_t gpio, String note, uint8_t midiNote);
  void hit(uint16_t duration = 20);
  void update();
  void off();
  uint8_t getPin();
  String getNote();
  uint8_t getMidiNote();

private:
  uint8_t pin;
  String note;
  uint8_t midiNote;
  bool active;
  uint64_t offTime;
};

// Solenoid Manager - kontrol solenoid
class SolenoidManager {
public:
  void begin();
  void update();
  void hit(uint8_t id, uint16_t duration = 20);
  void allOff();

  // Dynamic management
  bool loadConfig();
  bool saveConfig();
  void addSolenoid(uint8_t pin, String note, uint8_t midiNote);
  void removeSolenoid(uint8_t pin);
  uint8_t getCount() const;
    Solenoid*
    getItems();

private:
  Solenoid item[MAX_SOLENOID];
  uint8_t count;
};

extern SolenoidManager solenoid;

#endif
