// Ver 10.9.11 (26/12/25)
// Modernized the config portal styling and status messaging
// Added editable custom date messages in the config portal (stored in config.json)
// Refactored message presets into date/message entry pairs
// Improved DST auto-update handling during clock syncs
// Scroll the assigned IP address after WiFi connects
//
// Ver 10.9.10 (29/08/22)
// Fixed birthday messages, added a few more and added 10 day reminders
//
// Updates (01/06/24)
// The fix for wifi on the ESP8266 is to use Wireless Mode : Legacy

// Ver 10.9.9 (27/03/22)
// Changed the march dst setting - seemed to wrong 298 , 303


// Ver 10.9.8 (07/11/21)
// Now that we have the date captured have added a routine to display extra messages depending on the date
// The are 2 arrays, one with dates and another with corresponding messages - EG HAPPY BIRTHDAY!

// Ver 10.9.7 (05/11/21)
// Change clock sync to every 12 hours (test to see if it drifts much)
// Had to add a routine to remove 4 characters from the weather payload when connecting via 4G
// What an annoying situation  checking the start of the payload string and truncate if needed

// Ver 10.9.6 (02/11/21)
// Adding code to work out clock change - used lookup table rather than date logic - was just easier to program!
// Currently have 5 years programmed in 2021-2040
// now applies DST during time sync without a second sync
// Currently this will update dst when it syncs the clock once per hour - not necessarily on the hour.
// To get it to do the change on the hour would need to check the date each cycle
// Not sure if this would cause a LOOP with the dst switching back an hour over midnight (if thats how it would work)

// Ver 10.9.5 (01/11/21)
// Changed WIFI to CHIGLEY and had to enable on router for 2.4Ghz Auto select channel including channel 12, 13
// wifi connect seems a bit ropey - was working then stopped - will try closer to the router!
// so added a wifi scanning scrolling * message while connecting to wifi
//
// Due to library changes had to modify the HTTPClient code from
//
//  HTTPClient http;
//  http.begin(url);
//
//  TO
//
//  WiFiClient client;
//  HTTPClient http;
//  http.begin(client,url);
//
// Split the timing of the weather and timing updates
// so that weather is every 15 mins and weather is hourly
//
// Also removed the NEWS code as unlikely to use again
//
// Also added a fast scrolling * when trying to connect to wifi

// Ver 10.9.4
// Changed WIFI to WONKER9 to WONKER

// Ver 10.9.3
// Added comment on DST for click change
// fixed issue setting time & changed to sync t weather & time every 15 mins
// could change time syn to hourly but firing up wifi for weather anyway

// Ver 10.9.2
// News disabled (takes up too much screen time and isn't reliable
// Memory tracking also disabled
// Issue with keeping time after first boot - not sure settime is working as expected

// Ver 10.9.1
// Tracking memory usage

// Ver 10.9
// Replaced message length of 900 with auto (.length)

// Ver 10.8
// Updated the weather message to use the new routine & output
// Added GetNews routine, display routine updated but news not working (http error = -1) looks like related to https requests (weather is http)
// Output message max length increased from 50 to 900 - Set it to auto using string length function
// First news item seems to be getting truncated (might be a power thing on old laptop!)
// CHanged news from https to http
// modified message frequencet to 60s from 15s & text speed to 15 (quick)
// Changed BMP380 multiplier to 0.8 and made it a variable rather than hard coded (kinda)

// Ver 10.7
// This version changed to be like esp32 version
// GetWeatherNew routine added - compiles ok but not called yet

// Ver 10.6
// Now in GMT - need to make auto-clock change
// Also updated time hosts

// Ver 10.5
// Rejigging the display to add In/Out to temperature data
//
// Ver 10.4
// Added BMP280 with a scaling of 0.95 on the temperature
//
// https://github.com/ThingPulse/esp8266-weather-station/blob/master/examples/OpenWeatherMapCurrentDemo/OpenWeatherMapCurrentDemo.ino
//
// Display to ESP8266 Wiring
// Vcc - 3v
// GND - G
// DIN - D7
// CS  - D8
// CLK - D5
//I2C device found at address 0x76
// BMP280 to ESP8266 Wiring
// Vcc - 3v
// GND - G
// SCL - D1
// SDA - D2
// Reference http://www.esp8266learning.com/esp8266-bmp280-sensor-example.php

// Compile for Generic ESP8266 Module or NodeMCU 1.0

#include <string.h>
#include <ctype.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>

