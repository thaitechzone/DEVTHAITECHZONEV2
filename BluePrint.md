# Blueprint for ESPThaiTechZoneV2.0 Board Test Program

Create a complete PlatformIO project for a custom ESP32 board named **ESPThaiTechZoneV2.0**.

The program is a comprehensive board test application that:
- Automatically cycles through testing all outputs (3 Relays and 4 Auxiliary TTL outputs)
- Monitors 5 input states (3 toggle switches and 2 isolated inputs)
- Connects to WiFi and displays connection status with RSSI
- Synchronizes time using NTP servers (Bangkok GMT+7 timezone)
- Fetches and displays current weather data from OpenWeatherMap API
- Monitors air quality (PM2.5) using OpenWeatherMap Air Pollution API
- Displays all information on an I2C OLED screen with optimized layout
- Supports operation with or without OLED display connected
- Integrates DHT22 sensor for local temperature and humidity monitoring
- Switch-controlled relay toggle functionality (SW1-3 controls RELAY1-3)
- Simple Serial debug output for monitoring (telemetry functions removed)

**Key Features:**
- Non-blocking event-driven architecture
- Active Low configuration for all Relays (RL1-3) and Inputs (SW1-3, ISOIN1-2)
- OLED display with optimized refresh to prevent flicker
- Shortened date format (DD/MM/YY) to save display space
- Combined display of date, temperature, and humidity
- Air quality monitoring with PM2.5, AQI, and quality description
- Local DHT22 sensor readings with simulation fallback
- Switch debouncing and toggle functionality for manual relay control
- Simple Serial debug messages for system monitoring
- Graceful degradation when OLED or sensors are disconnected

---

## 1. Project Configuration (`platformio.ini`)

Set up the `platformio.ini` file with the following configuration for an ESP32 Dev Module and include the necessary libraries.

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

# Library dependencies
lib_deps =
  adafruit/Adafruit GFX Library
  adafruit/Adafruit SSD1306
  bblanchon/ArduinoJson@^6.19.4
  adafruit/DHT sensor library
```

---

## 2. Main Source Code (`src/main.cpp`)

Generate the C++ code for `src/main.cpp` based on the following detailed requirements.

### a. Header Includes

Include the necessary libraries:
- `Arduino.h` - Core Arduino framework
- `Wire.h` - I2C communication
- `Adafruit_GFX.h` and `Adafruit_SSD1306.h` - OLED display
- `WiFi.h` and `WiFiClientSecure.h` - WiFi and HTTPS connectivity
- `ArduinoJson.h` - JSON parsing for weather API
- `time.h` - NTP time synchronization
- `DHT.h` - DHT22 temperature and humidity sensor

### b. Pin Definitions

Define all GPIO pins used on the board:

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

// DHT22 Temperature/Humidity Sensor
#define DHT_PIN 13  // DHT22 Data Pin

// Auxiliary TTL 3.3V Outputs
#define AUX1_PIN 12
#define AUX2_PIN 14
#define AUX3_PIN 15
#define AUX4_PIN 25
```

### c. Configuration Constants

Define WiFi, NTP, and Weather API configuration:

```cpp
// WiFi Configuration
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

// NTP Configuration
const char* NTP_SERVER1 = "pool.ntp.org";
const char* NTP_SERVER2 = "time.nist.gov";
const char* NTP_SERVER3 = "time.google.com";
const long GMT_OFFSET_SEC = 7 * 3600;  // GMT+7 for Bangkok
const int DAYLIGHT_OFFSET_SEC = 0;     // Thailand doesn't use DST

// Weather Configuration
const char* OPENWEATHERMAP_API_KEY = "5285b3436c86bdab46069027fd961d09";
const char* WEATHER_CITY = "Tha%20Sala,Nakhon%20Si%20Thammarat,TH";
const char* OWM_HOST = "api.openweathermap.org";

// Air Quality Configuration
const float NAKHON_SI_THAMMARAT_LAT = 8.4304;   // Latitude
const float NAKHON_SI_THAMMARAT_LON = 99.9631;  // Longitude

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
```

