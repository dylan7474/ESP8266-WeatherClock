// Ver 10.9.10 (29/08/22)
// Fixed birthday messages, added a few more and added 10 day reminders
//
// The fix for wifi on the ESP8266 is to use Wireless Mode : Legacy (01/06/24)

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
// now gets the time twice - once to get the date and set the dst state, second to get the right time based on new dst
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
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

// Added for Version 10.4
#include <BMP280_DEV.h>
float temperature, pressure, altitude;  // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280;

// Added for Version 10.7
// #include <HTTPClient.h>  //from esp32...doesn't seem to work
#include <ESP8266HTTPClient.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14
#define DATA_PIN 13
#define CS_PIN 15

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String Version = "max72LedNodeMCU_Scroll_Working_v10.9.10";
float TempScale = 0.78;
int timezone = 0;
int dst = 0;  //dst = 0 for GMT , dst = 1 for bst
String nowTime;
String nowTimeShort;
String oldNowTime = "";
String dstMonth;  //This is to be used by the auto clock change routine
int dstDate;
int dstYear;

String StoredWeatherMainData;
String StoredWeatherDescription;
String StoredWeatherTemperature;

const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
const char* WIFI_SSID = "CHIGLEY";
const char* WIFI_PASSWORD = "CrimbleCrumble20$";

int bigcount = 0;

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

int const NoOfMessages = 30;
String MessageDate[NoOfMessages] = { "Jan  1", "Jan 7", "Jan 17", "Jan 25", "Feb 12", "Feb 14", "Apr  1", "Apr  15", "May 12", "May 22", "May 28", "Jun 2", "Jun 7", "Jun 12", "Jun 20", "Jun 30", "Jul 17", "Jul 27", "Aug 18", "Aug 28", "Oct 31", "Nov 2", "Nov 12", "Nov 29", "Dec  9", "Dec 15", "Dec 24", "Dec 25", "Dec 26", "Dec 31" };
String Message[NoOfMessages] = { "New Years Day", "10 days to Manuela\'s Bday", "Happy Birthday Manuela", "Burns Night", "Chinese New Year", "Valentines Day", "April Fools Day", "Good Friday", "10 days to Dylan\'s Bday", "Happy Birthday Dylan", "10 days to Maureen\'s Bday", "10 days to Mam\'s Bday", "Happy Birthday Maureen", "Happy Birthday Mam", "10 days to Mel\'s Bday", "Happy Birthday Mel", "10 days to Yvonne\'s Bday", "Happy Birthday Yvonne", "10 days to Dad\'s Bday", "Happy Birthday Dad", "Happy Halloween", "10 days to Harrys\'s Bday", "Happy Birthday Harry", "10 days to Robert\'s Bday", "Happy Birthday Robert", "10 days to Xmas", "Christmas Eve", "Christmas Day", "Boxing Day", "New Years Eve" };
String today;

void connectWifi() {
  Serial.print("Connecting to WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("*");
    ScrollMsg("Connecting to wifi", 15);
    delay(1000);
  }
  Serial.println("\nCONNECTED");
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
  Serial.print("DST = ");
  Serial.println(dst);

  //HACK NOW TIME
  //Serial.print("HACKING NOW TIME to ");
  //nowTime = "Mon Mar 30 22:52:59 2021";
  //Serial.println(nowTime);

  dstMonth = nowTime.substring(4, 7);
  dstDate = nowTime.substring(8, 10).toInt();
  dstYear = nowTime.substring(20, 25).toInt();
  //today=nowTime.substring(4,10);        // Moved this line to the main loop

  if (dstMonth.equals("Nov")) {
    Serial.println("It's winter Nov");
    dst = 0;
  }

  if (dstMonth == "Dec") {
    Serial.println("It's winter Dec");
    dst = 0;
  }

  if (dstMonth == "Jan") {
    Serial.println("It's winter Jan");
    dst = 0;
  }

  if (dstMonth == "Feb") {
    Serial.println("It's winter Feb");
    dst = 0;
  }

  if (dstMonth == "Apr") {
    Serial.println("It's summer Apr");
    dst = 1;
  }


  if (dstMonth == "May") {
    Serial.println("It's summer May");
    dst = 1;
  }


  if (dstMonth == "Jun") {
    Serial.println("It's summer Jun");
    dst = 1;
  }


  if (dstMonth == "Jul") {
    Serial.println("It's summer Jul");
    dst = 1;
  }

  if (dstMonth == "Aug") {
    Serial.println("It's summer Aug");
    dst = 1;
  }

  if (dstMonth == "Sep") {
    Serial.println("It's summer Sep");
    dst = 1;
  }

  // In these clock change months, night need to include a time check to make sure happens at 2am - tricky
  // Probably best to just leave an see what it does!
  // Could simulate maybe

  if (dstMonth == "Oct" && dstDate < oct[dstYear - 2021]) {
    Serial.println("It's summer Oct");
    dst = 1;
  }

  if (dstMonth == "Oct" && dstDate >= oct[dstYear - 2021]) {
    Serial.println("It's winter Oct");
    dst = 0;
  }

  if (dstMonth == "Mar" && dstDate < mar[dstYear - 2021]) {
    Serial.println("It's winter Mar");
    dst = 0;  //changed 27/03/22
  }

  if (dstMonth == "Mar" && dstDate >= mar[dstYear - 2021]) {
    Serial.println("It's summer Mar");
    dst = 1;  //changed 27/03/22
  }
}

void GetWeather() {

  // Connection string should be http://api.openweathermap.org/data/2.5/weather?lat=54.54&lon=-1.08&APPID=c848eb234e72b8e3808ab65d17d05231
  // Should output something like
  // {"coord":{"lon":-1.08,"lat":54.54},"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"base":"stations","main":{"temp":283.91,"pressure":994,"humidity":71,"temp_min":282.15,"temp_max":285.93},"visibility":10000,"wind":{"speed":3.6,"deg":240},"clouds":{"all":20},"dt":1571396447,"sys":{"type":1,"id":1428,"message":0.0093,"country":"GB","sunrise":1571380671,"sunset":1571418055},"timezone":3600,"id":7291323,"name":"Guisborough","cod":200}
  // json parsed from https://arduinojson.org/v6/assistant/

  Serial.println("Getting Weather");
  ScrollMsg("Getting Weather", 15);
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=54.54&lon=-1.08&units=metric&APPID=c848eb234e72b8e3808ab65d17d05231";
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

  // The below just for testing so you can see that the messages are correct
  int count = 0;
  while (count < NoOfMessages) {
    if (today == MessageDate[count]) {
      ScrollMsg(Message[count], 20);
    }
    Serial.print(MessageDate[count]);
    Serial.print(" - ");
    Serial.println(Message[count]);
    count++;
  }

  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

  connectWifi();
  SetTime();  //run once to get date to determine dst state
  SetTime();  //run again to set time with correct dst
  GetWeather();
  WiFi.disconnect();

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
      if (today == MessageDate[count]) {
        ScrollMsg(Message[count], 20);
      }
      // The below just for testing so you can see that the messages are correct
      //Serial.print(MessageDate[count]);
      //Serial.print(" - ");
      //Serial.println(Message[count]);
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
    WiFi.disconnect();
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
    WiFi.disconnect();
    PrintMsg(nowTime.substring(10, 16));  //Display Time  May be able to start at 11 (might be a space)
    bigcount = 0;
  }

  time_t now = time(nullptr);
  nowTime = ctime(&now);
  //Serial.print("NOW TIME = ");
  //Serial.println(nowTime);
  bigcount++;
  delay(1000);
}