// Added for Version 10.4
#include <BMP280_DEV.h>
float temperature, pressure, altitude;  // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280;

// Added for Version 10.7
// #include <HTTPClient.h>  //from esp32...doesn't seem to work
#include <ESP8266HTTPClient.h>

#include "Config.h"
#include "MessagePresets.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14
#define DATA_PIN 13
#define CS_PIN 15

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String Version = "max72LedNodeMCU_Scroll_Working_v10.9.11";
float TempScale = 0.78;
int timezone = 0;
int dst = 0;  //dst = 0 for GMT , dst = 1 for bst
String nowTime;
String nowTimeShort;
String oldNowTime = "";

String StoredWeatherMainData;
String StoredWeatherDescription;
String StoredWeatherTemperature;

const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();

int bigcount = 0;
const unsigned long kWiFiReconnectBackoffMs = 30000;
const unsigned long kWiFiStatusBlinkMs = 500;
const int kWiFiStatusLedPin = LED_BUILTIN;
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastWiFiStatusBlink = 0;
bool wifiStatusLedOn = false;
const unsigned long kMqttReconnectBackoffMs = 5000;

String payload;
boolean StartupState = true;
//This is a messy way to do it - replace with array down the line
//String BigNewsMess,mess1,mess2,mess3,des1,des2,des3,news1,news2,news3; //Message veriables for displaying news
String wx1, wx2, wx3;  //Message variables for displaying weather

//Memory tracking stuff
uint32_t memcurr = 0;
uint32_t memlast = 0;
uint32_t counter = 0;

//Definition for Clock Change - 2021 - 2040
//Index is (year-2021)
int mar[20] = { 28, 27, 26, 31, 30, 29, 28, 26, 25, 31, 30, 28, 27, 26, 25, 30, 29, 28, 27, 25 };
int oct[20] = { 31, 30, 29, 27, 26, 25, 31, 29, 28, 27, 26, 31, 30, 29, 28, 26, 25, 31, 30, 28 };

const MessageEntry* MessageEntries = nullptr;
size_t NoOfMessages = 0;
String today;

struct RuntimeConfig {
  String ssid;
  String password;
  String latitude;
  String longitude;
  String messagePreset;
  String customMessages;
  int timezoneOffset;
};

RuntimeConfig runtimeConfig;
ESP8266WebServer configServer(80);
bool configPortalSaved = false;
bool configServerRunning = false;
const char* kConfigPath = "/config.json";

WiFiClient mqttWiFiClient;
PubSubClient mqttClient(mqttWiFiClient);
bool mqttEnabled = false;
String mqttClientId;
String mqttTopicBase;
String mqttCommandTopic;
String mqttStateTopic;
String pendingMqttMessage;
bool mqttMessagePending = false;
unsigned long lastMqttReconnectAttempt = 0;

struct CustomMessageStore {
  MessageEntry* entries = nullptr;
  String* dates = nullptr;
  String* messages = nullptr;
  size_t count = 0;
};

CustomMessageStore customMessages;

const MessagePreset* FindMessagePreset(const char* name) {
  for (size_t i = 0; i < kMessagePresetCount; i++) {
    if (String(kMessagePresets[i].name).equalsIgnoreCase(name)) {
      return &kMessagePresets[i];
    }
  }
  return nullptr;
}

void ApplyMessagePreset(const MessagePreset* preset) {
  if (preset == nullptr) {
    return;
  }
  MessageEntries = preset->entries;
  NoOfMessages = preset->count;
}

void ApplyDefaultConfig() {
  runtimeConfig.ssid = DEFAULT_WIFI_SSID;
  runtimeConfig.password = DEFAULT_WIFI_PASSWORD;
  runtimeConfig.latitude = DEFAULT_LATITUDE;
  runtimeConfig.longitude = DEFAULT_LONGITUDE;
  runtimeConfig.messagePreset = MESSAGE_PRESET_NAME;
  runtimeConfig.customMessages = "";
  runtimeConfig.timezoneOffset = DEFAULT_TIMEZONE;
}

bool IsMqttConfigured() {
  if (MQTT_BROKER == nullptr) {
    return false;
  }
  if (strlen(MQTT_BROKER) == 0) {
    return false;
  }
  if (strcmp(MQTT_BROKER, "YOUR_MQTT_BROKER") == 0) {
    return false;
  }
  return true;
}