### d. Global Variables and Data Structures

1. **OLED Display Object:** `Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)`

2. **Test Cycle Variables:**
   - `current_test_step` - Current output being tested (0-6)
   - `total_test_steps` - Total number of outputs to test (7)
   - `test_step_timer` - Timer for test cycle
   - `test_interval_ms` - Interval between tests (1500ms)

5. **Weather Data Variables:**
   - `weather_update_timer` - Timer for weather updates
   - `weather_update_interval_ms` - Update interval (600000ms = 10 minutes)
   - `weather_data_valid` - Boolean flag for valid weather data
   - `weather_status` - String tracking weather fetch status

6. **DHT22 Sensor Variables:**
   - `dht_temperature` - Current temperature from DHT22
   - `dht_humidity` - Current humidity from DHT22
   - `dht_last_read` - Timer for DHT22 reading interval
   - `dht_read_interval` - Reading interval (2000ms minimum for DHT22)

7. **Switch Control Variables:**
   - `lastSwitchState[]` - Previous state for edge detection (SW1-3)
   - `lastSwitchMillis[]` - Debounce timers for each switch
   - `SWITCH_DEBOUNCE_MS` - Debounce time constant (50ms)

8. **Weather Data Structure:**
```cpp
struct CurrentWeather {
  String city_name;
  String description;
  int temp_current;
  int humidity;
  String icon_code;
};
```

9. **Air Quality Data Variables:**
   - `aqi_update_timer` - Timer for air quality updates
   - `aqi_update_interval_ms` - Update interval (600000ms = 10 minutes)
   - `aqi_data_valid` - Boolean flag for valid AQI data
   - `aqi_status` - String tracking AQI fetch status

10. **Air Quality Data Structure:**
```cpp
struct AirQuality {
  int aqi;           // Air Quality Index (1-5)
  float pm2_5;       // PM2.5 value
  float pm10;        // PM10 value
  String quality;    // Text description (Good, Fair, etc.)
};
```

11. **Status Variables:**
   - `oled_available` - Boolean flag for OLED availability
   - `time_synced` - Boolean flag for NTP sync status
   - `wifi_status` - String for WiFi connection status
   - `wifi_ip` - String for IP address
   - `last_test_item`, `last_wifi_status` - For display optimization
   - `last_debug_print` - Timer for periodic debug output (10 seconds)

### e. `setup()` Function

The `setup()` function performs initialization in this order:

1. **Serial Communication:** Initialize at 115200 baud for debugging
3. **GPIO Initialization:**
   - Set all output pins as OUTPUT
   - Initialize Relays to OFF state (HIGH for Active Low)
   - Initialize AUX outputs to LOW
   - Set all input pins as INPUT_PULLUP (for Active Low configuration)
   - Initialize DHT22 sensor on GPIO 13
   - Initialize switch debouncing arrays and relay state tracking
3. **OLED Display:**
   - Attempt to initialize display
   - If failed: Set `oled_available = false`, blink LED 3 times, continue
   - If successful: Display welcome message for 2 seconds
4. **WiFi Connection:** Call `connectWiFi()` with visual feedback
5. **NTP Time Sync:** Call `syncNTPTime()` if WiFi connected
6. **Initial Data Fetch:** Call `fetchWeatherData()` and `fetchAirQualityData()` if WiFi connected
7. **Initialize Timers:** Set `test_step_timer`, `weather_update_timer`, and `aqi_update_timer`

### f. `loop()` Function

Non-blocking event loop that performs:

1. **Automated Output Test Cycle:**
   - Call `runTestCycle()` to manage output testing
   - Update `current_test_name` based on `current_test_step`:
     - Step 0: "-> RELAY 1"
     - Step 1: "-> RELAY 2"
     - Step 2: "-> RELAY 3"
     - Step 3: "-> AUX 1"
     - Step 4: "-> AUX 2"
     - Step 5: "-> AUX 3"
     - Step 6: "-> AUX 4"

