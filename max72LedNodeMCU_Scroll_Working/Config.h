#pragma once

// Sample configuration file. Replace placeholder values with your real settings.

const char* DEFAULT_WIFI_SSID = "YOUR_WIFI_SSID";
const char* DEFAULT_WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// OpenWeather API key (required for weather updates).
const char* OPENWEATHER_API_KEY = "YOUR_OPENWEATHER_API_KEY";

// Default location used when no saved config exists in flash.
const char* DEFAULT_LATITUDE = "54.54";
const char* DEFAULT_LONGITUDE = "-1.08";

// Message preset to use (see MessagePresets.h for available names).
const char* MESSAGE_PRESET_NAME = "family";

// Default time zone offset in hours from UTC (e.g. 0 for GMT, 1 for CET).
const int DEFAULT_TIMEZONE = 0;

// Device label used for the configuration portal SSID.
const char* DEVICE_NAME = "WeatherClock";

// MQTT settings for external message control (optional).
// Set MQTT_BROKER to your broker hostname/IP to enable MQTT.
const char* MQTT_BROKER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_PREFIX = "weatherclock";