void HandleMqttMessage(char* topic, byte* payloadBytes, unsigned int length) {
  if (!mqttCommandTopic.equals(topic)) {
    return;
  }

  String payloadString;
  payloadString.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) {
    payloadString += static_cast<char>(payloadBytes[i]);
  }
  payloadString.trim();
  if (payloadString.length() == 0) {
    return;
  }
  pendingMqttMessage = payloadString;
  mqttMessagePending = true;
}

void SetupMqtt() {
  mqttEnabled = IsMqttConfigured();
  if (!mqttEnabled) {
    return;
  }
  mqttClientId = String("weatherclock-") + String(ESP.getChipId(), HEX);
  mqttTopicBase = String(MQTT_TOPIC_PREFIX) + "/" + String(ESP.getChipId(), HEX);
  mqttCommandTopic = mqttTopicBase + "/command/message";
  mqttStateTopic = mqttTopicBase + "/state/message";

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(HandleMqttMessage);
}

void EnsureMqttConnection() {
  if (!mqttEnabled) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (mqttClient.connected()) {
    return;
  }
  unsigned long now = millis();
  if (now - lastMqttReconnectAttempt < kMqttReconnectBackoffMs) {
    return;
  }
  lastMqttReconnectAttempt = now;
  if (mqttClient.connect(mqttClientId.c_str())) {
    mqttClient.subscribe(mqttCommandTopic.c_str());
  }
}

void PublishMqttState(const String& message) {
  if (!mqttEnabled || !mqttClient.connected()) {
    return;
  }
  mqttClient.publish(mqttStateTopic.c_str(), message.c_str(), true);
}

bool LoadRuntimeConfig() {
  ApplyDefaultConfig();
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return false;
  }
  if (!LittleFS.exists(kConfigPath)) {
    return false;
  }
  File configFile = LittleFS.open(kConfigPath, "r");
  if (!configFile) {
    return false;
  }
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();
  if (error) {
    return false;
  }
  runtimeConfig.ssid = doc["ssid"] | runtimeConfig.ssid;
  runtimeConfig.password = doc["password"] | runtimeConfig.password;
  runtimeConfig.latitude = doc["latitude"] | runtimeConfig.latitude;
  runtimeConfig.longitude = doc["longitude"] | runtimeConfig.longitude;
  runtimeConfig.messagePreset = doc["messagePreset"] | runtimeConfig.messagePreset;
  runtimeConfig.customMessages = doc["customMessages"] | runtimeConfig.customMessages;
  runtimeConfig.timezoneOffset = doc["timezoneOffset"] | runtimeConfig.timezoneOffset;
  return true;
}

