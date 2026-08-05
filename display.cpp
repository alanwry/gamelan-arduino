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
    Serial.println("[DISPLAY]: Terhubung");
    lcd.init();
    lcd.backlight();
  } else {
    Serial.println("[DISPLAY]: Error: Gagal terhubung");
  }
}

void DisplayManager::splash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AUTO GAMELAN");
  lcd.setCursor(0, 1);
  lcd.print(FW_VERSION);
  delay(1500);
}

void DisplayManager::ready() {
  strcpy(songName, "NO FILE");
  strcpy(statusName, "READY");
  currentFile = 0;
  totalFiles = 0;
  scrollPos = 0;
  lastScrollTime = 0;
  lcd.clear();
}

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

  char displayName[17];
  uint16_t nameLen = strlen(songName);

  if (totalFiles > 0) {
    if (nameLen > 10) {
      // Long name: enable scrolling
      if (scrollPos < nameLen - 8) {
        strncpy(displayName, songName + scrollPos, 13);
        displayName[13] = 0;
      } else {
        strcpy(displayName, songName);
      }
    } else {
      // Short name: show with index
      snprintf(displayName, 17, "%-11s[%u/%u]", songName, currentFile + 1, totalFiles);
    }
  } else {
    strcpy(displayName, songName);
  }

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(displayName);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(statusName);
}