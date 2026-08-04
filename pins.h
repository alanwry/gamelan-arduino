#ifndef PINS_H
#define PINS_H

#include "config.h"

//=========================
// BUZZER
//=========================
#define BUZZER_PIN 41

//=========================
// LCD
//=========================

#define LCD_SDA 8
#define LCD_SCL 9

// SD CARD SPI
//=========================

#define SD_CS 14//10
#define PIN_SD_CS SD_CS
#define SD_MOSI 13//11
#define SD_MISO 11//13
#define SD_SCK 12//12

//=========================
// BUTTON
//=========================
// Using PCF8574 I2C I/O Expander
#define PIN_PLAY_PAUSE 0
#define PIN_STOP 1
#define PIN_PREV 2
#define PIN_NEXT 3
#define PIN_MODE 4

#define PIN_LED 48
#define PIN_SD_DET 10  //14

#endif