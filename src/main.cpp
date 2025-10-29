#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include <DHT.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
// PubSubClient (ThingsBoard) removed

// ===== Pin Definitions =====
// Input Toggle Switches 
#define SW1_PIN 34
#define SW2_PIN 35
#define SW3_PIN 32

// Isolated Inputs
#define ISOIN1_PIN 33
#define ISOIN2_PIN 27

// Output Pins
#define LED_PIN 2
#define RL1_PIN 17  // RELAY1
#define RL2_PIN 16  // RELAY2
#define RL3_PIN 4   // RELAY3

// Auxiliary OUTPUTs TTL 3.3V
#define AUX1_PIN 12
#define AUX2_PIN 13
#define AUX3_PIN 14
#define AUX4_PIN 15

// DHT22 Temperature & Humidity Sensor
#define DHT_PIN 13        // DHT22 data pin
#define DHT_TYPE DHT22    // DHT22 (AM2302)

// ===== WiFi Configuration =====
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

// ===== Serial Configuration =====
// All status data is output to Serial monitor at 115200 baud for debugging

// ===== NTP Configuration =====
const char* NTP_SERVER1 = "pool.ntp.org";
const char* NTP_SERVER2 = "time.nist.gov";
const char* NTP_SERVER3 = "time.google.com";
const long GMT_OFFSET_SEC = 7 * 3600;  // GMT+7 for Bangkok
const int DAYLIGHT_OFFSET_SEC = 0;     // Thailand doesn't use DST

// ===== Weather Configuration =====
const char* OPENWEATHERMAP_API_KEY = "5285b3436c86bdab46069027fd961d09";
const char* WEATHER_CITY = "Tha%20Sala,Nakhon%20Si%20Thammarat,TH";
const char* OWM_HOST = "api.openweathermap.org";

// ===== Air Quality Configuration =====
const float NAKHON_SI_THAMMARAT_LAT = 8.4304;   // Latitude
const float NAKHON_SI_THAMMARAT_LON = 99.9631;  // Longitude

// ===== OLED Display Configuration =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT22 sensor initialization
DHT dht(DHT_PIN, DHT_TYPE);

// WiFi client for HTTPS calls (weather, air quality APIs)
WiFiClient espClient;

// ===== Web Server Configuration =====
AsyncWebServer server(80);
const char* http_username = "admin";
const char* http_password = "password";

// ===== Timer Variables =====
struct RelayTimer {
  bool active;
  int relay_num;
  unsigned long end_time;
};
RelayTimer relay_timer = {false, 0, 0};

// ===== Event Log Variables =====
const int MAX_LOGS = 50;
String event_logs[MAX_LOGS];
int log_index = 0;

// ===== Global Variables for Test Cycle =====
int current_test_step = 0;
const int total_test_steps = 7;
unsigned long test_step_timer = 0;
const unsigned long test_interval_ms = 1500;
bool auto_mode = true;  // Auto mode = cycling test, Manual mode = switch control

// ===== Global Variables for Weather =====
unsigned long weather_update_timer = 0;
const unsigned long weather_update_interval_ms = 600000;  // Update every 10 minutes
bool weather_data_valid = false;
String weather_status = "Not fetched";  // Track weather fetch status

// Data structure to hold current weather
struct CurrentWeather {
  String city_name;
  String description;
  int temp_current;
  int humidity;
  String icon_code;
};

CurrentWeather current_weather;

// ===== Global Variables for Air Quality =====
unsigned long aqi_update_timer = 0;
const unsigned long aqi_update_interval_ms = 600000;  // Update every 10 minutes
bool aqi_data_valid = false;
String aqi_status = "Not fetched";  // Track AQI fetch status

// Data structure to hold air quality data
struct AirQuality {
  int aqi;           // Air Quality Index (1-5)
  float pm2_5;       // PM2.5 value
  float pm10;        // PM10 value
  String quality;    // Text description (Good, Fair, etc.)
};

AirQuality air_quality;

// ===== Global Variables for DHT22 Sensor =====
unsigned long dht_update_timer = 0;
const unsigned long dht_update_interval_ms = 5000;  // Update every 5 seconds
bool dht_data_valid = false;
float dht_temperature = 0.0;
float dht_humidity = 0.0;
String dht_status = "Not read";

// ===== Status Tracking for Display Optimization =====
String last_test_item = "";
String last_wifi_status = "";
bool oled_available = false;  // Track OLED availability

// ===== WiFi Status Variables =====
String wifi_status = "Disconnected";
String wifi_ip = "N/A";
bool time_synced = false;

// ===== Debug Print Variables =====
unsigned long last_debug_print = 0;
const unsigned long debug_print_interval = 10000;  // Debug print every 10 seconds

// ===== Switch debounce / toggle globals =====
const unsigned long SWITCH_DEBOUNCE_MS = 50; // debounce window in milliseconds
unsigned long lastSwitchMillis[3] = {0, 0, 0};
bool lastSwitchState[3] = {HIGH, HIGH, HIGH}; // INPUT_PULLUP: HIGH = released