bool SaveRuntimeConfig() {
  if (!LittleFS.begin()) {
    return false;
  }
  DynamicJsonDocument doc(2048);
  doc["ssid"] = runtimeConfig.ssid;
  doc["password"] = runtimeConfig.password;
  doc["latitude"] = runtimeConfig.latitude;
  doc["longitude"] = runtimeConfig.longitude;
  doc["messagePreset"] = runtimeConfig.messagePreset;
  doc["customMessages"] = runtimeConfig.customMessages;
  doc["timezoneOffset"] = runtimeConfig.timezoneOffset;
  File configFile = LittleFS.open(kConfigPath, "w");
  if (!configFile) {
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();
  return true;
}

void ClearCustomMessages() {
  delete[] customMessages.entries;
  delete[] customMessages.dates;
  delete[] customMessages.messages;
  customMessages.entries = nullptr;
  customMessages.dates = nullptr;
  customMessages.messages = nullptr;
  customMessages.count = 0;
}

String NormalizeDateToken(const String& raw) {
  String trimmed = raw;
  trimmed.trim();
  if (trimmed.length() < 4) {
    return "";
  }
  String month = trimmed.substring(0, 3);
  month.toLowerCase();
  month.setCharAt(0, toupper(month.charAt(0)));
  String dayPart = trimmed.substring(3);
  dayPart.trim();
  int day = dayPart.toInt();
  if (day < 1 || day > 31) {
    return "";
  }
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%s %2d", month.c_str(), day);
  return String(buffer);
}

bool ApplyCustomMessages(const String& text) {
  ClearCustomMessages();
  String payloadText = text;
  payloadText.trim();
  if (payloadText.length() == 0) {
    return false;
  }
  size_t lineCount = 0;
  int start = 0;
  while (start < payloadText.length()) {
    int end = payloadText.indexOf('\n', start);
    if (end == -1) {
      end = payloadText.length();
    }
    String line = payloadText.substring(start, end);
    line.trim();
    int separator = line.indexOf('|');
    if (line.length() > 0 && separator > 0 && separator < line.length() - 1) {
      lineCount++;
    }
    start = end + 1;
  }
  if (lineCount == 0) {
    return false;
  }
  customMessages.entries = new MessageEntry[lineCount];
  customMessages.dates = new String[lineCount];
  customMessages.messages = new String[lineCount];
  customMessages.count = 0;
  start = 0;
  while (start < payloadText.length()) {
    int end = payloadText.indexOf('\n', start);
    if (end == -1) {
      end = payloadText.length();
    }
    String line = payloadText.substring(start, end);
    line.trim();
    int separator = line.indexOf('|');
    if (line.length() > 0 && separator > 0 && separator < line.length() - 1) {
      String dateToken = line.substring(0, separator);
      String messageToken = line.substring(separator + 1);
      dateToken.trim();
      messageToken.trim();
      if (messageToken.length() > 0) {
        String normalizedDate = NormalizeDateToken(dateToken);
        if (normalizedDate.length() > 0) {
          customMessages.dates[customMessages.count] = normalizedDate;
          customMessages.messages[customMessages.count] = messageToken;
          customMessages.entries[customMessages.count] = { customMessages.dates[customMessages.count].c_str(),
                                                          customMessages.messages[customMessages.count].c_str() };
          customMessages.count++;
        }
      }
    }
    start = end + 1;
  }
  if (customMessages.count == 0) {
    ClearCustomMessages();
    return false;
  }
  MessageEntries = customMessages.entries;
  NoOfMessages = customMessages.count;
  return true;
}

void ApplyMessageConfiguration() {
  if (!ApplyCustomMessages(runtimeConfig.customMessages)) {
    const char* presetName = runtimeConfig.messagePreset.length() > 0
                               ? runtimeConfig.messagePreset.c_str()
                               : MESSAGE_PRESET_NAME;
    const MessagePreset* preset = FindMessagePreset(presetName);
    if (preset == nullptr) {
      preset = &kMessagePresets[0];
    }
    ApplyMessagePreset(preset);
  }
}

void ApplyTimezoneConfiguration() {
  timezone = runtimeConfig.timezoneOffset;
}

String BuildConfigPage() {
  String page = F("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<title>Weather Clock Portal</title>"
                  "<style>"
                  ":root{color-scheme:dark;--bg:#05070f;--panel:#11162a;--accent:#27f3ff;--accent2:#9b6bff;--text:#e6f0ff;--muted:#8ea0c8;}"
                  "body{margin:0;font-family:'Segoe UI',Roboto,Arial,sans-serif;background:radial-gradient(circle at top,#111a3a,#05070f 55%);color:var(--text);}"
                  ".wrap{max-width:720px;margin:32px auto;padding:0 20px;}"
                  ".card{background:linear-gradient(145deg,rgba(23,32,66,.92),rgba(8,12,26,.95));"
                  "border:1px solid rgba(39,243,255,.2);border-radius:20px;padding:28px;box-shadow:0 20px 40px rgba(4,8,20,.65);}"
                  "h1{margin:0 0 8px;font-size:28px;letter-spacing:.04em;}"
                  "p{margin:0 0 24px;color:var(--muted);}"
                  "label{display:block;margin-top:16px;font-size:13px;text-transform:uppercase;letter-spacing:.12em;color:var(--muted);}"
                  "input,textarea,select{width:100%;margin-top:8px;padding:12px 14px;border-radius:12px;border:1px solid rgba(139,160,200,.3);"
                  "background:rgba(7,10,20,.9);color:var(--text);box-shadow:inset 0 0 0 1px rgba(39,243,255,.08);}"
                  "input:focus,textarea:focus,select:focus{outline:none;border-color:var(--accent);box-shadow:0 0 0 2px rgba(39,243,255,.2);}"
                  "textarea{min-height:160px;font-family:'SFMono-Regular',Consolas,monospace;font-size:13px;line-height:1.5;}"
                  ".hint{margin-top:8px;font-size:12px;color:var(--muted);}"
                  "button{margin-top:22px;padding:12px 22px;border:none;border-radius:999px;font-weight:600;letter-spacing:.08em;"
                  "color:#031018;background:linear-gradient(135deg,var(--accent),var(--accent2));box-shadow:0 12px 30px rgba(39,243,255,.35);}"
                  ".status{margin-top:18px;font-size:12px;color:var(--muted);}"
                  "</style></head><body><div class='wrap'><div class='card'>"
                  "<h1>Weather Clock Portal</h1><p>Configure Wi-Fi, location, time zone, and custom messages.</p>"
                  "<form method='POST' action='/save'>");
  page += "<label>Wi-Fi SSID</label><input name='ssid' value='" + runtimeConfig.ssid + "'>";
  page += "<label>Wi-Fi Password</label><input name='password' type='password' value='" + runtimeConfig.password + "'>";
  page += "<label>Latitude</label><input name='latitude' value='" + runtimeConfig.latitude + "'>";
  page += "<label>Longitude</label><input name='longitude' value='" + runtimeConfig.longitude + "'>";
  page += "<label>Message preset</label><select name='messagePreset'>";
  for (size_t i = 0; i < kMessagePresetCount; i++) {
    String presetName = kMessagePresets[i].name;
    page += "<option value='" + presetName + "'";
    if (presetName.equalsIgnoreCase(runtimeConfig.messagePreset)) {
      page += " selected";
    }
    page += ">" + presetName + "</option>";
  }
  page += "</select>";
  page += "<label>Time zone offset (UTC hours)</label><input name='timezoneOffset' type='number' step='1' min='-12' max='14' value='"
          + String(runtimeConfig.timezoneOffset) + "'>";
  page += "<div class='hint'>Use a whole-hour offset from UTC (e.g. -5 for EST, 1 for CET).</div>";
  page += "<label>Custom date messages</label><textarea name='customMessages' placeholder='Jan 1 | Happy New Year'>" + runtimeConfig.customMessages + "</textarea>";
  page += "<div class='hint'>Use one message per line: <strong>Mon DD | Message</strong> (e.g. <strong>Feb 14 | Happy Valentines Day</strong>). Leave empty to use the preset above.</div>";
  page += "<button type='submit'>Save</button></form><div class='status'>Signals will refresh after saving.</div></div></div></body></html>";
  return page;
}

void HandleConfigRoot() {
  configServer.send(200, "text/html", BuildConfigPage());
}

void HandleConfigSave() {
  if (configServer.hasArg("ssid")) {
    runtimeConfig.ssid = configServer.arg("ssid");
  }
  if (configServer.hasArg("password")) {
    runtimeConfig.password = configServer.arg("password");
  }
  if (configServer.hasArg("latitude")) {
    runtimeConfig.latitude = configServer.arg("latitude");
  }
  if (configServer.hasArg("longitude")) {
    runtimeConfig.longitude = configServer.arg("longitude");
  }
  if (configServer.hasArg("messagePreset")) {
    runtimeConfig.messagePreset = configServer.arg("messagePreset");
  }
  if (configServer.hasArg("timezoneOffset")) {
    runtimeConfig.timezoneOffset = configServer.arg("timezoneOffset").toInt();
  }
  if (configServer.hasArg("customMessages")) {
    runtimeConfig.customMessages = configServer.arg("customMessages");
  }
  SaveRuntimeConfig();
  ApplyMessageConfiguration();
  ApplyTimezoneConfiguration();
  configPortalSaved = true;
  configServer.send(200, "text/html", "<html><body style='font-family:Arial;background:#05070f;color:#e6f0ff;padding:20px;'><h1>Saved</h1><p>Settings updated. You can close this page.</p></body></html>");
}

void StartConfigPortalServer() {
  if (configServerRunning) {
    return;
  }
  configServer.on("/", HTTP_GET, HandleConfigRoot);
  configServer.on("/save", HTTP_POST, HandleConfigSave);
  configServer.begin();
  configServerRunning = true;
}

void StartConfigPortal() {
  configPortalSaved = false;
  WiFi.mode(WIFI_AP_STA);
  String apName = String(DEVICE_NAME) + "-" + String(ESP.getChipId(), HEX);
  WiFi.softAP(apName.c_str());
  IPAddress apIp = WiFi.softAPIP();
  Serial.print("Config portal AP: ");
  Serial.println(apName);
  Serial.print("Open http://");
  Serial.println(apIp);
  ScrollMsg("Config portal at 192.168.4.1", 15);

  StartConfigPortalServer();

  while (!configPortalSaved) {
    configServer.handleClient();
    delay(10);
  }

  configServer.stop();
  configServerRunning = false;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
}

void MonitorWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (lastWiFiReconnectAttempt != 0 && now - lastWiFiReconnectAttempt < kWiFiReconnectBackoffMs) {
    return;
  }

  Serial.println("WiFi disconnected, attempting reconnect");
  WiFi.mode(WIFI_STA);
  WiFi.begin(runtimeConfig.ssid.c_str(), runtimeConfig.password.c_str());
  lastWiFiReconnectAttempt = now;
}

