#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SD.h>
#include "freertos/semphr.h"

#define MAX_FILES 100
#define MAX_FILENAME 128

class SDCardManager {
public:

  bool begin();

  void update();  // Tambahkan ini

  bool isInserted(); // Add this

  void scan();

  bool next();

  bool prev();

  uint16_t getCount();

  const char* getCurrentFile();

  File openCurrent();

  File openFile(const char* path, const char* mode);
  bool deleteFile(const char* path);

private:

  char filenames[MAX_FILES][MAX_FILENAME];

  uint16_t totalFiles = 0;

  int16_t currentIndex = 0;

  SemaphoreHandle_t mutex;
};

extern SDCardManager sdcard;

#endif