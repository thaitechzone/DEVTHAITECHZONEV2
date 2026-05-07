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

// // ===== WiFi Configuration =====
// const char* WIFI_SSID = "myHome_2.4GHz";
// const char* WIFI_PASSWORD = "0939391546";

// ===== WiFi Configuration =====
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

// ===== NTP Configuration =====
const char* NTP_SERVER1 = "pool.ntp.org";
const char* NTP_SERVER2 = "time.nist.gov";
const char* NTP_SERVER3 = "time.google.com";
const long GMT_OFFSET_SEC = 7 * 3600;  // GMT+7 for Bangkok
const int DAYLIGHT_OFFSET_SEC = 0;     // Thailand doesn't use DST

// ===== Weather Configuration =====
const char* OPENWEATHERMAP_API_KEY = "5285b3436c86bdab46069027fd961d09";
const char* WEATHER_CITY = "Tha Sala,Nakhon Si Thammarat,TH";
const char* OWM_HOST = "api.openweathermap.org";

// ===== Air Quality Fallback Coordinates =====
const float DEFAULT_WEATHER_LAT = 8.4304;   // Used when weather city lookup fails
const float DEFAULT_WEATHER_LON = 99.9631;  // Used when weather city lookup fails

// ===== MQTT Configuration =====
const char* MQTT_BROKER = "broker.hivemq.com";  // HiveMQ free public broker
const int MQTT_PORT = 1883;                      // MQTT port (non-TLS)
const char* MQTT_BOARD_ID = "esp32-devkit-01";  // Unique board identifier (change for each board)

// Topic Structure:
// - Telemetry: device/{boardID}/telemetry
// - Control: device/{boardID}/control/relay/{relayNum} (1-3)
// - Control All: device/{boardID}/control/relay/all
// - Status: device/{boardID}/status

// ===== OLED Display Configuration =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== MQTT Client =====
WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool mqtt_connected = false;
unsigned long mqtt_reconnect_timer = 0;
const unsigned long mqtt_reconnect_interval_ms = 5000;  // Try to reconnect every 5 seconds
unsigned long mqtt_publish_timer = 0;
const unsigned long mqtt_publish_interval_ms = 30000;   // Publish telemetry every 30 seconds

// ===== Global Variables for Test Cycle =====
int current_test_step = 0;
const int total_test_steps = 7;
unsigned long test_step_timer = 0;
const unsigned long test_interval_ms = 1500;

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
  float latitude;
  float longitude;
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
String encodeUrlSpaces(String text);
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishTelemetry();
void handleRelayCommand(String relay_num, bool state);
void handleAllRelayCommand(bool state);

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

  // 6. Sync Time with NTP Server
  if(WiFi.status() == WL_CONNECTED) {
    syncNTPTime();
    fetchWeatherData();       // Fetch initial weather data
    fetchAirQualityData();    // Fetch initial air quality data
    
    // 7. Initialize MQTT
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    connectMQTT();
  }
  
  // Initialize timers
  test_step_timer = millis();
  weather_update_timer = millis();
  aqi_update_timer = millis();
}

// ===== Main Loop =====
void loop() {
  static String current_test_name = "Initializing";

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

  // 2. Update WiFi Status
  String wifi_info = getWiFiStatus();
  
  // 3. Maintain MQTT Connection
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      if (millis() - mqtt_reconnect_timer >= mqtt_reconnect_interval_ms) {
        mqtt_reconnect_timer = millis();
        connectMQTT();
      }
    } else {
      // MQTT connected - handle incoming messages
      mqttClient.loop();
      
      // Publish telemetry periodically
      if (millis() - mqtt_publish_timer >= mqtt_publish_interval_ms) {
        mqtt_publish_timer = millis();
        publishTelemetry();
      }
    }
  }
  
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

  // 6. Update OLED Display
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

