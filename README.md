# Smart Relay Control

A smart 2-channel relay control firmware built for the Wemos D1 Mini (ESP8266). It features a responsive, modern web interface for direct control and scheduling, integrates with an RTC (DS3231) and NTP for accurate timekeeping, and persists configurations using LittleFS.

## Features

- **Dual Channel Relay Control**: Independently control two relays via a modern, responsive web UI with glassmorphism design.
- **Scheduling**: Set custom ON/OFF schedules for each relay.
- **RTC & NTP Sync**: Uses a DS3231 RTC module for offline timekeeping and synchronizes with NTP servers when connected to the internet.
- **Setup Mode (AP)**: Broadcasts a setup AP (`SmartRelay_Setup`) for initial WiFi configuration.
- **Persistent Storage**: Saves schedules, relay states, and WiFi credentials on the flash memory using LittleFS.
- **RESTful API**: Exposes JSON APIs for status retrieval and configuration.

## Hardware Requirements

- Wemos D1 Mini (ESP8266)
- 2-Channel Relay Module
- DS3231 RTC Module (I2C)

## Pin Configuration

| Component | Pin (GPIO) | Wemos D1 Mini |
|-----------|------------|---------------|
| Relay 1   | 12         | D6            |
| Relay 2   | 13         | D7            |
| I2C SDA   | 4          | D2            |
| I2C SCL   | 5          | D1            |

## Setup and Installation

This project is built using [PlatformIO](https://platformio.org/).

1. Clone this repository.
2. Open the project in VS Code with the PlatformIO extension installed.
3. Build and upload the firmware to your Wemos D1 Mini.
4. On the first boot, the device will host a WiFi Access Point named `SmartRelay_Setup` (Password: `12345678`).
5. Connect to the access point and visit the web interface (typically `192.168.4.1`) to configure your local WiFi network. The device will automatically reboot and connect to your network.

## Dependencies

- [ArduinoJson](https://arduinojson.org/) (v6.x)
- [RTClib](https://github.com/adafruit/RTClib)
- LittleFS (Built-in)
- ESP8266WiFi / ESP8266WebServer (Built-in)

## License

This project is open-source and available under the MIT License.
