# gamelan-arduino

A compact Arduino-based implementation for creating gamelan-like percussion sounds and control. This repository contains C++/Arduino sketches and supporting code to drive physical actuators or generate MIDI/audio output to emulate gamelan ensembles.

## Features
- Play pre-programmed gamelan patterns and scales
- Support for physical actuators (solenoids, servos, piezo buzzers) or MIDI output
- Configurable pin mappings and timing/tempo settings
- Simple calibration/tuning routine for physical instruments
- Compatible with Arduino IDE, Arduino CLI, and PlatformIO

## Hardware (example)
The exact hardware depends on your target setup. Typical components:
- Arduino Uno / Nano / Mega (any AVR/ARM board supported by Arduino toolchain)
- Actuators: solenoids, small servos, or piezo buzzers
- Driver circuitry (transistors, MOSFETs, or driver boards) for solenoids/servos
- External power supply for actuators (do NOT power solenoids from the Arduino 5V rail)
- Optional: MIDI breakout (DIN or USB-MIDI) or MIDI over serial/USB interface
- Wires, protoboard, and mounting hardware

Refer to the wiring diagram in docs/wiring.md (or the `src/config.h` pin mappings) for the exact pin layout used by the code.

## Software requirements
- Arduino IDE 1.8+ or Arduino CLI, or PlatformIO
- Libraries (installed via Library Manager or PlatformIO):
  - (Examples) MIDI, EEPROM — check the top of the main sketch for required libs
- C++ source located in `src/` (or in the main `.ino` if single-file sketch)

## Install & Build

Using Arduino IDE
1. Open the main sketch (`gamelan.ino` or similar) in Arduino IDE.
2. Select your board and serial port (Tools → Board, Port).
3. Click Upload.

Using Arduino CLI
1. Compile: arduino-cli compile --fqbn <board_fqbn> /path/to/repo
2. Upload: arduino-cli upload -p /dev/ttyACM0 --fqbn <board_fqbn> /path/to/repo

Using PlatformIO
1. Install PlatformIO in VSCode.
2. Open the project folder and update `platformio.ini` board setting.
3. Run Build and Upload from the PlatformIO toolbar.

## Configuration
- Pin mappings, modes (MIDI vs Actuator), and timing constants are stored in a configuration header (e.g., `src/config.h`) or at the top of the main sketch.
- Change tempo and pattern settings in `patterns/` or the corresponding source files.
- If using external drivers, set the actuator enable/polarity in the config to match your driver board.

## Usage
- Power the Arduino and actuators (ensuring a common ground).
- Upload the sketch.
- Use serial monitor at configured baud (e.g., 115200) to:
  - Start/stop patterns
  - Trigger calibration
  - Switch modes (MIDI / standalone)
- Optional: Send MIDI to the Arduino to trigger notes, or have the Arduino send MIDI messages to a DAW/synth.

## Calibration & Tuning
1. Run the provided calibration routine (Serial command `calibrate`).
2. Adjust actuator timing and strike intensity variables in `config.h`.
3. For physical instruments, tune the fixed resonant elements (if any) mechanically or via damping materials.

## Troubleshooting
- Actuators not moving: check external power supply and driver wiring; confirm common ground.
- Strange timing: ensure no power supply current sag; try adding decoupling caps and separate supply for actuators.
- Serial/USB not connecting: check board selection and cable.

## Contributing
Contributions welcome. Please:
1. Fork the repo
2. Create a branch for your feature/fix
3. Open a pull request with a clear description and test steps

If you plan hardware changes, include wiring diagrams or photos and update docs.

## License
Choose a license for the project (e.g., MIT). Add LICENSE file to the repository.

## Acknowledgements
Inspired by traditional gamelan ensemble concepts and maker/hardware communities. Feel free to credit or link any reference projects you used.
