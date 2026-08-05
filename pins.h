#ifndef PINS_H
#define PINS_H

#include "config.h"

//=========================
// LCD
//=========================
#define LCD_SDA 8
#define LCD_SCL 9

// SD CARD SPI
//=========================
#define PIN_SD_CS 14
#define SD_MOSI 13
#define SD_MISO 11
#define SD_SCK 12

//=========================
// BUTTON / BUZZER / LED (PCF8574)
//=========================
#define PIN_PREV 0
#define PIN_PLAY_PAUSE 1
#define PIN_NEXT 2
#define PIN_MODE 3

#define PIN_BUZZER 4
#define PIN_LED_NET 5
#define PIN_LED_RUN 6
#define PIN_LED_ERR 7

#define PIN_SD_DET 10

#endif