// ===== Helper Function: Encode Spaces for URL Query =====
String encodeUrlSpaces(String text) {
  text.replace(" ", "%20");
  return text;
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
  String encoded_city = encodeUrlSpaces(String(WEATHER_CITY));
  String url = String("/data/2.5/weather?q=") + encoded_city +
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
  current_weather.latitude = doc["coord"]["lat"].as<float>();
  current_weather.longitude = doc["coord"]["lon"].as<float>();

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
  
  // Use coordinates resolved from WEATHER_CITY, fallback to default if weather lookup failed
  float air_quality_lat = weather_data_valid ? current_weather.latitude : DEFAULT_WEATHER_LAT;
  float air_quality_lon = weather_data_valid ? current_weather.longitude : DEFAULT_WEATHER_LON;
  String url = String("/data/2.5/air_pollution?lat=") + String(air_quality_lat, 4) +
               "&lon=" + String(air_quality_lon, 4) +
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

// ===== Helper Function: Connect to MQTT Broker =====
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  Serial.print("Connecting to MQTT broker ");
  Serial.print(MQTT_BROKER);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  
  // Create MQTT client ID with board ID
  String client_id = "esp32-" + String(MQTT_BOARD_ID) + "-" + String(random(10000));
  
  // Attempt to connect
  if (mqttClient.connect(client_id.c_str())) {
    mqtt_connected = true;
    Serial.println("✓ MQTT Connected!");
    Serial.print("Board ID: ");
    Serial.println(MQTT_BOARD_ID);
    
    // Subscribe to control topics for all relays
    String control_relay1 = String("device/") + MQTT_BOARD_ID + "/control/relay/1";
    String control_relay2 = String("device/") + MQTT_BOARD_ID + "/control/relay/2";
    String control_relay3 = String("device/") + MQTT_BOARD_ID + "/control/relay/3";
    String control_relay_all = String("device/") + MQTT_BOARD_ID + "/control/relay/all";
    
    mqttClient.subscribe(control_relay1.c_str());
    mqttClient.subscribe(control_relay2.c_str());
    mqttClient.subscribe(control_relay3.c_str());
    mqttClient.subscribe(control_relay_all.c_str());
    
    Serial.println("Subscribed to relay control topics");
    Serial.println(String("Relay all topic: ") + control_relay_all);
    
    // Publish initial status
    String status_topic = String("device/") + MQTT_BOARD_ID + "/status";
    String status_json = "{\"board_id\":\"" + String(MQTT_BOARD_ID) + "\",\"status\":\"online\"}";
    mqttClient.publish(status_topic.c_str(), status_json.c_str());
    
  } else {
    mqtt_connected = false;
    Serial.print("✗ MQTT Connection failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" (will retry in 5 seconds)");
  }
}

// ===== Helper Function: MQTT Callback (Handle Incoming Messages) =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  
  // Convert payload to string
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Payload: ");
  Serial.println(message);
  
  // Parse topic: device/{boardID}/control/relay/{relayNum|all}
  String topic_str = String(topic);
  
  // Check if this is a relay control message
  if (topic_str.startsWith("device/" + String(MQTT_BOARD_ID) + "/control/relay/")) {
    // Extract relay number or "all" from topic
    int relay_index = topic_str.lastIndexOf('/');
    String relay_num_str = topic_str.substring(relay_index + 1);
    
    // Parse command JSON: {"state": true/false} or {"on": true/false} or just true/false
    bool relay_state = false;
    
    // Try to parse as JSON first
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error) {
      // JSON parsing successful
      if (doc.containsKey("state")) {
        relay_state = doc["state"].as<bool>();
      } else if (doc.containsKey("on")) {
        relay_state = doc["on"].as<bool>();
      }
    } else {
      // Try to parse as plain boolean text
      if (message == "true" || message == "1" || message == "on" || message == "ON") {
        relay_state = true;
      } else if (message == "false" || message == "0" || message == "off" || message == "OFF") {
        relay_state = false;
      }
    }
    
    // Handle the relay command
    if (relay_num_str == "all") {
      handleAllRelayCommand(relay_state);
    } else {
      handleRelayCommand(relay_num_str, relay_state);
    }
  }
}

