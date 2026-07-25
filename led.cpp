#include "led.h"
#include "player.h"
#include "webserver.h"

LedController led;

void LedController::begin(uint8_t pin) {
  _pin = pin;
  strip = new Adafruit_NeoPixel(1, _pin, NEO_GRB + NEO_KHZ800);
  strip->begin();
  strip->setBrightness(50);
  setColor(255, 0, 0);  // Default Red
}

void LedController::setColor(uint8_t r, uint8_t g, uint8_t b) {
  currentColor = strip->Color(r, g, b);
  strip->setPixelColor(0, currentColor);
  strip->show();
}

void LedController::update() {
  // Logic moved to loop() to handle blinking patterns
}