void UpdateWiFiStatusIndicator() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(kWiFiStatusLedPin, LOW);
    wifiStatusLedOn = true;
    return;
  }

  unsigned long now = millis();
  if (now - lastWiFiStatusBlink >= kWiFiStatusBlinkMs) {
    lastWiFiStatusBlink = now;
    wifiStatusLedOn = !wifiStatusLedOn;
    digitalWrite(kWiFiStatusLedPin, wifiStatusLedOn ? LOW : HIGH);
  }
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi ");
    Serial.println(runtimeConfig.ssid);
    WiFi.begin(runtimeConfig.ssid.c_str(), runtimeConfig.password.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      Serial.print("*");
      ScrollMsg("Connecting to wifi", 15);
      delay(1000);
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi connect failed");
      StartConfigPortal();
    }
  }
  Serial.println("\nCONNECTED");
  IPAddress stationIp = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(stationIp);
  ScrollMsg("IP " + stationIp.toString(), 15);
}

int CalculateDstForUtc(time_t nowUtc) {
  tm timeinfo;
  gmtime_r(&nowUtc, &timeinfo);
  int year = timeinfo.tm_year + 1900;
  int month = timeinfo.tm_mon + 1;
  int day = timeinfo.tm_mday;
  int hour = timeinfo.tm_hour;

  if (year < 2021 || year > 2040) {
    return dst;
  }

  int marDay = mar[year - 2021];
  int octDay = oct[year - 2021];

  if (month < 3 || month > 10) {
    return 0;
  }

  if (month > 3 && month < 10) {
    return 1;
  }

  if (month == 3) {
    if (day < marDay) {
      return 0;
    }
    if (day > marDay) {
      return 1;
    }
    return hour >= 1 ? 1 : 0;
  }

  if (month == 10) {
    if (day < octDay) {
      return 1;
    }
    if (day > octDay) {
      return 0;
    }
    return hour >= 1 ? 0 : 1;
  }

  return 0;
}