// ===== Helper Function: Handle Relay Commands =====
void handleRelayCommand(String relay_num, bool state) {
  int relay_pin = 0;
  
  Serial.print("Setting Relay ");
  Serial.print(relay_num);
  Serial.print(" to ");
  Serial.println(state ? "ON" : "OFF");
  
  // Get pin number for relay
  if (relay_num == "1") {
    relay_pin = RL1_PIN;
  } else if (relay_num == "2") {
    relay_pin = RL2_PIN;
  } else if (relay_num == "3") {
    relay_pin = RL3_PIN;
  } else {
    Serial.println("Invalid relay number");
    return;
  }
  
  // Set relay state (Active Low: LOW = ON, HIGH = OFF)
  digitalWrite(relay_pin, state ? LOW : HIGH);
  
  // Publish feedback
  String feedback_topic = String("device/") + MQTT_BOARD_ID + "/relay/" + relay_num + "/status";
  String feedback_json = "{\"state\":" + String(state ? "true" : "false") + "}";
  mqttClient.publish(feedback_topic.c_str(), feedback_json.c_str());
  
  Serial.print("Relay ");
  Serial.print(relay_num);
  Serial.print(" feedback published: ");
  Serial.println(feedback_json);
}

// ===== Helper Function: Handle All Relay Commands =====
void handleAllRelayCommand(bool state) {
  Serial.print("Setting all relays to ");
  Serial.println(state ? "ON" : "OFF");

  handleRelayCommand("1", state);
  handleRelayCommand("2", state);
  handleRelayCommand("3", state);

  String feedback_topic = String("device/") + MQTT_BOARD_ID + "/relay/all/status";
  String feedback_json = "{\"relay1\":" + String(state ? "true" : "false") +
                         ",\"relay2\":" + String(state ? "true" : "false") +
                         ",\"relay3\":" + String(state ? "true" : "false") + "}";
  mqttClient.publish(feedback_topic.c_str(), feedback_json.c_str());

  Serial.print("All relay feedback published: ");
  Serial.println(feedback_json);
}

// ===== Helper Function: Publish Telemetry =====
void publishTelemetry() {
  if (!mqttClient.connected()) {
    return;
  }
  
  // Create telemetry JSON
  DynamicJsonDocument doc(512);
  
  // Add air quality data
  if (aqi_data_valid) {
    doc["aqi"] = air_quality.aqi;
    doc["pm2_5"] = air_quality.pm2_5;
    doc["pm10"] = air_quality.pm10;
    doc["quality"] = air_quality.quality;
  }
  
  // Add weather data
  if (weather_data_valid) {
    doc["temperature"] = current_weather.temp_current;
    doc["humidity"] = current_weather.humidity;
    doc["weather_lat"] = current_weather.latitude;
    doc["weather_lon"] = current_weather.longitude;
  }
  
  // Add relay states (Active Low: LOW = ON)
  doc["relay1"] = !digitalRead(RL1_PIN);
  doc["relay2"] = !digitalRead(RL2_PIN);
  doc["relay3"] = !digitalRead(RL3_PIN);
  
  // Add timestamp
  if (time_synced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timestamp[30];
      sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
              timeinfo.tm_year + 1900,
              timeinfo.tm_mon + 1,
              timeinfo.tm_mday,
              timeinfo.tm_hour,
              timeinfo.tm_min,
              timeinfo.tm_sec);
      doc["timestamp"] = timestamp;
    }
  }
  
  // Add WiFi signal strength
  if (WiFi.status() == WL_CONNECTED) {
    doc["rssi"] = WiFi.RSSI();
  }
  
  // Add uptime
  doc["uptime_seconds"] = millis() / 1000;
  
  // Serialize and publish
  String telemetry_topic = String("device/") + MQTT_BOARD_ID + "/telemetry";
  String telemetry_json = "";
  serializeJson(doc, telemetry_json);
  
  if (mqttClient.publish(telemetry_topic.c_str(), telemetry_json.c_str())) {
    Serial.print("Telemetry published: ");
    Serial.println(telemetry_json);
  } else {
    Serial.println("Failed to publish telemetry");
  }
}