// ===== Function Declarations =====
void updateOledDisplay(String test_item, String wifi_info);
void turnOffAllOutputs();
void runTestCycle();
String readInputStates();
void connectWiFi();
String getWiFiStatus();
void syncNTPTime();
String getCurrentTime();
void fetchWeatherData();
void fetchAirQualityData();
String getAQIQuality(int aqi);
void readDHT22Data();
void setRelayState(int relay, bool state);
void setAuxState(int aux, bool state);
void handleSwitches();
void setupWebServer();
void addLog(String message);
void handleTimers();

// ===== Setup Function =====
void setup() {
  // 1. Initialize Serial for debugging (optional)
  Serial.begin(115200);
  
  // 2. Initialize all output pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RL1_PIN, OUTPUT);
  pinMode(RL2_PIN, OUTPUT);
  pinMode(RL3_PIN, OUTPUT);
  pinMode(AUX1_PIN, OUTPUT);
  pinMode(AUX2_PIN, OUTPUT);
  pinMode(AUX3_PIN, OUTPUT);
  pinMode(AUX4_PIN, OUTPUT);
  
  // Set all outputs to initial state
  // Note: All Relays (RL1, RL2, RL3) are Active Low (OFF when HIGH)
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RL1_PIN, HIGH);  // Active Low - OFF
  digitalWrite(RL2_PIN, HIGH);  // Active Low - OFF
  digitalWrite(RL3_PIN, HIGH);  // Active Low - OFF
  digitalWrite(AUX1_PIN, LOW);
  digitalWrite(AUX2_PIN, LOW);
  digitalWrite(AUX3_PIN, LOW);
  digitalWrite(AUX4_PIN, LOW);
  
  // 3. Initialize all input pins with PULLUP
  // Note: All inputs (SW1-3, ISOIN1-2) are Active Low
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(ISOIN1_PIN, INPUT_PULLUP);
  pinMode(ISOIN2_PIN, INPUT_PULLUP);
  
  // 4. Initialize DHT22 sensor
  dht.begin();
  Serial.println("DHT22 sensor initialized");
  
  // 5. Initialize OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // OLED not available - continue without display
    oled_available = false;
    Serial.println("OLED not detected - continuing without display");
    // Blink LED 3 times to indicate OLED not found
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  } else {
    // Display successful - show welcome message
    oled_available = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("ThaiTech");
    display.println("Zone V2.0");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println("Board--Ready");
    display.display();
    delay(2000);
  }
  
  // 6. Connect to WiFi
  connectWiFi();
  
  // 7. Initialize LittleFS
  if(!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
  } else {
    Serial.println("LittleFS Mounted Successfully");
  }
  
  // 8. Setup Web Server
  setupWebServer();
  Serial.println("Web Server Started");
  Serial.print("Access Dashboard at: http://");
  Serial.println(WiFi.localIP());
  
  // 9. Serial debug setup
  Serial.println("Serial debugging enabled (115200 baud)");
  
  // 10. Sync Time with NTP Server and initialize data
  if(WiFi.status() == WL_CONNECTED) {
    syncNTPTime();
    fetchWeatherData();       // Fetch initial weather data
    fetchAirQualityData();    // Fetch initial air quality data
  }
  
  // Read initial DHT22 data
  readDHT22Data();
  
  // Initialize timers
  test_step_timer = millis();
  weather_update_timer = millis();
  aqi_update_timer = millis();
  dht_update_timer = millis();
}

