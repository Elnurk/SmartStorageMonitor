# SmartStorageMonitor

SmartStorageMonitor is an Arduino-based smart storage system designed to monitor environmental conditions and inventory in a storage room or warehouse.

## Features

- Monitors key environmental parameters such as temperature, humidity, and light.
- Triggers sound and LED alarm when movement is noticed inside of the storage room while Guard Mode is active.
- Controls actuators based on remote commands.
- Stores inventory data locally and transmits it over serial or network interfaces.
- Provides an easy-to-use serial command interface for debugging.

## Hardware

- Arduino-compatible microcontroller (e.g., Arduino Uno or Mega).
- Sensors: temperature/humidity (DHT22 or similar), light, and optional RFID reader.
- Actuators: relays to control lights, fans, or other devices.
- Additional modules as required (e.g., WiFi or Bluetooth for connectivity).

## Software / Stack

- Firmware written in C++ using the Arduino framework.
- `Main.ino` – the standard firmware for controlling sensors and actuators.

## Build & Upload

To build and upload the firmware:

### Using the Arduino IDE

1. Clone or download this repository.
2. Open `Main.ino` in the Arduino IDE.
3. Select the correct board and serial port from the **Tools** menu.
4. Click **Verify** to compile the sketch and **Upload** to flash it to your board.

### Using arduino-cli

If you prefer the command line:

```bash
# Install board core and dependencies if needed
arduino-cli core install arduino:avr

# Compile
arduino-cli compile --fqbn arduino:avr:uno Main.ino

# Upload (replace /dev/ttyUSB0 with your serial device)
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno Main.ino
```

## Contribution

Contributions are welcome. Please open an issue to discuss any changes or improvements.

## License

This project is licensed under the MIT License – see the `LICENSE` file for details.
