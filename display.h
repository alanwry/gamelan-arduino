#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// LCD Display Manager - mengelola tampilan pada LCD 16x2
class DisplayManager {
public:
  void begin();                 // Inisialisasi LCD
  void splash();                // Tampilkan splash screen
  void ready();                 // Reset ke state siap
  void error(const char *msg);  // Tampilkan pesan error

  // Setter untuk konten display
  void showSong(const char *song);
  void showStatus(const char *status);
  void showFileIndex(uint16_t current, uint16_t total);

  void update();  // Update display (di loop utama)

private:
  char songName[17];
  char statusName[17];
  uint16_t currentFile = 0;
  uint16_t totalFiles = 0;

  // Scrolling untuk nama file panjang
  uint32_t scrollPos = 0;
  uint32_t lastScrollTime = 0;
  static const uint16_t SCROLL_INTERVAL = 400;

  void scrollDisplay();
};

extern DisplayManager display;

#endif