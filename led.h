#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>

class LedController {
public:
  void begin(uint8_t pin);
  void update();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
private:
  Adafruit_NeoPixel* strip;
  uint8_t _pin;
  uint32_t currentColor = 0;
};

extern LedController led;
#endif
