#include "config.h"
#include "pins.h"

#include "button.h"
#include "buzzer.h"
#include "display.h"
#include "led.h"
#include "midi.h"
#include "player.h"
#include "sdcard.h"
#include "solenoid.h"
#include "webserver.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Queue untuk event tombol
QueueHandle_t buttonQueue;
SemaphoreHandle_t wifiSemaphore;

void midiTask(void *pvParameters) {
  for (;;) {
    ButtonID evt;
    if (xQueueReceive(buttonQueue, &evt, 0) == pdPASS) {
      player.handleEvent(evt);
    }
    player.update();
    vTaskDelay(1 / portTICK_PERIOD_MS); // Yield to other tasks
  }
}

void systemTask(void *pvParameters) {
  for (;;) {
    buzzer.update();
    button.update();

    // Logic for WiFi toggle (AP/STA) based on button hold
    uint32_t holdTime = button.getStopHoldDuration();
    static bool wifiActionTaken = false;
    if (holdTime > 0) {
        if (!wifiActionTaken) {
            if (xSemaphoreTake(wifiSemaphore, pdMS_TO_TICKS(100))) {
                if (holdTime >= WIFI_DISABLE_MS) { // 5s
                    if (WiFi.getMode() == WIFI_AP) {
                        wifiManager.stopAll();
                        buzzer.wifiOff();
                        if (wifiManager.isSTAEnabled()) wifiManager.startSTA();
                        wifiActionTaken = true;
                    }
                } else if (holdTime >= WIFI_ENABLE_MS) { // 2s
                    if (WiFi.getMode() != WIFI_AP) {
                        wifiManager.stopAll();
                        wifiManager.startAP();
                        buzzer.wifiOn();
                        wifiActionTaken = true;
                    }
                }
                xSemaphoreGive(wifiSemaphore);
            }
        }
    } else { wifiActionTaken = false; }

    ButtonID evt = button.getEvent();
    if (evt != BTN_NONE) {
      xQueueSend(buttonQueue, &evt, 0);
    }

    webServer.update();
    solenoid.update();
    display.update();
    led.update();

    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("===================================");
  Serial.println("GAMELAN SASAK");
  Serial.println(FW_VERSION);
  Serial.println("===================================");

  Wire.begin(LCD_SDA, LCD_SCL);
  button.begin(); 
  buzzer.begin(PIN_BUZZER);
  display.begin();
  display.splash();
  wifiManager.begin();
  
  wifiSemaphore = xSemaphoreCreateMutex();
  if (wifiManager.isSTAEnabled()) wifiManager.startSTA();
  
  led.begin();
  sdcard.begin();
  solenoid.begin();
  sdcard.scan();
  player.begin();
  display.ready();
  
  buzzer.startup(); // Start beep AFTER hardware is ready

  buttonQueue = xQueueCreate(10, sizeof(ButtonID));
  
  xTaskCreatePinnedToCore(midiTask, "midiTask", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(systemTask, "systemTask", 8192, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelete(NULL); // Hapus loop utama
}