bool ApplyDstIfNeeded(time_t nowUtc) {
  int newDst = CalculateDstForUtc(nowUtc);
  if (newDst == dst) {
    return false;
  }

  dst = newDst;
  configTime(timezone * 3600, dst * 3600, "0.uk.pool.ntp.org", "1.uk.pool.ntp.org");
  Serial.print("DST updated to ");
  Serial.println(dst);
  return true;
}

void SetTime(void) {
  // In most cases it's best to use pool.ntp.org to find an NTP server (or 0.pool.ntp.org, 1.pool.ntp.org
  // , etc if you need multiple server names)
  // for list : https://gist.github.com/mutin-sa/eea1c396b1e610a2da1e5550d94b0453

  //configTime(timezone * 3600, dst * 3600, "time.google.com", "uk.pool.ntp.org");  //time.google.com //uk.pool.ntp.org
  configTime(timezone * 3600, dst * 3600, "0.uk.pool.ntp.org", "1.uk.pool.ntp.org");
  Serial.print("Synchronising Time : ");

  nowTime = "";  // have to reset this variable to tell if the time read was seccessful

  while (nowTime.substring(20, 21) != "2")  //Think this check for the year to check if time has been set
  {
    //Serial.println("\nACTUALLY UPDATING THE TIME\n");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.print(".");
    ScrollMsg("Synchronising Time", 15);
    time_t now = time(nullptr);

    nowTime = ctime(&now);

  nowTimeShort = nowTime.substring(10, 16);

  delay(1000);  // is this really needed?
  }
  Serial.print("\nTime Synchronised : ");
  Serial.println(nowTime);
  time_t nowUtc = time(nullptr);
  if (ApplyDstIfNeeded(nowUtc)) {
    time_t nowLocal = time(nullptr);
    nowTime = ctime(&nowLocal);
    nowTimeShort = nowTime.substring(10, 16);
    Serial.print("Time adjusted for DST: ");
    Serial.println(nowTime);
  }
}

