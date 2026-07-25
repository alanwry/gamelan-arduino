#include "scheduler.h"
#include "event_queue.h"
#include "mapping.h"
#include "solenoid.h"
#include "esp_timer.h"
#include "player.h"

Scheduler scheduler;

void Scheduler::begin() {
}

void Scheduler::play() {
  running = true;
  startUS = esp_timer_get_time();
}

void Scheduler::stop() {
  running = false;
  solenoid.allOff();
}

void Scheduler::pause() {
  running = false;
}

void Scheduler::update() {
  if (!running)
    return;

  uint64_t elapsed = esp_timer_get_time() - startUS;

  MidiEvent e;

  while (eventQueue.peek(e)) {
    if (e.timeUS > elapsed)
      break;

    eventQueue.pop(e);

    switch (e.type) {
      case EVENT_NOTE_ON:
        for (int i = 0; i < MAP_SIZE; i++) {
          if (noteMap[i].midi == e.note) {
            solenoid.hit(noteMap[i].solenoid, player.getSolenoidTime());
            break;
          }
        }
        break;

      default:
        break;
    }
  }
}