// ===== Main Loop =====
void loop() {
  static String current_test_name = "Initializing";
  
  // 0. Startup sequence: Turn ON relays 1-3 for 3 seconds, then OFF for 3 seconds
  static bool startup_sequence_done = false;
  static unsigned long startup_timer = 0;
  static int startup_phase = 0; // 0=start, 1=relays ON, 2=relays OFF, 3=done
  
  if (!startup_sequence_done) {
    if (startup_phase == 0) {
      // Start sequence - turn ON all relays (Active Low: LOW = ON)
      digitalWrite(RL1_PIN, LOW);
      digitalWrite(RL2_PIN, LOW);
      digitalWrite(RL3_PIN, LOW);
      startup_timer = millis();
      startup_phase = 1;
      current_test_name = "STARTUP: ON";
      Serial.println("Startup: Relays 1-3 ON for 3 seconds");
    } else if (startup_phase == 1 && (millis() - startup_timer >= 3000)) {
      // After 3 seconds ON, turn OFF all relays (Active Low: HIGH = OFF)
      digitalWrite(RL1_PIN, HIGH);
      digitalWrite(RL2_PIN, HIGH);
      digitalWrite(RL3_PIN, HIGH);
      startup_timer = millis();
      startup_phase = 2;
      current_test_name = "STARTUP: OFF";
      Serial.println("Startup: Relays 1-3 OFF for 3 seconds");
    } else if (startup_phase == 2 && (millis() - startup_timer >= 3000)) {
      // After 3 seconds OFF, startup sequence complete
      startup_sequence_done = true;
      startup_phase = 3;
      current_test_name = "Initializing";
      Serial.println("Startup sequence completed - normal operation begins");
    }
    
    // During startup sequence, only update OLED and return early
    String wifi_info = getWiFiStatus();
    updateOledDisplay(current_test_name, wifi_info);
    delay(10);
    return;
  }
  
  // 1. No external connection maintenance needed

  // 1.5 Handle physical switches (debounced toggles for RELAY1-3)
  handleSwitches();
  
  // 1.6 Handle timers
  handleTimers();
  
  // 2. Automated Output Test Cycle (only in auto mode)
  if (auto_mode) {
    runTestCycle();
    
    // Update test name based on CURRENT active output (before incrementing)
    // This ensures the display matches what's actually ON
    switch(current_test_step) {
      case 0: current_test_name = "-> RELAY 1"; break;
      case 1: current_test_name = "-> RELAY 2"; break;
      case 2: current_test_name = "-> RELAY 3"; break;
      case 3: current_test_name = "-> AUX 1"; break;
      case 4: current_test_name = "-> AUX 2"; break;
      case 5: current_test_name = "-> AUX 3"; break;
      case 6: current_test_name = "-> AUX 4"; break;
      default: current_test_name = "-> ALL OFF"; break;
    }
  } else {
    current_test_name = "MANUAL MODE";
  }
  
  // 3. Update WiFi Status
  String wifi_info = getWiFiStatus();
  
  // 4. Update Weather Data periodically
  if (millis() - weather_update_timer >= weather_update_interval_ms) {
    weather_update_timer = millis();
    fetchWeatherData();
  }
  
  // 5. Update Air Quality Data periodically
  if (millis() - aqi_update_timer >= aqi_update_interval_ms) {
    aqi_update_timer = millis();
    fetchAirQualityData();
  }
  
  // 6. Update DHT22 Data periodically
  if (millis() - dht_update_timer >= dht_update_interval_ms) {
    dht_update_timer = millis();
    readDHT22Data();
  }
  
  // 7. Update OLED Display
  updateOledDisplay(current_test_name, wifi_info);
  
  // 8. Small delay for stability
  delay(10);
}

// ===== Helper Function: Run Test Cycle =====
void runTestCycle() {
  static bool outputs_set = false;
  
  // Check if it's time to change to next test step
  if(millis() - test_step_timer >= test_interval_ms) {
    test_step_timer = millis();
    outputs_set = false;  // Flag to set outputs on next loop iteration
    
    // Move to next test step
    current_test_step++;
    if(current_test_step >= total_test_steps) {
      current_test_step = 0;
    }
  }
  
  // Set outputs immediately when step changes
  if(!outputs_set) {
    outputs_set = true;
    
    // Turn off all outputs first
    turnOffAllOutputs();
    
    // Toggle LED to show activity
    static bool led_state = false;
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    
    // Turn on the specific output for current test step IMMEDIATELY
    // Note: All Relays (RL1, RL2, RL3) are Active Low (ON when LOW)
    switch(current_test_step) {
      case 0: digitalWrite(RL1_PIN, LOW); break;   // Active Low - ON
      case 1: digitalWrite(RL2_PIN, LOW); break;   // Active Low - ON
      case 2: digitalWrite(RL3_PIN, LOW); break;   // Active Low - ON
      case 3: digitalWrite(AUX1_PIN, HIGH); break;
      case 4: digitalWrite(AUX2_PIN, HIGH); break;
      case 5: digitalWrite(AUX3_PIN, HIGH); break;
      case 6: digitalWrite(AUX4_PIN, HIGH); break;
    }
  }
}

// ===== Helper Function: Turn Off All Outputs =====
void turnOffAllOutputs() {
  // All Relays (RL1, RL2, RL3) are Active Low - OFF when HIGH
  digitalWrite(RL1_PIN, HIGH);  // Active Low - OFF
  digitalWrite(RL2_PIN, HIGH);  // Active Low - OFF
  digitalWrite(RL3_PIN, HIGH);  // Active Low - OFF
  digitalWrite(AUX1_PIN, LOW);
  digitalWrite(AUX2_PIN, LOW);
  digitalWrite(AUX3_PIN, LOW);
  digitalWrite(AUX4_PIN, LOW);
}

// ===== Helper Function: Read Input States =====
String readInputStates() {
  // Note: All inputs are Active Low (pressed/active = LOW, show as "1")
  String result = "SW:";
  result += digitalRead(SW1_PIN) ? "0" : "1";   // Active Low - inverted
  result += digitalRead(SW2_PIN) ? "0" : "1";   // Active Low - inverted
  result += digitalRead(SW3_PIN) ? "0" : "1";   // Active Low - inverted
  result += " ISO:";
  result += digitalRead(ISOIN1_PIN) ? "0" : "1"; // Active Low - inverted
  result += digitalRead(ISOIN2_PIN) ? "0" : "1"; // Active Low - inverted
  return result;
}

