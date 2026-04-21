# ESP8266 Weather Clock

This project is an ESP8266-based scrolling weather clock built around MAX7219 LED matrix modules. It connects to Wi-Fi, fetches time via NTP, pulls weather data, and scrolls time, temperature, and custom messages across the display. The firmware is stored in `max72LedNodeMCU_Scroll_Working/max72LedNodeMCU_Scroll_Working.ino`.

## Recent updates (v8.9.15)

* Incremented the firmware-reported version string to `v8.9.15` for this release.
* Updated the firmware-reported version string to `v8.9.14`, including the startup display text.
* Documented MQTT broker/topic integration details in the firmware header comments.
* Refreshed the configuration portal styling and status messaging.
* Added editable custom date messages in the configuration portal (saved in `config.json`).
* Aligned message presets with date/message entry pairs to match the portal format.
* Improved DST auto-update handling during time syncs.
* Scrolls the assigned IP address after Wi-Fi connects.
* Calls out the optional MQTT message queue integration for remote message commands.
* Hardened weather payload parsing so restart-time API responses with extra transport bytes no longer decode as `0C Wind 0 m/s`.
* Increased weather JSON parsing headroom and added a sketch-level `WEATHER_API_KEY` placeholder define for easier API key replacement.
* Switched weather API fetches to HTTPS with redirect support and explicit empty-payload diagnostics to prevent `EmptyInput` parse failures.

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

Open `max72LedNodeMCU_Scroll_Working/max72LedNodeMCU_Scroll_Working.ino` in the Arduino IDE or use `arduino-cli` and compile for **Generic ESP8266 Module** or **NodeMCU 1.0**. Update `max72LedNodeMCU_Scroll_Working/Config.h` with your Wi-Fi settings and set `WEATHER_API_KEY` in the sketch (default is `YOUR_OPENWEATHER_API_KEY`) before flashing.

## Basic controls

* **Power on**: the device boots, connects to Wi-Fi, and begins scrolling the clock and weather messages.
* **Wi-Fi credentials**: update `max72LedNodeMCU_Scroll_Working_v10.9.10/Config.h` with your Wi-Fi + OpenWeather API key before flashing.
* **Location settings**: update the default latitude/longitude in `Config.h` or use the on-device web portal to save them.
* **Display behavior**: choose a preset in `Config.h` (`MESSAGE_PRESET_NAME`) or in the config portal, and edit presets in `MessagePresets.h` to customize date-based messages using `{ "Mon DD", "Message" }` entry pairs.
* **Time zone**: set the default UTC offset in `Config.h` (`DEFAULT_TIMEZONE`) or update it in the config portal.
* **UK daylight saving**: the clock recalculates BST/GMT in firmware (last Sunday in March/October at 01:00 UTC) and updates the offset without requiring a reboot.
* **Time sync on boot**: the clock synchronizes time once during startup and applies any DST adjustment during that sync.
* **IP address on connect**: after Wi-Fi connects, the display scrolls the assigned IP address once so you can find the device on the network.
* **Wi-Fi stays connected**: the device keeps Wi-Fi in station mode and will auto-reconnect if the connection drops (LED blinks while reconnecting).
* **Overnight router reboots**: periodic weather/time refresh now uses bounded reconnect/time-sync attempts, so temporary Wi-Fi outages won't trap the firmware in blocking loops.

## MQTT message control (optional)

The clock can subscribe to MQTT messages and scroll them on the display. Set your broker details in `max72LedNodeMCU_Scroll_Working/Config.h` to enable it:

```cpp
#define MQTT_BROKER "YOUR_MQTT_BROKER"
#define MQTT_PORT 1883
#define MQTT_TOPIC_PREFIX "weatherclock"
```

If these defines are omitted in a local `Config.h`, firmware now falls back to placeholder defaults so the sketch still compiles with MQTT disabled.

When enabled, the device uses its chip ID to build topics:

* **Command topic:** `<MQTT_TOPIC_PREFIX>/<chipid>/command/message`
* **State topic:** `<MQTT_TOPIC_PREFIX>/<chipid>/state/message` (retained)

Publish a plain text payload to the command topic and the clock will store it, then scroll it after each weather update (similar to the custom date messages). The latest stored message is published to the state topic so Home Assistant or other clients can track it.

## Configuration portal

If the clock cannot connect to Wi-Fi, it starts an access point named `WeatherClock-<chipid>`. Connect to that network and visit `http://192.168.4.1` to set Wi-Fi credentials, latitude/longitude, message presets, time zone offset, and custom date messages. Settings are stored on the device and reused on future boots.

After saving settings in the portal, the access point remains available so you can reconnect and make additional changes without power-cycling the clock.

When the clock is connected to your network, the same configuration portal is available from the device's assigned IP address.

Custom date messages are entered one per line using the format `Mon DD | Message` (for example `Feb 14 | Happy Valentines Day`). Leave the list empty to continue using the selected preset in `MessagePresets.h`.

The portal also displays the device chip ID along with the MQTT command/state topics built from it, which you can copy into Home Assistant or other MQTT clients.