2. **Switch Handling:** Call `handleSwitches()` to process switch presses and toggle relays

3. **DHT22 Sensor Reading:** Call `readDHT22Data()` to read temperature and humidity

4. **WiFi Status Update:** Call `getWiFiStatus()` to get current WiFi info

5. **Weather Data Update:**
   - Check if `weather_update_interval_ms` has elapsed
   - Call `fetchWeatherData()` to refresh weather data

6. **Air Quality Data Update:**
   - Check if `aqi_update_interval_ms` has elapsed
   - Call `fetchAirQualityData()` to refresh air quality data

7. **Display Update:** Call `updateOledDisplay()` with current status strings

8. **Small Delay:** `delay(10)` for stability

### g. Helper Functions

#### `runTestCycle()`
- Non-blocking timer-based function
- Every `test_interval_ms`:
  - Turn off all outputs via `turnOffAllOutputs()`
  - Toggle LED to show activity
  - Turn ON the specific output for current step (considering Active Low for relays)
  - Increment `current_test_step` (wrap around at 7)
- Ensures GPIO activation happens BEFORE display update

#### `turnOffAllOutputs()`
- Set RL1-3 to HIGH (Active Low - OFF)
- Set AUX1-4 to LOW

#### `handleSwitches()`
- Read SW1-3 with debouncing (50ms debounce time)
- Detect falling edge (press) for each switch
- Toggle corresponding relay state (RELAY1-3)
- Update physical relay outputs (Active Low)
- Reset debounce timers after state changes

#### `readDHT22Data()`
- Read DHT22 sensor every 2 seconds minimum
- Update global `dht_temperature` and `dht_humidity`
- Provide simulated values if sensor not connected:
  - Temperature: 25.0 + sin(millis()/10000.0) * 5.0 (20-30°C range)
  - Humidity: 60.0 + cos(millis()/8000.0) * 15.0 (45-75% range)
- Handle sensor read failures gracefully

#### `readInputStates()`
- Read all 5 inputs (SW1-3, ISOIN1-2)
- Return formatted string: "SW:XXX ISO:XX"
- Invert readings for Active Low (LOW = pressed/active = "1")

#### `connectWiFi()`
- Display connection status on OLED and Serial
- Connect to WiFi with timeout (20 attempts)
- Show IP address and RSSI on success
- Handles gracefully if OLED not available

#### `syncNTPTime()`
- Configure NTP with GMT+7 timezone
- Wait for time sync with timeout (10 seconds)
- Display sync status on OLED and Serial
- Set `time_synced` flag on success

#### `getCurrentTime()`
- Get local time from NTP
- Return formatted string "HH:MM:SS"
- Return "N/A" if not synced

#### `getWiFiStatus()`
- Check WiFi connection status
- Return "WiFi:XXdBm" with RSSI if connected
- Return "WiFi:DC" if disconnected

#### `fetchWeatherData()`
- HTTPS GET request to OpenWeatherMap Current Weather API 2.5
- Uses WiFiClientSecure with `setInsecure()`
- Proper HTTP header handling and status code checking
- Parse JSON response using ArduinoJson
- Extract: city_name, temp_current, humidity, description, icon_code
- Update `weather_data_valid` and `weather_status` flags
- Handle all error conditions with descriptive status messages

#### `fetchAirQualityData()`
- HTTPS GET request to OpenWeatherMap Air Pollution API 2.5
- Uses same WiFiClientSecure connection and API key as weather
- Request URL: `http://api.openweathermap.org/data/2.5/air_pollution?lat={LAT}&lon={LON}&appid={API_KEY}`
- Parse JSON response to extract:
  - `pm2_5`: PM2.5 concentration (µg/m³)
  - `pm10`: PM10 concentration (µg/m³)
  - `aqi`: Air Quality Index (1-5 scale)
- Call `getAQIQuality()` to convert AQI to descriptive text
- Update `aqi_data_valid` and `aqi_status` flags
- Handle all error conditions with descriptive status messages