// ===== Helper Function: Connect to WiFi =====
void connectWiFi() {
  Serial.println("WiFi Connecting...");
  Serial.println(WIFI_SSID);
  
  if(oled_available) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("WiFi Connecting...");
    display.setCursor(0, 12);
    display.println(WIFI_SSID);
    display.display();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    if(oled_available) {
      display.print(".");
      display.display();
    }
    attempts++;
  }
  
  Serial.println();
  
  if(oled_available) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_status = "Connected";
    wifi_ip = WiFi.localIP().toString();
    
    Serial.println("WiFi Connected!");
    Serial.print("IP: ");
    Serial.println(wifi_ip);
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    if(oled_available) {
      display.println("WiFi Connected!");
      display.setCursor(0, 12);
      display.print("IP: ");
      display.println(wifi_ip);
      display.setCursor(0, 24);
      display.print("RSSI: ");
      display.print(WiFi.RSSI());
      display.println(" dBm");
    }
  } else {
    wifi_status = "Failed";
    wifi_ip = "N/A";
    
    Serial.println("WiFi Failed!");
    
    if(oled_available) {
      display.println("WiFi Failed!");
      display.setCursor(0, 12);
      display.println("Check credentials");
    }
  }
  
  if(oled_available) {
    display.display();
    delay(3000);
  } else {
    delay(1000);
  }
}

// ===== Helper Function: Sync NTP Time =====
void syncNTPTime() {
  Serial.println("Syncing Time...");
  Serial.println("NTP Server");
  
  if(oled_available) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Syncing Time...");
    display.setCursor(0, 12);
    display.println("NTP Server");
    display.display();
  }
  
  // Configure NTP with Bangkok timezone (GMT+7)
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  
  // Wait for time sync (max 10 seconds)
  int timeout = 0;
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo) && timeout < 20) {
    delay(500);
    Serial.print(".");
    if(oled_available) {
      display.print(".");
      display.display();
    }
    timeout++;
  }
  
  Serial.println();
  
  if(oled_available) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }
  
  if (getLocalTime(&timeinfo)) {
    time_synced = true;
    
    Serial.println("Time Synced!");
    Serial.printf("Date: %02d/%02d/%04d\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.println("Bangkok (GMT+7)");
    
    if(oled_available) {
      display.println("Time Synced!");
      display.setCursor(0, 12);
      display.print("Date: ");
      display.printf("%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      display.setCursor(0, 24);
      display.print("Time: ");
      display.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      display.setCursor(0, 36);
      display.println("Bangkok (GMT+7)");
    }
  } else {
    time_synced = false;
    
    Serial.println("Time Sync Failed!");
    
    if(oled_available) {
      display.println("Time Sync Failed!");
      display.setCursor(0, 12);
      display.println("Using system time");
    }
  }
  
  if(oled_available) {
    display.display();
    delay(3000);
  } else {
    delay(1000);
  }
}

// ===== Helper Function: Get Current Time =====
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "N/A";
  }
  
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  return String(timeStr);
}

// ===== Helper Function: Get WiFi Status =====
String getWiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    return "WiFi:" + String(rssi) + "dBm";
  } else {
    return "WiFi:DC";  // Disconnected
  }
}

// ===== Helper Function: Fetch Weather Data =====
void fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    weather_status = "No WiFi";
    weather_data_valid = false;
    return;
  }

  weather_status = "Connecting...";
  
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(OWM_HOST, 443)) {
    weather_status = "Connect fail";
    weather_data_valid = false;
    return;
  }

  weather_status = "Requesting...";
  
  // Using Current Weather API with city name
  String url = String("/data/2.5/weather?q=") + WEATHER_CITY +
               "&appid=" + OPENWEATHERMAP_API_KEY + 
               "&units=metric&lang=th";

  // Send HTTP GET request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + OWM_HOST + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "Accept: application/json\r\n" +
               "Connection: close\r\n\r\n");

  // Wait for response with timeout
  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > 10000) {
      weather_status = "Timeout";
      weather_data_valid = false;
      client.stop();
      return;
    }
    delay(10);
  }

  weather_status = "Reading...";

  // Read HTTP status line
  String status_line = client.readStringUntil('\n');
  if (status_line.indexOf("200") == -1) {
    weather_status = "HTTP " + status_line.substring(9, 12);
    weather_data_valid = false;
    client.stop();
    return;
  }

  // Skip remaining HTTP headers
  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) {
      break;  // Empty line means headers are done
    }
  }

  weather_status = "Parsing...";
  
  // Now read the JSON body
  String json_body = "";
  while (client.available()) {
    json_body += client.readString();
  }
  
  client.stop();

  if (json_body.length() == 0) {
    weather_status = "Empty resp";
    weather_data_valid = false;
    return;
  }

  // Use ArduinoJson to parse the response
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, json_body);

  if (error) {
    weather_status = "Parse err";
    weather_data_valid = false;
    return;
  }

  // Check if we got valid data
  if (!doc.containsKey("main") || !doc.containsKey("weather")) {
    weather_status = "No data";
    weather_data_valid = false;
    return;
  }

  // Extract current weather data
  current_weather.city_name = doc["name"].as<String>();
  current_weather.temp_current = round(doc["main"]["temp"].as<float>());
  current_weather.humidity = doc["main"]["humidity"].as<int>();
  current_weather.description = doc["weather"][0]["description"].as<String>();
  current_weather.icon_code = doc["weather"][0]["icon"].as<String>();

  weather_data_valid = true;
  weather_status = "OK";
}

