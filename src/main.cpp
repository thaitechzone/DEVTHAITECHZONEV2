#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include <PubSubClient.h>

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

// ===== WiFi Configuration =====
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

// ===== MQTT Configuration (ThingsBoard) =====
// ThingsBoard Cloud: demo.thingsboard.io (Free tier)
// ThingsBoard PE: thingsboard.cloud
// Local installation: Your server IP
const char* MQTT_BROKER = "demo.thingsboard.io";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "ESP32_ThaiTechZone_V2";

// ThingsBoard Device Access Token (Get from Device Credentials page)
// TODO: Replace with your actual device token from ThingsBoard
const char* MQTT_USER = "37u5JARLNBIgKmTUnmDM";  // Device Access Token
const char* MQTT_PASSWORD = "";  // Leave empty for ThingsBoard

// ThingsBoard MQTT Topics (Standard format)
const char* TB_TOPIC_TELEMETRY = "v1/devices/me/telemetry";        // Send telemetry data
const char* TB_TOPIC_ATTRIBUTES = "v1/devices/me/attributes";      // Send/request attributes
const char* TB_TOPIC_RPC_REQUEST = "v1/devices/me/rpc/request/+";  // Receive RPC commands
const char* TB_TOPIC_RPC_RESPONSE = "v1/devices/me/rpc/response/"; // Send RPC response (+ requestId)

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

// ===== MQTT Client =====
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Set MQTT Buffer Size (default is 256, increase to 1024 for large telemetry)
const int MQTT_BUFFER_SIZE = 1024;

// ===== Global Variables for Test Cycle =====
int current_test_step = 0;
const int total_test_steps = 7;
unsigned long test_step_timer = 0;
const unsigned long test_interval_ms = 1500;
bool auto_mode = true;  // Auto mode = cycling test, Manual mode = MQTT control

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

// ===== Status Tracking for Display Optimization =====
String last_test_item = "";
String last_wifi_status = "";
bool oled_available = false;  // Track OLED availability

// ===== WiFi Status Variables =====
String wifi_status = "Disconnected";
String wifi_ip = "N/A";
bool time_synced = false;

// ===== MQTT Status Variables =====
bool mqtt_connected = false;
unsigned long mqtt_reconnect_timer = 0;
const unsigned long mqtt_reconnect_interval = 5000;
unsigned long mqtt_publish_timer = 0;
const unsigned long mqtt_publish_interval = 10000;  // Publish status every 10 seconds (ลดลง)
unsigned long last_debug_print = 0;
const unsigned long debug_print_interval = 10000;  // Debug print every 10 seconds

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
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishMQTTStatus();
void setRelayState(int relay, bool state);
void setAuxState(int aux, bool state);

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
  
  // 4. Initialize OLED Display
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
  
  // 5. Connect to WiFi
  connectWiFi();
  
  // 6. Setup MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);  // Increase buffer size for large telemetry
  
  Serial.print("MQTT Buffer Size set to: ");
  Serial.print(MQTT_BUFFER_SIZE);
  Serial.println(" bytes");
  
  // 7. Sync Time with NTP Server
  if(WiFi.status() == WL_CONNECTED) {
    syncNTPTime();
    fetchWeatherData();       // Fetch initial weather data
    fetchAirQualityData();    // Fetch initial air quality data
    connectMQTT();            // Connect to MQTT broker
  }
  
  // Initialize timers
  test_step_timer = millis();
  weather_update_timer = millis();
  aqi_update_timer = millis();
  mqtt_publish_timer = millis();
}

