#include "player.h"

#include "config.h"
#include "display.h"
#include "event_queue.h"
#include "midi.h"
#include "sdcard.h"
#include "solenoid.h"
#include "esp_timer.h"
#include "buzzer.h"
#include <Preferences.h>

Player player;
Preferences prefs;

void Player::begin() {
  prefs.begin("gamelan", false);
  autoMode = prefs.getBool("autoMode", false);
  solenoidTime = prefs.getUShort("solenoidTime", 20);
  prefs.end();

  loaded = false;
  playing = false;
  paused = false;
  startUS = 0;
  elapsedUS = 0;
  eventQueue.clear();

  Serial.printf("[PLAYER]: Mode inisialisasi: %s\n", autoMode ? "OTOMATIS (LOOP)" : "MANUAL");
  Serial.printf("[PLAYER]: Solenoid Time inisialisasi: %d ms\n", solenoidTime);

  if (sdcard.getCount() > 0) {
    load();
  } else {
    display.showSong("NO FILE");
    display.showStatus("NO MIDI");
  }
}

uint16_t Player::getSolenoidTime() {
  return solenoidTime;
}

void Player::setSolenoidTime(uint16_t time) {
  solenoidTime = time;
  prefs.begin("gamelan", false);
  prefs.putUShort("solenoidTime", time);
  prefs.end();
  Serial.printf("[PLAYER]: Solenoid Time disimpan: %d ms\n", solenoidTime);
}

bool Player::load() {
  if (sdcard.getCount() == 0) {
    loaded = false;
    eventQueue.clear();
    return false;
  }

  File file = sdcard.openCurrent();
  if (!file) {
    loaded = false;
    display.showStatus("LOAD FAIL");
    return false;
  }

  if (!midi.open(file)) {
    loaded = false;
    display.showStatus("LOAD FAIL");
    return false;
  }

  eventQueue.clear();
  bool ok = midi.parse();
  midi.close();
  loaded = ok;

  if (ok) {
    display.showSong(sdcard.getCurrentFile());
    display.showStatus("LOADED");
    Serial.printf("[PLAYER]: File dimuat: %s\n", sdcard.getCurrentFile());
  } else {
    display.showStatus("LOAD FAIL");
  }

  return ok;
}

void Player::play() {
  if (sdcard.getCount() == 0) {
    display.showStatus("NO MIDI");
    return;
  }
  if (!loaded && !load()) return;

  Serial.printf("[PLAYER]: %s %s\n", paused ? "Melanjutkan" : "Mulai memainkan", sdcard.getCurrentFile());

  playing = true;
  if (paused) {
    // Resume: sesuaikan startUS agar seolah-olah musik berjalan dari posisi terakhir
    startUS = esp_timer_get_time() - elapsedUS;
    paused = false;
  } else {
    // Restart/Fresh start
    elapsedUS = 0;
    startUS = esp_timer_get_time();
    paused = false;
  }

  display.showStatus("PLAYING");
}

void Player::stop() {
  if (!playing && !paused) return;

  playing = false;
  paused = false;
  loaded = false;
  elapsedUS = 0;  // Reset posisi
  solenoid.allOff();
  eventQueue.clear();
  display.showStatus("STOPPED");
  Serial.println("[PLAYER]: Berhenti memainkan");
}

void Player::pause() {
  if (!playing) return;

  // Simpan posisi saat ini
  elapsedUS += (esp_timer_get_time() - startUS);

  playing = false;
  paused = true;
  solenoid.allOff();
  display.showStatus("PAUSED");
}

bool Player::isPlaying() {
  return playing;
}

void Player::nextFile() {
  stop();
  sdcard.next();
  load();
}

void Player::prevFile() {
  stop();
  sdcard.prev();
  load();
}

void Player::toggleMode() {
  autoMode = !autoMode;

  prefs.begin("gamelan", false);
  prefs.putBool("autoMode", autoMode);
  prefs.end();

  Serial.printf("[PLAYER]: Mode diubah ke: %s\n", autoMode ? "OTOMATIS (LOOP)" : "MANUAL");
  if (autoMode) buzzer.modeAuto();
  else buzzer.modeManual();
  display.showStatus(autoMode ? "AUTO MODE" : "MANUAL MODE");
}

bool Player::isAutoMode() {
  return autoMode;
}

void Player::update() {
  ButtonID evt = button.getEvent();

  switch (evt) {
    case BTN_START:
      if (playing) pause();
      else if (paused) play();
      else play();
      break;
    case BTN_STOP:
      stop();
      break;
    case BTN_NEXT:
      nextFile();
      break;
    case BTN_PREV:
      prevFile();
      break;
    case BTN_MODE:
      toggleMode();
      break;
    default: break;
  }

  if (!playing) return;
  if (sdcard.getCount() == 0) {
    stop();
    return;
  }

  // Hitung waktu total berlalu (waktu yang disimpan + waktu berjalan saat ini)
  uint64_t elapsed = elapsedUS + (esp_timer_get_time() - startUS);
  MidiEvent evtData;

  while (eventQueue.peek(evtData)) {
    if (evtData.timeUS > elapsed) break;
    eventQueue.pop(evtData);
    if (evtData.type == EVENT_NOTE_ON) {
      solenoid.hit(evtData.solenoidId, player.getSolenoidTime());
    }
  }

  if (eventQueue.empty()) {
    if (autoMode) {
      Serial.println("[PLAYER]: Loop: Jeda 2 detik sebelum berikutnya");
      display.showStatus("NEXT IN 2S");
      delay(2000);
      nextFile();
      play();
    } else {
      stop();
      display.showStatus("DONE");
    }
  }
}