void GetWeather() {

  // Connection string should be http://api.openweathermap.org/data/2.5/weather?lat=54.54&lon=-1.08&APPID=c848eb234e72b8e3808ab65d17d05231
  // Should output something like
  // {"coord":{"lon":-1.08,"lat":54.54},"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"base":"stations","main":{"temp":283.91,"pressure":994,"humidity":71,"temp_min":282.15,"temp_max":285.93},"visibility":10000,"wind":{"speed":3.6,"deg":240},"clouds":{"all":20},"dt":1571396447,"sys":{"type":1,"id":1428,"message":0.0093,"country":"GB","sunrise":1571380671,"sunset":1571418055},"timezone":3600,"id":7291323,"name":"Guisborough","cod":200}
  // json parsed from https://arduinojson.org/v6/assistant/

  Serial.println("Getting Weather");
  ScrollMsg("Getting Weather", 15);
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + runtimeConfig.latitude + "&lon=" + runtimeConfig.longitude
               + "&units=metric&APPID=" + OPENWEATHER_API_KEY;
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);    //Specify the URL
  int httpCode = http.GET();  //Make the request
  if (httpCode > 0) {         //Check for the returning code
    payload = http.getString();

    //When using 4G this payload seems to have an additional string at the beginning which is thowing off the decode
    //1e0
    //{"coord"
    // Instead just
    // {"coord"
    // Also seen 1e3 so looks like the number changes
    // need to be able to skip over this if the start of the payload is not a {

    int payloadlen = payload.length() - 4;

    Serial.print(">>>>>>>> FIRST payload CHAR IS ");
    Serial.print(payload.substring(0, 1));
    Serial.println(" >>>>>>>>>>");

    if (payload.substring(0, 1) != "{") {
      payload = payload.substring(4, payloadlen);
      Serial.println("For the moment will just strip off the first 4 chars");
      ScrollMsg("4G mode", 15);
    }
  } else {
    Serial.println("Error on Weather HTTP request");
  }
  http.end();

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(0) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(14) + 270;
  DynamicJsonDocument doc(capacity);

  int payloadlen = payload.length() + 1;

  char Tjson[payloadlen];
  payload.toCharArray(Tjson, payloadlen);
  char* json = Tjson;

  deserializeJson(doc, json);

  JsonObject main = doc["main"];
  const char* name = doc["name"];
  wx1 = reinterpret_cast<const char*>(name);

  JsonObject weather_0 = doc["weather"][0];
  int weather_0_id = weather_0["id"];                            // 500
  const char* weather_0_main = weather_0["main"];                // "Rain"
  const char* weather_0_description = weather_0["description"];  // "light rain"
  const char* weather_0_icon = weather_0["icon"];                // "10d"];
  wx2 = reinterpret_cast<const char*>(weather_0_description);

  float wind_speed = doc["wind"]["speed"];  // 1.5
  float main_temp = main["temp"];

  Serial.println(wx1);
  Serial.println(wx2);
  Serial.print("Wind Speed : ");
  Serial.println(wind_speed);
  Serial.print("Temperature : ");
  Serial.println(main_temp);

  wx3 = wx2 + " " + (int)main_temp + "C Wind " + (int)wind_speed + " m/s";

  Serial.print("Weather Message : ");
  Serial.println(wx3);

  StoredWeatherDescription = wx3;  //Set this for compatibility
}

void ScrollMsg(String messageString, int messageSpeed) {
  messageString += "           ";

  int messageLen = messageString.length() + 1;
  char message[messageLen];

  messageString.toCharArray(message, messageLen);  //Have to convert string to char array for display
  P.displayClear();
  P.displayReset();
  P.displayText(message, PA_CENTER, messageSpeed, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!P.displayAnimate()) {
    delay(1);
  }
}

