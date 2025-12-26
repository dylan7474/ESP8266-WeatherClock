# ESP8266 Weather Clock

This project is an ESP8266-based scrolling weather clock built around MAX7219 LED matrix modules. It connects to Wi-Fi, fetches time via NTP, pulls weather data, and scrolls time, temperature, and custom messages across the display. The firmware is stored in `max72LedNodeMCU_Scroll_Working_v10.9.10.ino`.

## Build

### Linux (Make)

This repo ships lightweight Makefiles for convenience, and the firmware build is done with the Arduino toolchain via Arduino CLI. The `make` target installs required libraries (MD_Parola, MD_MAX72XX, ArduinoJson, BMP280_DEV) before compiling, so ensure you have network access or preinstall those libraries with `arduino-cli lib install`.

```bash
./configure
make
```

The Arduino CLI compile step can take around 1–2 minutes on a cold build and may be quiet until it completes.

To run a dependency check before building (Arduino CLI + ESP8266 core), use:

```bash
make check
```

You can override the board FQBN or output directory when building:

```bash
make FQBN=esp8266:esp8266:generic BUILD_DIR=build
```

### Arduino build

Open `max72LedNodeMCU_Scroll_Working_v10.9.10.ino` in the Arduino IDE or use `arduino-cli` and compile for **Generic ESP8266 Module** or **NodeMCU 1.0**. Update `max72LedNodeMCU_Scroll_Working_v10.9.10/Config.h` with your Wi-Fi and OpenWeather API key before flashing.

## Basic controls

* **Power on**: the device boots, connects to Wi-Fi, and begins scrolling the clock and weather messages.
* **Wi-Fi credentials**: update `max72LedNodeMCU_Scroll_Working_v10.9.10/Config.h` with your Wi-Fi + OpenWeather API key before flashing.
* **Location settings**: update the default latitude/longitude in `Config.h` or use the on-device web portal to save them.
* **Display behavior**: choose a preset in `Config.h` (`MESSAGE_PRESET_NAME`) and edit presets in `MessagePresets.h` to customize date-based messages using `{ "Mon DD", "Message" }` entry pairs.

## Configuration portal

If the clock cannot connect to Wi-Fi, it starts an access point named `WeatherClock-<chipid>`. Connect to that network and visit `http://192.168.4.1` to set Wi-Fi credentials and the latitude/longitude. Settings are stored on the device and reused on future boots.

## Roadmap

* Add more configuration options to the portal (message presets, time zone).
