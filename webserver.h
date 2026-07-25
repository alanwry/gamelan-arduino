#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>

class WebServerManager {
public:
  void begin();

  void stop();

  void update();

  bool isActive() const;
};

extern WebServerManager webServer;

#endif