// ===== Main Loop =====
void loop() {
  static String current_test_name = "Initializing";
  
  // 1. Maintain MQTT Connection
  if (!mqttClient.connected()) {
    if (millis() - mqtt_reconnect_timer >= mqtt_reconnect_interval) {
      mqtt_reconnect_timer = millis();
      connectMQTT();
    }
  } else {
    mqttClient.loop();
  }
  
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
  
  // 6. Publish MQTT Status periodically
  if (mqttClient.connected() && (millis() - mqtt_publish_timer >= mqtt_publish_interval)) {
    mqtt_publish_timer = millis();
    publishMQTTStatus();
  }
  
  // 7. Update OLED Display
  updateOledDisplay(current_test_name, wifi_info);
  
  // Small delay for stability
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
    
    // MQTT Status
    Serial.print("║ MQTT: ");
    Serial.print(mqttClient.connected() ? "✅ CONNECTED" : "❌ DISCONNECTED");
    Serial.print(" | Broker: ");
    Serial.println(MQTT_BROKER);
    
    Serial.println("╠════════════════════════════════════════════════╣");
    
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

// ===== Helper Function: Connect to MQTT Broker (ThingsBoard) =====
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    mqtt_connected = false;
    return;
  }
  
  Serial.print("Connecting to ThingsBoard: ");
  Serial.println(MQTT_BROKER);
  
  // Attempt to connect with Access Token as username
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    mqtt_connected = true;
    Serial.println("ThingsBoard Connected!");
    
    // Subscribe to RPC (Remote Procedure Call) commands
    mqttClient.subscribe(TB_TOPIC_RPC_REQUEST);
    
    Serial.println("Subscribed to RPC commands");
    
    // Publish initial attributes (device info)
    DynamicJsonDocument doc(256);
    doc["deviceType"] = "ESP32_IoT_Board";
    doc["firmwareVersion"] = "3.0_ThingsBoard";
    doc["model"] = "ThaiTechZone_V2";
    
    String output;
    serializeJson(doc, output);
    mqttClient.publish(TB_TOPIC_ATTRIBUTES, output.c_str());
    
    if(oled_available) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("ThingsBoard");
      display.println("Connected!");
      display.setCursor(0, 20);
      display.println(MQTT_BROKER);
      display.display();
      delay(2000);
    }
  } else {
    mqtt_connected = false;
    Serial.print("ThingsBoard Connection Failed, rc=");
    Serial.println(mqttClient.state());
    Serial.println("Check Access Token!");
  }
}

// ===== Helper Function: MQTT Callback for Incoming Messages (ThingsBoard RPC) =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("ThingsBoard RPC received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Parse JSON message
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("Failed to parse RPC message");
    return;
  }
  
  // Extract request ID from topic (format: v1/devices/me/rpc/request/123)
  String topicStr = String(topic);
  int lastSlash = topicStr.lastIndexOf('/');
  String requestId = topicStr.substring(lastSlash + 1);
  
  // Get RPC method name
  const char* method = doc["method"];
  
  if (method == nullptr) {
    Serial.println("No method specified in RPC");
    return;
  }
  
  Serial.print("RPC Method: ");
  Serial.println(method);
  
  // Response document
  DynamicJsonDocument response(256);
  bool success = false;
  
  // Handle different RPC methods
  if (strcmp(method, "setMode") == 0) {
    // Set Auto/Manual mode
    String mode = doc["params"]["mode"].as<String>();
    if (mode == "auto") {
      auto_mode = true;
      success = true;
      response["mode"] = "auto";
      Serial.println("Switched to AUTO mode");
    } else if (mode == "manual") {
      auto_mode = false;
      turnOffAllOutputs();
      success = true;
      response["mode"] = "manual";
      Serial.println("Switched to MANUAL mode");
    }
  }
  else if (strcmp(method, "setRelay") == 0 && !auto_mode) {
    // Control Relay (only in manual mode)
    int relay = doc["params"]["relay"].as<int>();
    bool state = doc["params"]["state"].as<bool>();
    
    if (relay >= 1 && relay <= 3) {
      setRelayState(relay, state);
      success = true;
      response["relay"] = relay;
      response["state"] = state;
    }
  }
  else if (strcmp(method, "setAux") == 0 && !auto_mode) {
    // Control AUX (only in manual mode)
    int aux = doc["params"]["aux"].as<int>();
    bool state = doc["params"]["state"].as<bool>();
    
    if (aux >= 1 && aux <= 4) {
      setAuxState(aux, state);
      success = true;
      response["aux"] = aux;
      response["state"] = state;
    }
  }
  else if (strcmp(method, "getValue") == 0) {
    // Get current status
    success = true;
    response["mode"] = auto_mode ? "auto" : "manual";
    response["uptime"] = millis() / 1000;
    
    JsonObject relays = response.createNestedObject("relays");
    relays["rl1"] = !digitalRead(RL1_PIN);
    relays["rl2"] = !digitalRead(RL2_PIN);
    relays["rl3"] = !digitalRead(RL3_PIN);
  }
  else if (strcmp(method, "turnOffAll") == 0) {
    // Turn off all outputs
    turnOffAllOutputs();
    success = true;
    response["message"] = "All outputs turned off";
  }
  
  // Send RPC response back to ThingsBoard
  String responseTopic = String(TB_TOPIC_RPC_RESPONSE) + requestId;
  String responseStr;
  serializeJson(response, responseStr);
  mqttClient.publish(responseTopic.c_str(), responseStr.c_str());
  
  Serial.print("RPC Response: ");
  Serial.println(responseStr);
}