// ===== Helper Function: Fetch Air Quality Data =====
void fetchAirQualityData() {
  if (WiFi.status() != WL_CONNECTED) {
    aqi_status = "No WiFi";
    aqi_data_valid = false;
    return;
  }

  aqi_status = "Connecting...";
  
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(OWM_HOST, 443)) {
    aqi_status = "Connect fail";
    aqi_data_valid = false;
    return;
  }

  aqi_status = "Requesting...";
  
  // Using Air Pollution API with coordinates
  String url = String("/data/2.5/air_pollution?lat=") + String(NAKHON_SI_THAMMARAT_LAT, 4) +
               "&lon=" + String(NAKHON_SI_THAMMARAT_LON, 4) +
               "&appid=" + OPENWEATHERMAP_API_KEY;

  // Send HTTP GET request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + OWM_HOST + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "Accept: application/json\r\n" +
               "Connection: close\r\n\r\n");

  // Wait for response with timeout
  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > 10000) {
      aqi_status = "Timeout";
      aqi_data_valid = false;
      client.stop();
      return;
    }
    delay(10);
  }

  aqi_status = "Reading...";

  // Read HTTP status line
  String status_line = client.readStringUntil('\n');
  if (status_line.indexOf("200") == -1) {
    aqi_status = "HTTP " + status_line.substring(9, 12);
    aqi_data_valid = false;
    client.stop();
    return;
  }

  // Skip remaining HTTP headers
  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) {
      break;  // Empty line means headers are done
    }
  }

  aqi_status = "Parsing...";
  
  // Now read the JSON body
  String json_body = "";
  while (client.available()) {
    json_body += client.readString();
  }
  
  client.stop();

  if (json_body.length() == 0) {
    aqi_status = "Empty resp";
    aqi_data_valid = false;
    return;
  }

  // Use ArduinoJson to parse the response
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, json_body);

  if (error) {
    aqi_status = "Parse err";
    aqi_data_valid = false;
    return;
  }

  // Check if we got valid data
  if (!doc.containsKey("list") || doc["list"].size() == 0) {
    aqi_status = "No data";
    aqi_data_valid = false;
    return;
  }

  // Extract air quality data
  air_quality.aqi = doc["list"][0]["main"]["aqi"].as<int>();
  air_quality.pm2_5 = doc["list"][0]["components"]["pm2_5"].as<float>();
  air_quality.pm10 = doc["list"][0]["components"]["pm10"].as<float>();
  air_quality.quality = getAQIQuality(air_quality.aqi);

  aqi_data_valid = true;
  aqi_status = "OK";
}

// ===== Helper Function: Get AQI Quality Description =====
String getAQIQuality(int aqi) {
  // Based on OpenWeatherMap AQI scale (1-5)
  switch(aqi) {
    case 1: return "Good";
    case 2: return "Fair";
    case 3: return "Moderate";
    case 4: return "Poor";
    case 5: return "VeryPoor";
    default: return "Unknown";
  }
}