#### `getAQIQuality(int aqi)`
- Convert numeric AQI (1-5) to quality description
- Return values:
  - 1 → "Good"
  - 2 → "Fair"
  - 3 → "Moderate"
  - 4 → "Poor"
  - 5 → "VeryPoor"
  - default → "Unknown"

#### `updateOledDisplay(String test_item, String wifi_info)`
- **Serial Monitor Output:**
  - Print detailed status report every 10 seconds
  - Output format:
    ```
    ╔════════════════════════════════════════════════╗
    ║         ESP32 BOARD STATUS REPORT              ║
    ╠════════════════════════════════════════════════╣
    ║ Mode: AUTO | Uptime: 12345 sec
    ║ WiFi: -45 dBm | IP: 192.168.1.100
    ║ Debug: Serial Monitor Only
    ╠════════════════════════════════════════════════╣
    ║ OUTPUT STATES:
    ║   Relay 1: ON  | Relay 2: OFF | Relay 3: OFF
    ║   AUX 1: OFF | AUX 2: OFF | AUX 3: OFF | AUX 4: OFF
    ║ INPUT STATES:
    ║   SW1: RELEASED | SW2: RELEASED | SW3: RELEASED
    ║   ISO1: INACTIVE | ISO2: INACTIVE
    ╠════════════════════════════════════════════════╣
    ║ WEATHER DATA:
    ║   Temperature: 28°C | Humidity: 75%
    ║   City: Tha Sala | Clear sky
    ║ AIR QUALITY DATA:
    ║   PM2.5: 35.0 µg/m³ | PM10: 45.0 µg/m³
    ║   AQI: 2 (Fair)
    ║ DHT22 SENSOR DATA:
    ║   Temperature: 25.8°C | Humidity: 62.5%
    ╚════════════════════════════════════════════════╝
    ```

- **OLED Display Layout:**
  - Skip if `oled_available == false`
  - Optimize: Only redraw if data changed or 1 second elapsed (for time updates)
  - **Line 1 (Size 2):** Test item name (e.g., "-> RELAY 1")
  - **Line 2:** Horizontal separator line
  - **Line 3 (Size 1):** Input states and relay states "SW:XXX R:XXX"
  - **Line 4 (Size 1):** WiFi status + current time "WiFi:-45dBm HH:MM:SS"
  - **Line 5 (Size 1):** Date + DHT22 readings "16/10/25 DHT:25.8C 62%"
  - **Line 6 (Size 1):** Weather + Air Quality "28C H75% PM35 AQI2"

---

## 3. Key Implementation Details

### Active Low Configuration
- **Relays (RL1-3):** HIGH = OFF, LOW = ON
- **Inputs (SW1-3, ISOIN1-2):** Use INPUT_PULLUP, LOW = pressed/active
- Display shows inverted input values (LOW input shown as "1")

### Non-Blocking Architecture
- All timing uses `millis()` - no blocking delays in main loop
- Multiple independent timers for different tasks
- Event-driven updates only when necessary

### OLED Fault Tolerance
- Check `oled_available` before all display operations
- Continue all functionality if OLED disconnects
- LED blinks 3 times on startup if OLED not detected
- All status mirrored to Serial Monitor

### Display Optimization
- Track previous display state to prevent unnecessary redraws
- Force update every 1 second for time display
- Immediate update when test step or data changes
- Prevents screen flicker

### Weather API Integration
- HTTPS connection with proper certificate handling
- Robust HTTP response parsing with status code checking
- JSON parsing with error handling
- 10-minute update interval to respect API rate limits
- Displays current temperature and humidity only (accurate data)

### Air Quality API Integration
- HTTPS connection to OpenWeatherMap Air Pollution API 2.5
- Uses same API key as weather service
- Location: Nakhon Si Thammarat (8.4304°N, 99.9631°E)
- 10-minute update interval (synchronized with weather updates)
- Displays PM2.5, AQI (1-5 scale), and quality description
- AQI Scale: 1=Good, 2=Fair, 3=Moderate, 4=Poor, 5=VeryPoor
- Follows WHO Air Quality Guidelines for PM2.5 monitoring