// ===== Helper Function: Publish MQTT Status (ThingsBoard Telemetry) =====
void publishMQTTStatus() {
  if (!mqttClient.connected()) {
    Serial.println("⚠️  MQTT not connected - skipping telemetry");
    return;
  }
  
  // Create telemetry JSON document with optimized size
  DynamicJsonDocument doc(512);  // ลดขนาดลง เพราะไม่ส่ง Weather/AQI ตลอดเวลา
  
  // System information (ส่งเสมอ)
  doc["uptime"] = millis() / 1000;
  doc["mode"] = auto_mode ? "auto" : "manual";
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();
  
  // Relay States (Active Low - inverted for clarity)
  doc["relay1"] = !digitalRead(RL1_PIN);
  doc["relay2"] = !digitalRead(RL2_PIN);
  doc["relay3"] = !digitalRead(RL3_PIN);
  
  // AUX States
  doc["aux1"] = digitalRead(AUX1_PIN);
  doc["aux2"] = digitalRead(AUX2_PIN);
  doc["aux3"] = digitalRead(AUX3_PIN);
  doc["aux4"] = digitalRead(AUX4_PIN);
  
  // Input States (Active Low - inverted)
  doc["switch1"] = !digitalRead(SW1_PIN);
  doc["switch2"] = !digitalRead(SW2_PIN);
  doc["switch3"] = !digitalRead(SW3_PIN);
  doc["iso_input1"] = !digitalRead(ISOIN1_PIN);
  doc["iso_input2"] = !digitalRead(ISOIN2_PIN);
  
  // Weather Data - ส่งเฉพาะเมื่อมีข้อมูลจริง
  if (weather_data_valid) {
    doc["temperature"] = current_weather.temp_current;
    doc["humidity"] = current_weather.humidity;
    // ข้าม city และ description เพื่อลดขนาด JSON
  }
  
  // Air Quality Data - ส่งเฉพาะเมื่อมีข้อมูลจริง
  if (aqi_data_valid) {
    doc["pm2_5"] = air_quality.pm2_5;
    doc["aqi"] = air_quality.aqi;
    // ข้าม pm10 และ air_quality text เพื่อลดขนาด JSON
  }
  
  // Check if JSON serialization is successful
  if (doc.overflowed()) {
    Serial.println("❌ JSON buffer overflow!");
    return;
  }
  
  // Serialize and publish to ThingsBoard
  String output;
  size_t jsonSize = serializeJson(doc, output);
  
  Serial.println("\n┌─────────────────────────────────────────────┐");
  Serial.println("│       SENDING TELEMETRY TO THINGSBOARD     │");
  Serial.println("├─────────────────────────────────────────────┤");
  Serial.print("│ JSON Size: ");
  Serial.print(jsonSize);
  Serial.print(" bytes (Buffer: ");
  Serial.print(MQTT_BUFFER_SIZE);
  Serial.println(" bytes)");
  Serial.print("│ Weather: ");
  Serial.print(weather_data_valid ? "✓" : "✗");
  Serial.print(" | AQI: ");
  Serial.print(aqi_data_valid ? "✓" : "✗");
  Serial.print(" | MQTT: ");
  Serial.println(mqttClient.connected() ? "✓" : "✗");
  Serial.println("└─────────────────────────────────────────────┘");
  
  // Check if JSON fits in MQTT buffer
  if (jsonSize > MQTT_BUFFER_SIZE) {
    Serial.println("❌ JSON too large for MQTT buffer!");
    Serial.print("   Need: ");
    Serial.print(jsonSize);
    Serial.print(" bytes, Have: ");
    Serial.print(MQTT_BUFFER_SIZE);
    Serial.println(" bytes");
    return;
  }
  
  // Publish with retained flag = false
  bool published = mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str());
  
  if (published) {
    Serial.println("✅ Telemetry sent successfully!");
    Serial.print("   JSON: ");
    Serial.println(output);  // แสดง JSON ที่ส่งจริง
    Serial.println();
  } else {
    Serial.println("❌ Failed to send telemetry!");
    Serial.print("   MQTT State Code: ");
    Serial.print(mqttClient.state());
    Serial.println(" (0=Connected, -1=Disconnected, -2=Failed)");
    Serial.print("   JSON: ");
    Serial.println(output);
    Serial.println();
  }
}

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
  
  // Send immediate telemetry update
  DynamicJsonDocument doc(128);
  String relayKey = "relay" + String(relay);
  doc[relayKey] = state;
  
  String output;
  serializeJson(doc, output);
  mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str());
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
  
  // Send immediate telemetry update
  DynamicJsonDocument doc(128);
  String auxKey = "aux" + String(aux);
  doc[auxKey] = state;
  
  String output;
  serializeJson(doc, output);
  mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str());
}