// ===== Helper Function: Update OLED Display =====
void updateOledDisplay(String test_item, String wifi_info) {
  static unsigned long last_update = 0;
  static String last_time_str = "";
  String current_time_str = time_synced ? getCurrentTime() : "";
  
  // Force update every second if time is synced, or when data changes
  bool force_update = (time_synced && (millis() - last_update >= 1000));
  bool data_changed = (test_item != last_test_item || 
                       wifi_info != last_wifi_status ||
                       current_time_str != last_time_str);
  
  // Print debug info every 10 seconds (ไม่บ่อยเกินไป)
  if (millis() - last_debug_print >= debug_print_interval) {
    last_debug_print = millis();
    
    Serial.println("\n╔════════════════════════════════════════════════╗");
    Serial.println("║         ESP32 BOARD STATUS REPORT              ║");
    Serial.println("╠════════════════════════════════════════════════╣");
    
    // System Info
    Serial.print("║ Mode: ");
    Serial.print(auto_mode ? "AUTO" : "MANUAL");
    Serial.print(" | Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" sec");
    
    // WiFi Info
    Serial.print("║ WiFi: ");
    Serial.print(WiFi.RSSI());
    Serial.print(" dBm | IP: ");
    Serial.println(WiFi.localIP());
    
    // System Status
    Serial.print("║ Debug: ");
    Serial.println("Serial Monitor Only");    Serial.println("╠════════════════════════════════════════════════╣");
    
    // Output States
    Serial.println("║ OUTPUT STATES:");
    Serial.print("║   Relay 1: ");
    Serial.print(!digitalRead(RL1_PIN) ? "ON " : "OFF");
    Serial.print(" | Relay 2: ");
    Serial.print(!digitalRead(RL2_PIN) ? "ON " : "OFF");
    Serial.print(" | Relay 3: ");
    Serial.println(!digitalRead(RL3_PIN) ? "ON " : "OFF");
    
    Serial.print("║   AUX 1: ");
    Serial.print(digitalRead(AUX1_PIN) ? "ON " : "OFF");
    Serial.print(" | AUX 2: ");
    Serial.print(digitalRead(AUX2_PIN) ? "ON " : "OFF");
    Serial.print(" | AUX 3: ");
    Serial.print(digitalRead(AUX3_PIN) ? "ON " : "OFF");
    Serial.print(" | AUX 4: ");
    Serial.println(digitalRead(AUX4_PIN) ? "ON " : "OFF");
    
    // Input States
    Serial.println("║ INPUT STATES:");
    Serial.print("║   SW1: ");
    Serial.print(!digitalRead(SW1_PIN) ? "PRESSED" : "RELEASED");
    Serial.print(" | SW2: ");
    Serial.print(!digitalRead(SW2_PIN) ? "PRESSED" : "RELEASED");
    Serial.print(" | SW3: ");
    Serial.println(!digitalRead(SW3_PIN) ? "PRESSED" : "RELEASED");
    
    Serial.print("║   ISO1: ");
    Serial.print(!digitalRead(ISOIN1_PIN) ? "ACTIVE" : "INACTIVE");
    Serial.print(" | ISO2: ");
    Serial.println(!digitalRead(ISOIN2_PIN) ? "ACTIVE" : "INACTIVE");
    
    Serial.println("╠════════════════════════════════════════════════╣");
    
    // Weather Data
    Serial.println("║ WEATHER DATA:");
    if (weather_data_valid) {
      Serial.print("║   Temperature: ");
      Serial.print(current_weather.temp_current);
      Serial.print("°C | Humidity: ");
      Serial.print(current_weather.humidity);
      Serial.println("%");
      Serial.print("║   City: ");
      Serial.print(current_weather.city_name);
      Serial.print(" | ");
      Serial.println(current_weather.description);
    } else {
      Serial.print("║   Status: ");
      Serial.print(weather_status);
      Serial.println(" (Waiting for data...)");
    }
    
    // Air Quality Data
    Serial.println("║ AIR QUALITY DATA:");
    if (aqi_data_valid) {
      Serial.print("║   PM2.5: ");
      Serial.print(air_quality.pm2_5, 1);
      Serial.print(" µg/m³ | PM10: ");
      Serial.print(air_quality.pm10, 1);
      Serial.println(" µg/m³");
      Serial.print("║   AQI: ");
      Serial.print(air_quality.aqi);
      Serial.print(" (");
      Serial.print(air_quality.quality);
      Serial.println(")");
    } else {
      Serial.print("║   Status: ");
      Serial.print(aqi_status);
      Serial.println(" (Waiting for data...)");
    }
    
    // DHT22 Sensor Data
    Serial.println("║ DHT22 SENSOR DATA:");
    if (dht_data_valid) {
      Serial.print("║   Temperature: ");
      Serial.print(dht_temperature, 1);
      Serial.print("°C | Humidity: ");
      Serial.print(dht_humidity, 1);
      Serial.println("%");
    } else {
      Serial.print("║   Status: ");
      Serial.print(dht_status);
      Serial.println(" (Sensor error)");
    }
    
    Serial.println("╚════════════════════════════════════════════════╝\n");
  }
  
  // Skip OLED update if not available
  if(!oled_available) {
    return;
  }
  
  if(!force_update && !data_changed) {
    return;  // No change, skip redraw
  }
  
  last_test_item = test_item;
  last_wifi_status = wifi_info;
  last_time_str = current_time_str;
  last_update = millis();
  
  // Clear and redraw display
  display.clearDisplay();
  
  // Line 1: Test Item Name (Large Text)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(test_item);
  
  // Line 2: Separator Line
  display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  
  // Line 3: Input States (Small Text)
  display.setTextSize(1);
  display.setCursor(0, 22);
  String input_states = readInputStates();
  display.println(input_states);
  
  // Line 4: WiFi Status and Time
  display.setCursor(0, 32);
  display.print(wifi_info);
  display.print(" ");
  if(time_synced) {
    display.print(getCurrentTime());
  }
  
  // Line 5: Date, Temperature and Humidity
  display.setCursor(0, 42);
  if(time_synced) {
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
      display.printf("%02d/%02d/%02d", 
                     timeinfo.tm_mday, 
                     timeinfo.tm_mon + 1, 
                     (timeinfo.tm_year + 1900) % 100);
      
      // Add weather data on same line if available
      if(weather_data_valid) {
        display.printf(" %dC H%d%%",
            current_weather.temp_current, 
            current_weather.humidity);
      }
    }
  } else {
    display.print("Time not synced");
  }
  
  // Line 6: Air Quality Data with description
  display.setCursor(0, 52);
  if (aqi_data_valid) {
    // Show PM2.5, AQI and quality description
    display.printf("PM%.0f AQI%d %s",
        air_quality.pm2_5,
        air_quality.aqi,
        air_quality.quality.c_str());
  } else if (weather_data_valid && !time_synced) {
    // Fallback: show weather if no time sync (shouldn't normally happen)
    display.printf("%dC H%d%%",
        current_weather.temp_current, 
        current_weather.humidity);
  } else {
    // Show error status
    display.print("AQI:");
    display.print(aqi_status);
  }
  
  // Update display
  display.display();
}

