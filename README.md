# SmartStorage

SmartStorage is an Arduino-based smart storage system designed to monitor and manage environmental conditions and inventory in a storage room or warehouse. The project consists of two firmware variants for different hardware setups.

## Features

- Monitors key environmental parameters such as temperature, humidity, and light.
- Controls actuators like fans or lights based on sensor readings or remote commands.
- Stores inventory data locally and transmits it over serial or network interfaces.
- Provides an easy-to-use serial command interface for configuration and control.

## Hardware

- Arduino-compatible microcontroller (e.g., Arduino Uno or Mega).
- Sensors: temperature/humidity (DHT22 or similar), light, and optional RFID reader.
- Actuators: relays to control lights, fans, or other devices.
- Additional modules as required (e.g., WiFi or Bluetooth for connectivity).

## Software / Stack

- Firmware written in C++ using the Arduino framework.
- Two firmware sketches are included:
  - `SMART_SKLAD.ino` – the standard firmware for controlling sensors and actuators.
  - `SMART_SKLADBekaVer.ino` – an alternative version with customized features.

## Build & Upload

To build and upload the firmware:

### Using the Arduino IDE

1. Clone or download this repository.
2. Open either `SMART_SKLAD.ino` or `SMART_SKLADBekaVer.ino` in the Arduino IDE.
3. Select the correct board and serial port from the **Tools** menu.
4. Click **Verify** to compile the sketch and **Upload** to flash it to your board.

### Using arduino-cli

If you prefer the command line:

```bash
# Install board core and dependencies if needed
arduino-cli core install arduino:avr

# Compile
arduino-cli compile --fqbn arduino:avr:uno SMART_SKLAD.ino

# Upload (replace /dev/ttyUSB0 with your serial device)
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno SMART_SKLAD.ino
```

You can substitute `SMART_SKLAD.ino` with `SMART_SKLADBekaVer.ino` to build the alternative firmware.

## Contribution

Contributions are welcome. Please open an issue to discuss any changes or improvements.

## License

This project is licensed under the MIT License – see the `LICENSE` file for details.