// ===== Helper Function: Update OLED Display =====
void updateOledDisplay(String test_item, String wifi_info) {
  static unsigned long last_update = 0;
  static String last_time_str = "";
  static String last_relay_states = "";
  static bool last_mqtt_connected = false;
  String current_time_str = time_synced ? getCurrentTime() : "";
  String relay_states = "";
  relay_states += !digitalRead(RL1_PIN) ? "1" : "0";
  relay_states += !digitalRead(RL2_PIN) ? "1" : "0";
  relay_states += !digitalRead(RL3_PIN) ? "1" : "0";

  bool mqtt_is_connected = mqttClient.connected();
  
  // Force update every second if time is synced, or when data changes
  bool force_update = (time_synced && (millis() - last_update >= 1000));
  bool data_changed = (test_item != last_test_item || 
                       wifi_info != last_wifi_status ||
                       current_time_str != last_time_str ||
                       relay_states != last_relay_states ||
                       mqtt_is_connected != last_mqtt_connected);
  
  // Print debug info every 10 seconds (ไม่บ่อยเกินไป)
  if (millis() - last_debug_print >= debug_print_interval) {
    last_debug_print = millis();
    
    Serial.println("\n╔════════════════════════════════════════════════╗");
    Serial.println("║         ESP32 BOARD STATUS REPORT              ║");
    Serial.println("╠════════════════════════════════════════════════╣");
    
    // System Info
    Serial.print("║ Mode: ");
    Serial.print("AUTO");
    Serial.print(" | Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" sec");
    
    // WiFi Info
    Serial.print("║ WiFi: ");
    Serial.print(WiFi.RSSI());
    Serial.print(" dBm | IP: ");
    Serial.println(WiFi.localIP());
    
    // Output States
    Serial.println("╠════════════════════════════════════════════════╣");
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
  last_relay_states = relay_states;
  last_mqtt_connected = mqtt_is_connected;
  last_update = millis();
  
  // Clear and redraw display
  display.clearDisplay();
  
  // Line 1: Current system mode and time
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("ESP32 MQTT");
  if (time_synced) {
    display.setCursor(80, 0);
    display.print(current_time_str.substring(0, 5));
  }
  
  // Line 2: Network status
  display.setCursor(0, 10);
  display.print(wifi_info);
  display.setCursor(82, 10);
  display.print("MQ:");
  display.print(mqtt_is_connected ? "OK" : "DC");
  
  // Line 3: Relay states and all-relay control hint
  display.setCursor(0, 21);
  display.print("RLY:");
  display.print(relay_states);
  display.print(" Topic:/relay");
  
  // Line 4: Weather and AQI summary
  display.setCursor(0, 32);
  if (weather_data_valid) {
    display.printf("T:%dC H:%d%%",
        current_weather.temp_current,
        current_weather.humidity);
  } else {
    display.print("Weather:");
    display.print(weather_status.substring(0, 10));
  }

  if (aqi_data_valid) {
    display.printf(" AQI:%d", air_quality.aqi);
  }

  // Line 5: Air quality detail
  display.setCursor(0, 43);
  if (aqi_data_valid) {
    display.printf("PM2.5:%.0f", air_quality.pm2_5);
  } else {
    display.print("AQI:");
    display.print(aqi_status.substring(0, 12));
  }

  // Line 6: Telemetry and board status
  display.setCursor(0, 54);
  display.print(mqtt_is_connected ? "PUB:30s" : "OFFLINE");
  
  // Update display
  display.display();
}