// ===== Helper Function: Read DHT22 Data =====
void readDHT22Data() {
  // Try to read real sensor data first
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  
  // Check if readings are valid (not NaN)
  if (isnan(temp) || isnan(humid)) {
    // If sensor fails, use simulated values for demonstration
    dht_data_valid = false;
    dht_status = "Sensor error - using simulated data";
    
    // Generate simulated temperature (25-35°C with slight variation)
    static float sim_temp = 28.5;
    sim_temp += (random(-10, 11) / 10.0); // ±1°C variation
    if (sim_temp < 25.0) sim_temp = 25.0;
    if (sim_temp > 35.0) sim_temp = 35.0;
    dht_temperature = sim_temp;
    
    // Generate simulated humidity (40-80% with variation)
    static float sim_humid = 65.0;
    sim_humid += (random(-20, 21) / 10.0); // ±2% variation
    if (sim_humid < 40.0) sim_humid = 40.0;
    if (sim_humid > 80.0) sim_humid = 80.0;
    dht_humidity = sim_humid;
    
    Serial.println("DHT22: Using simulated data (sensor not connected)");
  } else {
    // Real sensor data is valid
    dht_data_valid = true;
    dht_status = "OK";
    dht_temperature = temp;
    dht_humidity = humid;
    
    Serial.print("DHT22: Real data - Temp: ");
    Serial.print(dht_temperature, 1);
    Serial.print("°C, Humidity: ");
    Serial.print(dht_humidity, 1);
    Serial.println("%");
  }
}

// ===== External Integration Functions Removed =====
// ThingsBoard/MQTT integration has been completely removed from this build

// ===== Helper Function: Set Relay State =====
void setRelayState(int relay, bool state) {
  // Relay pins are Active Low: LOW = ON, HIGH = OFF
  int pin = -1;
  switch(relay) {
    case 1: pin = RL1_PIN; break;
    case 2: pin = RL2_PIN; break;
    case 3: pin = RL3_PIN; break;
    default: return;
  }
  
  digitalWrite(pin, state ? LOW : HIGH);  // Inverted because Active Low
  
  Serial.print("Relay ");
  Serial.print(relay);
  Serial.print(" set to ");
  Serial.println(state ? "ON" : "OFF");
}

// ===== Helper Function: Set AUX State =====
void setAuxState(int aux, bool state) {
  int pin = -1;
  switch(aux) {
    case 1: pin = AUX1_PIN; break;
    case 2: pin = AUX2_PIN; break;
    case 3: pin = AUX3_PIN; break;
    case 4: pin = AUX4_PIN; break;
    default: return;
  }
  
  digitalWrite(pin, state ? HIGH : LOW);
  
  Serial.print("AUX ");
  Serial.print(aux);
  Serial.print(" set to ");
  Serial.println(state ? "ON" : "OFF");
}

// ===== Helper Function: Handle Switches (debounced toggle for Relays 1-3) =====
void handleSwitches() {
  // Array of switch pins and corresponding relay pins
  const int swPins[3] = {SW1_PIN, SW2_PIN, SW3_PIN};
  const int rlPins[3] = {RL1_PIN, RL2_PIN, RL3_PIN};

  unsigned long now = millis();

  for (int i = 0; i < 3; i++) {
    bool reading = digitalRead(swPins[i]); // INPUT_PULLUP: HIGH = released, LOW = pressed

    // If stable longer than debounce interval
    if (reading != lastSwitchState[i]) {
      // state changed; reset timer
      lastSwitchMillis[i] = now;
      lastSwitchState[i] = reading;
    } else {
      // state unchanged; check if it's a debounced falling edge (pressed)
      if (now - lastSwitchMillis[i] >= SWITCH_DEBOUNCE_MS) {
        // We consider a press when reading == LOW
        static bool processed[3] = {false,false,false};

        if (reading == LOW && !processed[i]) {
          // Toggle the relay: relays are active LOW (LOW = ON)
          bool currentOn = !digitalRead(rlPins[i]);
          bool newOn = !currentOn;
          digitalWrite(rlPins[i], newOn ? LOW : HIGH);

          Serial.print("SW"); Serial.print(i+1);
          Serial.print(" pressed -> Relay"); Serial.print(i+1);
          Serial.print(" set to "); Serial.println(newOn ? "ON" : "OFF");

          processed[i] = true; // avoid retrigger until release
        }

        // Reset processed flag on release
        if (reading == HIGH) {
          static bool processed_release[3] = {false,false,false};
          if (processed[i]) {
            processed[i] = false;
          }
        }
      }
    }
  }
}

