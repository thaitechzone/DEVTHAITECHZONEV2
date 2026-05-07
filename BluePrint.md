# Blueprint for ESPThaiTechZoneV2.0 Board Test Program (No DHT22)

Create a complete PlatformIO project for a custom ESP32 board named ESPThaiTechZoneV2.0.

This program is a comprehensive board test application that:
- Automatically cycles through testing all outputs (3 relays and 4 auxiliary TTL outputs)
- Monitors 5 input states (3 toggle switches and 2 isolated inputs)
- Connects to WiFi and displays connection status with RSSI
- Synchronizes time using NTP servers (Bangkok GMT+7 timezone)
- Fetches and displays current weather data from OpenWeatherMap API
- Monitors air quality (PM2.5) using OpenWeatherMap Air Pollution API
- Displays all information on an I2C OLED screen with optimized layout
- Supports operation with or without OLED display connected
- Provides switch-controlled relay toggle functionality (SW1-3 controls RELAY1-3)
- Uses simple Serial debug output for monitoring (telemetry functions removed)

Key features:
- Non-blocking event-driven architecture
- Active Low configuration for relays (RL1-3) and inputs (SW1-3, ISOIN1-2)
- OLED display with optimized refresh to prevent flicker
- Shortened date format (DD/MM/YY) to save display space
- Air quality monitoring with PM2.5, AQI, and quality description
- Switch debouncing and toggle functionality for manual relay control
- Graceful degradation when OLED is disconnected

---

## 1. Project Configuration (platformio.ini)

Use ESP32 Dev Module and the following dependencies:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps =
  adafruit/Adafruit GFX Library
  adafruit/Adafruit SSD1306
  bblanchon/ArduinoJson@^6.19.4
```

---

## 2. Main Source Code (src/main.cpp)

### a. Header Includes

Include:
- Arduino.h
- Wire.h
- Adafruit_GFX.h
- Adafruit_SSD1306.h
- WiFi.h
- WiFiClientSecure.h
- ArduinoJson.h
- time.h

### b. Pin Definitions

```cpp
// Input Toggle Switches (Active Low)
#define SW1_PIN 34
#define SW2_PIN 35
#define SW3_PIN 32

// Isolated Inputs (Active Low)
#define ISOIN1_PIN 33
#define ISOIN2_PIN 27

// Output Pins
#define LED_PIN 2
#define RL1_PIN 17  // RELAY1 (Active Low)
#define RL2_PIN 16  // RELAY2 (Active Low)
#define RL3_PIN 4   // RELAY3 (Active Low)

// Auxiliary TTL 3.3V Outputs
#define AUX1_PIN 12
#define AUX2_PIN 14
#define AUX3_PIN 15
#define AUX4_PIN 13

// Serial AUX pins
#define AUX_SR1_TX 25
#define AUX_SR1_RX 26
```

### c. Configuration Constants

```cpp
// WiFi Configuration
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

// NTP Configuration
const char* NTP_SERVER1 = "pool.ntp.org";
const char* NTP_SERVER2 = "time.nist.gov";
const char* NTP_SERVER3 = "time.google.com";
const long GMT_OFFSET_SEC = 7 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

// Weather Configuration
const char* OPENWEATHERMAP_API_KEY = "<YOUR_API_KEY>";
const char* WEATHER_CITY = "Tha%20Sala,Nakhon%20Si%20Thammarat,TH";
const char* OWM_HOST = "api.openweathermap.org";

// Air Quality Configuration
const float NAKHON_SI_THAMMARAT_LAT = 8.4304;
const float NAKHON_SI_THAMMARAT_LON = 99.9631;

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
```

### d. Global Variables and Data Structures

1. OLED display object:
   - Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)

2. Test cycle variables:
   - current_test_step (0-6)
   - total_test_steps = 7
   - test_step_timer
   - test_interval_ms = 1500

3. Weather variables:
   - weather_update_timer
   - weather_update_interval_ms = 600000
   - weather_data_valid
   - weather_status

4. Switch control variables:
   - lastSwitchState[]
   - lastSwitchMillis[]
   - SWITCH_DEBOUNCE_MS = 50

5. Weather structure:

```cpp
struct CurrentWeather {
  String city_name;
  String description;
  int temp_current;
  int humidity;
  String icon_code;
};
```

6. Air quality variables:
   - aqi_update_timer
   - aqi_update_interval_ms = 600000
   - aqi_data_valid
   - aqi_status

7. Air quality structure:

```cpp
struct AirQuality {
  int aqi;
  float pm2_5;
  float pm10;
  String quality;
};
```

8. Status variables:
   - oled_available
   - time_synced
   - wifi_status
   - wifi_ip
   - last_test_item
   - last_wifi_status
   - last_debug_print

### e. setup() Function

Initialize in this order:
1. Serial at 115200
2. GPIO setup
   - Outputs as OUTPUT
   - Relays OFF (HIGH for active-low)
   - AUX outputs LOW
   - Inputs as INPUT_PULLUP
   - Initialize switch debounce and relay tracking state
3. OLED init
   - If failed: set oled_available = false, blink LED 3 times, continue
   - If success: show welcome for 2 seconds
4. WiFi connect via connectWiFi()
5. NTP sync via syncNTPTime() when WiFi is connected
6. Initial fetch via fetchWeatherData() and fetchAirQualityData()
7. Initialize timers

### f. loop() Function

Non-blocking loop tasks:
1. Run automated output test via runTestCycle()
2. Handle switch presses via handleSwitches()
3. Update WiFi info via getWiFiStatus()
4. Refresh weather every 10 minutes
5. Refresh AQI every 10 minutes
6. Update OLED via updateOledDisplay()
7. delay(10)

### g. Helper Functions

Required helpers:
- runTestCycle()
- turnOffAllOutputs()
- handleSwitches()
- readInputStates()
- connectWiFi()
- syncNTPTime()
- getCurrentTime()
- getWiFiStatus()
- fetchWeatherData()
- fetchAirQualityData()
- getAQIQuality(int aqi)
- updateOledDisplay(String test_item, String wifi_info)

---

## 3. Implementation Notes

### Active-low behavior
- Relays: HIGH = OFF, LOW = ON
- Inputs: INPUT_PULLUP, LOW = active

### Non-blocking design
- Use millis() timers
- No long blocking delays in loop()

### OLED fault tolerance
- Check oled_available before all OLED operations
- Continue running when OLED is missing

### API and network behavior
- Use HTTPS with WiFiClientSecure and setInsecure()
- Handle HTTP status and parse errors explicitly
- Update weather and AQI every 10 minutes

### Switch control
- SW1-3 toggles RELAY1-3
- Debounced falling-edge detection

### Serial debug
- Print full status report every 10 seconds
- Include mode, uptime, output states, input states, WiFi, weather, and AQI

---

## 4. Validation Checklist

The program should:
- Compile for ESP32 without errors
- Cycle through 7 outputs every 1.5 seconds
- Handle active-low relay/input logic correctly
- Keep operating when OLED is not connected
- Connect to WiFi and show RSSI/IP
- Sync NTP time in GMT+7
- Fetch weather and AQI data
- Debounce switches and toggle relays correctly
- Print serial debug reports every 10 seconds

---

## 5. Usage

1. Build and upload with PlatformIO
2. Open Serial Monitor at 115200
3. Verify output cycling and input response
4. Verify WiFi and time status
5. Verify weather and AQI updates

---

This blueprint reflects the production target for ESPThaiTechZoneV2.0 without DHT22 integration.
