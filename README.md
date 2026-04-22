# ESP8266 Weather Clock

This project is an ESP8266-based scrolling weather clock built around MAX7219 LED matrix modules. It connects to Wi-Fi, fetches time via NTP, pulls weather data, and scrolls time, temperature, and custom messages across the display. The firmware is stored in `max72LedNodeMCU_Scroll_Working/max72LedNodeMCU_Scroll_Working.ino`.

## Recent updates (v10.9.17)

* Updated firmware version string to `max72LedNodeMCU_Scroll_Working_v10.9.17`.
* Hardened periodic weather/time refresh logic so scheduled updates are skipped (instead of blocking) while Wi-Fi is down and resume automatically after reconnect.
* Added timeout guards for NTP sync and weather HTTP calls to reduce the chance of lockups during temporary network outages (for example router reboots).
* MQTT queue polling now runs every loop cycle.
* MQTT status topics (including battery) are captured and shown at startup and during each display cycle.
* Added additional serial debug output for MQTT broker connection, subscriptions, incoming status messages, and state publishes.
* Documented MQTT broker/topic integration details in the firmware header comments.
* Refreshed the configuration portal styling and status messaging.
* Added editable custom date messages in the configuration portal (saved in `config.json`).
* Aligned message presets with date/message entry pairs to match the portal format.
* Improved DST auto-update handling during time syncs.
* Scrolls the assigned IP address after Wi-Fi connects.
* Calls out the optional MQTT message queue integration for remote message commands.
* Weather requests now use HTTPS and handle API error payloads more explicitly so bad responses do not appear as `Unknown`/`0C`.
* The built-in blue Wi-Fi LED now stays off during normal operation and only flashes when Wi-Fi has been disconnected for more than 5 minutes.

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

Open `max72LedNodeMCU_Scroll_Working/max72LedNodeMCU_Scroll_Working.ino` in the Arduino IDE or use `arduino-cli` and compile for **Generic ESP8266 Module** or **NodeMCU 1.0**. Update `max72LedNodeMCU_Scroll_Working/Config.h` with your Wi-Fi and OpenWeather API key before flashing.

## Basic controls

* **Power on**: the device boots, connects to Wi-Fi, and begins scrolling the clock and weather messages.
* **Wi-Fi credentials**: update `max72LedNodeMCU_Scroll_Working_v10.9.10/Config.h` with your Wi-Fi + OpenWeather API key before flashing.
* **Location settings**: update the default latitude/longitude in `Config.h` or use the on-device web portal to save them.
* **Display behavior**: choose a preset in `Config.h` (`MESSAGE_PRESET_NAME`) or in the config portal, and edit presets in `MessagePresets.h` to customize date-based messages using `{ "Mon DD", "Message" }` entry pairs.
* **Time zone**: set the default UTC offset in `Config.h` (`DEFAULT_TIMEZONE`) or update it in the config portal.
* **UK daylight saving**: the clock recalculates BST/GMT in firmware (last Sunday in March/October at 01:00 UTC) and updates the offset without requiring a reboot.
* **Time sync on boot**: the clock synchronizes time once during startup and applies any DST adjustment during that sync.
* **IP address on connect**: after Wi-Fi connects, the display scrolls the assigned IP address once so you can find the device on the network.
* **Wi-Fi stays connected**: the device keeps Wi-Fi in station mode and will auto-reconnect if the connection drops (the blue LED remains off and starts flashing only after 5 minutes of continuous Wi-Fi outage).

## MQTT message control (optional)

The clock can subscribe to MQTT messages and scroll them on the display. Set your broker details in `max72LedNodeMCU_Scroll_Working/Config.h` to enable it:

```cpp
const char* MQTT_BROKER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_PREFIX = "weatherclock";
```

When enabled, the device uses its chip ID to build topics:

* **Command topic:** `<MQTT_TOPIC_PREFIX>/<chipid>/command/message`
* **State topic:** `<MQTT_TOPIC_PREFIX>/<chipid>/state/message` (retained)

Publish plain text payloads to status topics under the command root (for example `<MQTT_TOPIC_PREFIX>/<chipid>/command/battery` or `/command/message`). The clock polls MQTT every loop cycle, stores the latest value for each status key, and scrolls battery/status values at startup and after each weather display cycle. The latest processed status is published to the state topic so Home Assistant or other clients can track it.

## Configuration portal

If the clock cannot connect to Wi-Fi, it starts an access point named `WeatherClock-<chipid>`. Connect to that network and visit `http://192.168.4.1` to set Wi-Fi credentials, latitude/longitude, message presets, time zone offset, and custom date messages. Settings are stored on the device and reused on future boots.

After saving settings in the portal, the access point remains available so you can reconnect and make additional changes without power-cycling the clock.

When the clock is connected to your network, the same configuration portal is available from the device's assigned IP address.

Custom date messages are entered one per line using the format `Mon DD | Message` (for example `Feb 14 | Happy Valentines Day`). Leave the list empty to continue using the selected preset in `MessagePresets.h`.

The portal also displays the device chip ID along with the MQTT command/state topics built from it, which you can copy into Home Assistant or other MQTT clients.

## Roadmap

Suggested improvements for future versions:

* **Web UI enhancements:** add form validation, clearer save/restart feedback, and mobile-friendly layout refinements in the configuration portal.
* **Weather resiliency:** support a secondary weather provider fallback and cache the last successful forecast with an on-screen "stale data" indicator.
* **Time/location usability:** add a searchable city lookup in the portal so latitude/longitude does not need to be entered manually.
* **Display personalization:** allow runtime brightness schedules, animation speed controls, and per-message duration settings from the portal or MQTT.
* **MQTT/Home Assistant integration:** provide optional auto-discovery topics and richer structured state payloads (JSON) for easier dashboard setup.
* **Diagnostics & updates:** add a lightweight diagnostics page (Wi-Fi RSSI, uptime, heap, last sync times) and optional OTA update workflow documentation.