// ===== Web Server Functions =====

// Add log entry
void addLog(String message) {
  struct tm timeinfo;
  char timeStr[20];
  if (getLocalTime(&timeinfo)) {
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    sprintf(timeStr, "%lu", millis() / 1000);
  }
  
  event_logs[log_index] = String(timeStr) + " - " + message;
  log_index = (log_index + 1) % MAX_LOGS;
  
  Serial.print("LOG: ");
  Serial.println(message);
}

// Handle timers
void handleTimers() {
  if (relay_timer.active && millis() >= relay_timer.end_time) {
    // Timer expired - turn off relay
    setRelayState(relay_timer.relay_num, false);
    addLog("Timer completed for Relay " + String(relay_timer.relay_num));
    relay_timer.active = false;
  }
}

// Setup Web Server
void setupWebServer() {
  // Serve static files from LittleFS
  server.serveStatic("/", LittleFS, "/").setDefaultFile("login.html");
  
  // API: Get Status (with authentication)
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    
    DynamicJsonDocument doc(1024);
    
    // System info
    doc["mode"] = auto_mode ? "AUTO" : "MANUAL";
    doc["uptime"] = millis() / 1000;
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["free_heap"] = ESP.getFreeHeap();
    
    // Relay states (Active Low - inverted for clarity)
    doc["relay1"] = !digitalRead(RL1_PIN);
    doc["relay2"] = !digitalRead(RL2_PIN);
    doc["relay3"] = !digitalRead(RL3_PIN);
    
    // AUX states
    doc["aux1"] = digitalRead(AUX1_PIN);
    doc["aux2"] = digitalRead(AUX2_PIN);
    doc["aux3"] = digitalRead(AUX3_PIN);
    doc["aux4"] = digitalRead(AUX4_PIN);
    
    // Input states (Active Low - inverted)
    doc["switch1"] = !digitalRead(SW1_PIN);
    doc["switch2"] = !digitalRead(SW2_PIN);
    doc["switch3"] = !digitalRead(SW3_PIN);
    doc["iso_input1"] = !digitalRead(ISOIN1_PIN);
    doc["iso_input2"] = !digitalRead(ISOIN2_PIN);
    
    // Sensor data
    doc["dht_temperature"] = dht_temperature;
    doc["dht_humidity"] = dht_humidity;
    
    if (weather_data_valid) {
      doc["weather_temp"] = current_weather.temp_current;
      doc["weather_humid"] = current_weather.humidity;
    }
    
    if (aqi_data_valid) {
      doc["aqi"] = air_quality.aqi;
      doc["aqi_quality"] = air_quality.quality;
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // API: Toggle Relay (with authentication)
  server.on("^\\/api\\/relay\\/([1-3])\\/toggle$", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    
    String relayNum = request->pathArg(0);
    int relay = relayNum.toInt();
    
    // Get current state and toggle
    int pin = (relay == 1) ? RL1_PIN : (relay == 2) ? RL2_PIN : RL3_PIN;
    bool currentState = !digitalRead(pin); // Active Low - invert
    bool newState = !currentState;
    
    setRelayState(relay, newState);
    addLog("Web: Relay " + String(relay) + " " + (newState ? "ON" : "OFF"));
    
    DynamicJsonDocument doc(128);
    doc["relay"] = relay;
    doc["state"] = newState;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // API: Toggle Mode (with authentication)
  server.on("/api/mode/toggle", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    
    auto_mode = !auto_mode;
    addLog("Mode changed to " + String(auto_mode ? "AUTO" : "MANUAL"));
    
    DynamicJsonDocument doc(128);
    doc["mode"] = auto_mode ? "AUTO" : "MANUAL";
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // API: Start Timer (with authentication)
  server.on("/api/timer", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    
    // This will be called after body is received
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    // Parse JSON body
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    int relay = doc["relay"];
    int duration = doc["duration"];
    
    if (relay < 1 || relay > 3 || duration < 1) {
      request->send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
      return;
    }
    
    // Start timer
    relay_timer.active = true;
    relay_timer.relay_num = relay;
    relay_timer.end_time = millis() + (duration * 1000);
    
    // Turn on relay
    setRelayState(relay, true);
    addLog("Timer started: Relay " + String(relay) + " for " + String(duration) + "s");
    
    DynamicJsonDocument responseDoc(128);
    responseDoc["success"] = true;
    responseDoc["relay"] = relay;
    responseDoc["duration"] = duration;
    
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
  });
  
  // API: Get Logs (with authentication)
  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    
    DynamicJsonDocument doc(2048);
    JsonArray logs = doc.createNestedArray("logs");
    
    for (int i = 0; i < MAX_LOGS; i++) {
      int idx = (log_index + i) % MAX_LOGS;
      if (event_logs[idx].length() > 0) {
        logs.add(event_logs[idx]);
      }
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Start server
  server.begin();
  addLog("Web Server Started");
}