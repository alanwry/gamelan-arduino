# Gamelan SASAK (Arduino ESP32)

Compact implementation for controlling Gamelan-like percussion, now optimized for ESP32 with dual-core multitasking and web-based configuration.

## Key Architecture
- **Dual-Core FreeRTOS**: Uses `xTaskCreatePinnedToCore` to separate tasks:
  - **Core 1 (`midiTask`)**: Dedicated to high-precision MIDI processing (`player.update()`).
  - **Core 0 (`systemTask`)**: Dedicated to system maintenance, web server (`webserver.update()`), WiFi management, button inputs, display, and buzzer notifications.
- **Non-Blocking Buzzer**: Buzzer notifications utilize a non-blocking `timer-based` system to ensure functionality even during file I/O.
- **Robust WiFi Management**: Implements thread-safe WiFi switching between AP and STA modes to prevent memory panics (`Guru Meditation`).

## Features
- **MIDI Playback**: Plays MIDI files from SD card with solenoid support.
- **Web Interface**: Manage MIDI files, configure actuator timing, solenoid mappings, and WiFi settings via browser.
- **WiFi Connectivity**: Supports AP and STA modes for easy dashboard access.
- **Interactive Controls**: Buttons for playback, navigation, and AP mode toggling.
- **Status Notifications**: Audio feedback via active buzzer for system states (startup, buttons, mode changes, successful saves, firmware updates).

## Hardware
- ESP32-based controller.
- PCF8574 I/O expander for buttons/LEDs/buzzer.
- SD Card module for MIDI storage.
- Solenoid actuators with driver circuitry.
- Display (I2C) for status feedback.

## Configuration & Usage
- **WiFi Mode**: Hold the `MODE` button for 2s to switch to AP mode (192.168.4.1), or 5s to switch back to STA mode (if configured).
- **Web Dashboard**: Access `http://mydashboard.local` when connected to the system's WiFi to upload files, configure actuators, and update firmware.
- **Buzzer Patterns**:
  - **Startup**: 1 beep (150ms).
  - **Buttons**: 1 short beep (50ms).
  - **AP Mode**: 2 beeps (100ms each).
  - **Firmware/Wifi Save**: 1 long beep (300ms).

## Contributing & Troubleshooting
- Ensure solid power supply for actuators (use dedicated rail).
- If system panics occur during WiFi switching, logs usually indicate potential memory/concurrency issues; tasks are synchronized via mutexes to prevent this.
- Maintain consistency with `config.h` pin mappings.