void PrintMsg(String messageString) {
  int messageLen = messageString.length() + 1;
  char message[messageLen];

  messageString.toCharArray(message, messageLen);  //Have to convert string to char array for display
  P.displayClear();
  P.displayReset();
  P.displayText(message, PA_CENTER, 15, 2000, PA_PRINT, PA_NO_EFFECT);
  while (!P.displayAnimate()) {
    delay(1);
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("\n");
  Serial.println(Version);
  P.begin();
  ScrollMsg(Version, 15);
  pinMode(kWiFiStatusLedPin, OUTPUT);
  digitalWrite(kWiFiStatusLedPin, HIGH);

  LoadRuntimeConfig();
  ApplyMessageConfiguration();
  ApplyTimezoneConfiguration();

  // The below just for testing so you can see that the messages are correct
  int count = 0;
  while (count < NoOfMessages) {
    if (today == MessageEntries[count].date) {
      ScrollMsg(MessageEntries[count].message, 20);
    }
    Serial.print(MessageEntries[count].date);
    Serial.print(" - ");
    Serial.println(MessageEntries[count].message);
    count++;
  }

  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

  connectWifi();
  SetupMqtt();
  EnsureMqttConnection();
  SetTime();  //sync time and apply dst if needed
  GetWeather();
  StartConfigPortalServer();

  // Added for Version 10.4
  bmp280.begin(BMP280_I2C_ALT_ADDR);
  bmp280.setTempOversampling(OVERSAMPLING_X2);
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);  // Set the standby time to 2 seconds
  bmp280.startNormalConversion();

  while (!bmp280.getMeasurements(temperature, pressure, altitude))  // Check if the measurement is complete
  {
    delay(1);
  }

  Serial.println("BMP280 data");
  Serial.println(temperature);
  Serial.println(temperature * TempScale);
}

void loop(void) {
  MonitorWiFiConnection();
  UpdateWiFiStatusIndicator();
  if (configServerRunning) {
    configServer.handleClient();
  }
  EnsureMqttConnection();
  if (mqttEnabled) {
    mqttClient.loop();
  }
  if (mqttMessagePending) {
    ScrollMsg(pendingMqttMessage, 25);
    PublishMqttState(pendingMqttMessage);
    mqttMessagePending = false;
  }

  today = nowTime.substring(4, 10);

  // Get bright after 7am but dim after 10pm

  if ((nowTime.substring(10, 13).toInt() > 21) || (nowTime.substring(10, 13).toInt() < 7)) {
    P.setIntensity(0);  // range 0 - 15
  } else {
    P.setIntensity(15);  // range 0 - 15
  }

  if (nowTime.substring(10, 16) != oldNowTime) {
    PrintMsg(nowTime.substring(10, 16));
    oldNowTime = nowTime.substring(10, 16);
  }

  if ((bigcount % 20) == 0)  // Display Weather Data every 20 seconds (was 60)
  {

    // Added for Version 10.4
    while (!bmp280.getMeasurements(temperature, pressure, altitude))  // Check if the measurement is complete
    {
      delay(1);
    }
    String tempString = "In " + String(int(temperature * TempScale)) + "C";  //adjusted BMP280 temperature (was 0.9)
    //PrintMsg(tempString);
    ScrollMsg(StoredWeatherDescription, 25);  //Display Weather Conditions
    PrintMsg(tempString);                     //Display the temperature again as I always miss it!
    //PrintMsg(nowTime.substring(10,16));                          //Display Time

    //Put some code to display extra messages here

    int count = 0;

    while (count < NoOfMessages) {
      if (today == MessageEntries[count].date) {
        ScrollMsg(MessageEntries[count].message, 20);
      }
      // The below just for testing so you can see that the messages are correct
      //Serial.print(MessageEntries[count].date);
      //Serial.print(" - ");
      //Serial.println(MessageEntries[count].message);
      count++;
    }

    PrintMsg(today);  // Show todays date

    PrintMsg(nowTime.substring(10, 16));  //Display Time  May be able to start at 11 (might be a space)
  }

  if (bigcount == 900)  //900 = every 15 mins - update weather ONLY
  {
    ScrollMsg("15 minute weather update", 30);
    Serial.println("15 minute weather update");
    connectWifi();
    //SetTime();
    GetWeather();
    PrintMsg(nowTime.substring(10, 16));  //Display Time  May be able to start at 11 (might be a space)
    bigcount = 0;
  }

  if (bigcount == 43200)  //3600 = 1 hour - update time ONLY 43200 = 12 hours
  {
    ScrollMsg("12 Hourly time update", 30);
    Serial.println("12 Hourly time update");
    connectWifi();
    SetTime();
    //GetWeather();
    PrintMsg(nowTime.substring(10, 16));  //Display Time  May be able to start at 11 (might be a space)
    bigcount = 0;
  }

  if ((bigcount % 60) == 0) {
    time_t nowUtc = time(nullptr);
    ApplyDstIfNeeded(nowUtc);
  }

  time_t now = time(nullptr);
  nowTime = ctime(&now);
  //Serial.print("NOW TIME = ");
  //Serial.println(nowTime);
  bigcount++;
  delay(1000);
}
