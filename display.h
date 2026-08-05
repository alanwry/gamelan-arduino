#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define SCROLL_INTERVAL 300

class DisplayManager {
public:
  void begin();
  void splash();
  void ready();
  void error(const char *msg);
  void showSong(const char *song);
  void showStatus(const char *status);
  void showFileIndex(uint16_t current, uint16_t total);
  void update();
private:
  void scrollDisplay();
  char songName[17];
  char statusName[17];
  uint16_t currentFile;
  uint16_t totalFiles;
  uint16_t scrollPos;
  uint32_t lastScrollTime;
};

extern DisplayManager display;

#endif