### DHT22 Sensor Integration
- Local temperature and humidity monitoring on GPIO 13
- 2-second minimum reading interval for sensor stability
- Simulated values when sensor not connected for development
- Temperature simulation: 20-30°C sinusoidal variation
- Humidity simulation: 45-75% cosine variation
- Graceful fallback ensures system always has data

### Switch-Controlled Relay Operation
- SW1, SW2, SW3 control RELAY1, RELAY2, RELAY3 respectively
- Debounced edge detection (50ms debounce time)
- Toggle operation: each press toggles relay ON/OFF state
- Independent operation alongside automatic test cycle
- Real-time status display showing current relay states

### Display Optimization
- Date format shortened from "Date: DD/MM/YYYY" to "DD/MM/YY" (saves 9 characters)
- Line 5 combines date and DHT22 sensor readings
- Line 6 combines weather data and air quality information
- Efficient use of 128x64 OLED display space
- Shows both relay states and switch states for manual control feedback

### Serial Debug Integration
- 115200 baud for fast communication
- Comprehensive status report every 10 seconds
- Human-readable format for easy monitoring
- Includes all sensor data, relay states, switch states, and system status
- Real-time monitoring of DHT22 sensor readings
- Individual action logging (switch presses, relay changes)
- Works independently of OLED status for headless operation

### System Architecture (Current)
- **Standalone Operation:** No external cloud integrations (ThingsBoard/MQTT removed)
- **Local Monitoring:** All status via Serial Monitor and OLED display
- **Dual Mode Operation:**
  - **AUTO Mode:** Automatic cycling through relay/AUX outputs every 1.5 seconds
  - **MANUAL Mode:** Switch-controlled relay operation (SW1-3 → RELAY1-3)
- **Sensor Integration:** Weather APIs, Air Quality APIs, and local DHT22 sensor
- **Fault Tolerance:** Graceful degradation when sensors or display unavailable
- **Memory Efficient:** Removed JSON serialization overhead for better performance

---

## 4. Testing and Validation

The program should:
✅ Compile without errors for ESP32
✅ Cycle through all 7 outputs every 1.5 seconds
✅ Correctly handle Active Low for relays and inputs
✅ Display all information on OLED when available
✅ Continue operation if OLED disconnects
✅ Connect to WiFi and maintain connection
✅ Sync time with NTP servers
✅ Fetch weather data from OpenWeatherMap
✅ Read DHT22 sensor data with simulation fallback
✅ Handle switch-controlled relay toggles with debouncing
✅ Output comprehensive debug status to Serial Monitor every 10 seconds
✅ Log individual actions (switch presses, relay changes) in real-time
✅ Respond to input changes in real-time
✅ Handle all error conditions gracefully

---

## 5. Usage Instructions

1. **Upload to Board:** Use PlatformIO to compile and upload
2. **Monitor Serial:** Open Serial Monitor at 115200 baud to see status
3. **Observe OLED:** Check display for visual feedback (if connected)
4. **Test Outputs:** Verify each relay and AUX activates in sequence
5. **Test Inputs:** Toggle switches and verify display updates
6. **Check WiFi:** Confirm connection and RSSI display
7. **Verify Time:** Check time updates every second
8. **Monitor Weather:** Observe temperature and humidity updates
9. **Test DHT22:** Check local temperature/humidity readings on GPIO 13
10. **Test Switch Control:** Press SW1-3 to manually toggle RELAY1-3
11. **Monitor Debug Output:** Check Serial output for comprehensive status reports and action logs

---

*This blueprint represents the complete, production-ready ESPThaiTechZoneV2.0 board test program with WiFi, NTP, weather API integration, DHT22 sensor support, switch-controlled relay functionality, comprehensive Serial debug output, and full OLED fault tolerance.*