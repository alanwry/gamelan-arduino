#include "display.h"
#include "config.h"
#include "pins.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COL, LCD_ROW);
DisplayManager display;

void DisplayManager::begin() {
  Wire.begin(LCD_SDA, LCD_SCL);
  Wire.beginTransmission(LCD_ADDRESS);
  if (Wire.endTransmission() == 0) {
    Serial.println("[SYSTEM]: Display terhubung");
    lcd.init();
    lcd.backlight();
    initialized = true;
  } else {
    Serial.println("[SYSTEM]: Error: Display gagal terhubung");
    initialized = false;
  }
}

bool DisplayManager::isInitialized() {
  return initialized;
}

void DisplayManager::splash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAMELAN NUNGGAL");
  lcd.setCursor(0, 1);
  lcd.print("Loading...");
}

void DisplayManager::ready() {
  strcpy(songName, "NO FILE");
  strcpy(statusName, "STOP");
  currentFile = 0;
  totalFiles = 0;
  scrollPos = 0;
  lastScrollTime = 0;
  lcd.clear();
}

void DisplayManager::showSong(const char *song) {
  strncpy(songName, song, 16);
  songName[16] = 0;
  scrollPos = 0;
  lastScrollTime = millis();
}

void DisplayManager::showStatus(const char *status) {
  strncpy(statusName, status, 16);
  statusName[16] = 0;
}

void DisplayManager::showFileIndex(uint16_t current, uint16_t total) {
  currentFile = current;
  totalFiles = total;
}

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

// Update display every 250ms
void DisplayManager::update() {
  static uint32_t last = 0;
  if (millis() - last < 250)
    return;
  last = millis();

  scrollDisplay();
  
  // Format judul agar scroll 16 karakter
  char displaySong[17];
  int len = strlen(songName);
  if (len <= 16) {
      strcpy(displaySong, songName);
  } else {
      int start = scrollPos % (len + 1);
      int end = (start + 16 > len) ? len : start + 16;
      strncpy(displaySong, songName + start, end - start);
      displaySong[end-start] = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(statusName);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(displaySong);
}