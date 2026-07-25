#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum ButtonID {
  BTN_NONE = 0,
  BTN_START,
  BTN_STOP,
  BTN_NEXT,
  BTN_PREV,
  BTN_MODE
};

class ButtonManager {

public:
  void begin();

  void update();

  ButtonID getEvent();

  bool getWifiEnableLongPress();
  bool getWifiDisableLongPress();
  uint32_t getStopHoldDuration();

private:
  ButtonID event = BTN_NONE;

  bool lastState[4];
  bool pressedState[4];

  uint32_t lastTime[4];
  uint32_t lastPressTime[4];
  bool stopLongPressTriggered = false;
  bool stopHeld = false;
  uint32_t stopPressStart = 0;
};

extern ButtonManager button;

#endif