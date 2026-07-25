#include "display.h"
#include "config.h"
#include "pins.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD I2C
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COL, LCD_ROW);
DisplayManager display;

// Inisialisasi LCD dengan I2C
void DisplayManager::begin() {
  Wire.begin(LCD_SDA, LCD_SCL);
  Wire.beginTransmission(LCD_ADDRESS);
  if (Wire.endTransmission() == 0) {
    Serial.println("[DISPLAY]: Terhubung");
  } else {
    Serial.println("[DISPLAY]: Error: Gagal terhubung");
  }
  lcd.init();
  lcd.backlight();
}

// Tampilkan splash screen pada startup
void DisplayManager::splash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AUTO GAMELAN");
  lcd.setCursor(0, 1);
  lcd.print(FW_VERSION);
  delay(1500);
}

// Reset display ke state siap
void DisplayManager::ready() {
  strcpy(songName, "NO FILE");
  strcpy(statusName, "READY");
  currentFile = 0;
  totalFiles = 0;
  scrollPos = 0;
  lastScrollTime = 0;
  lcd.clear();
}

// Tampilkan pesan error
void DisplayManager::error(const char *msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR");
  lcd.setCursor(0, 1);
  char errMsg[17];
  strncpy(errMsg, msg, 16);
  errMsg[16] = 0;
  lcd.print(errMsg);
}

// Set nama file/lagu (trigger reset scrolling)
void DisplayManager::showSong(const char *song) {
  strncpy(songName, song, 16);
  songName[16] = 0;
  scrollPos = 0;
  lastScrollTime = millis();
}

// Set status playback (READY, PLAYING, PAUSED, etc)
void DisplayManager::showStatus(const char *status) {
  strncpy(statusName, status, 16);
  statusName[16] = 0;
}

// Set file index untuk menampilkan [x/y]
void DisplayManager::showFileIndex(uint16_t current, uint16_t total) {
  currentFile = current;
  totalFiles = total;
}

// Kontrol scrolling untuk nama file panjang
void DisplayManager::scrollDisplay() {
  if (strlen(songName) <= LCD_COL)
    return;

  uint32_t now = millis();
  if (now - lastScrollTime < SCROLL_INTERVAL)
    return;

  lastScrollTime = now;
  scrollPos++;
  if (scrollPos > strlen(songName) + 3)
    scrollPos = 0;
}

// Update display setiap 250ms (dipanggil di loop utama)
void DisplayManager::update() {
  static uint32_t last = 0;
  if (millis() - last < 250)
    return;
  last = millis();

  scrollDisplay();

  char displayName[17];
  uint16_t nameLen = strlen(songName);

  // Format nama file dengan index [current/total]
  if (totalFiles > 0) {
    if (nameLen > 10) {
      // Nama panjang: aktifkan scrolling
      if (scrollPos < nameLen - 8) {
        strncpy(displayName, songName + scrollPos, 13);
        displayName[13] = 0;
      } else {
        strcpy(displayName, songName);
      }
    } else {
      // Nama pendek: tampilkan dengan index
      snprintf(displayName, 17, "%-11s[%u/%u]", songName, currentFile + 1, totalFiles);
    }
  } else {
    // Tidak ada file: tampilkan nama saja
    strcpy(displayName, songName);
  }

  // Write ke LCD (baris 1 = nama, baris 2 = status)
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(displayName);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(statusName);
}