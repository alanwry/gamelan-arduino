#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <cstdint>

class Scheduler {
public:
  void begin();

  void update();

  void play();

  void stop();

  void pause();

private:
  bool running = false;

  uint64_t startUS = 0;
};

extern Scheduler scheduler;

#endif