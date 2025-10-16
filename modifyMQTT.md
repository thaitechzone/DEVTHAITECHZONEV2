แน่นอนครับ\! การเพิ่ม MQTT เข้าไปคือการยกระดับโปรเจกต์นี้ให้กลายเป็นอุปกรณ์ IoT ที่สมบูรณ์แบบอย่างแท้จริง ทำให้คุณสามารถควบคุมและตรวจสอบบอร์ดจากที่ไหนก็ได้ผ่าน Dashboard

นี่คือ Prompt ในรูปแบบ Markdown ที่ออกแบบมาอย่างละเอียด เพื่อให้ Copilot สามารถช่วยคุณปรับปรุงโค้dได้อย่างแม่นยำและรวดเร็วที่สุดครับ

-----

````markdown
# Prompt for GitHub Copilot: Integrate MQTT for IoT Dashboard Control

**Goal:** Modify the existing ESPThaiTechZoneV2.0 project to add full MQTT communication. The device should connect to an MQTT broker to allow remote control of relays and monitoring of switches from an IoT dashboard.

---

### 1. Update Project Configuration (`platformio.ini`)

Add the **`PubSubClient`** library to `lib_deps`. This is the standard library for MQTT on Arduino/ESP32.

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
  knolleary/PubSubClient@^2.8
````

-----

### 2\. Modify Main Source Code (`src/main.cpp`)

#### a. Add New Library Includes

At the top of the file, include the `PubSubClient` library.

```cpp
#include <time.h>
#include <PubSubClient.h> // <<< ADD THIS LINE
```

#### b. Add New MQTT Configuration Constants

Place this new section near the WiFi and Weather configurations. **You will need to fill in your actual MQTT broker details here.**

```cpp
// ===== MQTT Configuration =====
const char* MQTT_SERVER = "your_mqtt_broker_address"; // e.g., "broker.hivemq.com"
const int MQTT_PORT = 1883;
const char* MQTT_USER = "your_mqtt_username"; // Can be empty if not required
const char* MQTT_PASSWORD = "your_mqtt_password"; // Can be empty if not required
const char* MQTT_CLIENT_ID = "ESPThaiTechZoneV2_01"; // Must be unique for each device

// --- MQTT Topics ---
// Base topic for this device
const char* MQTT_BASE_TOPIC = "thaitechzone/v2_board";
// Topics for controlling relays (dashboard sends to these)
const char* MQTT_RELAY1_COMMAND_TOPIC = "thaitechzone/v2_board/command/relay1";
const char* MQTT_RELAY2_COMMAND_TOPIC = "thaitechzone/v2_board/command/relay2";
const char* MQTT_RELAY3_COMMAND_TOPIC = "thaitechzone/v2_board/command/relay3";
// Topics for reporting state (device sends to these)
const char* MQTT_RELAY1_STATE_TOPIC = "thaitechzone/v2_board/state/relay1";
const char* MQTT_RELAY2_STATE_TOPIC = "thaitechzone/v2_board/state/relay2";
const char* MQTT_RELAY3_STATE_TOPIC = "thaitechzone/v2_board/state/relay3";
const char* MQTT_SW1_STATE_TOPIC = "thaitechzone/v2_board/state/switch1";
const char* MQTT_SW2_STATE_TOPIC = "thaitechzone/v2_board/state/switch2";
const char* MQTT_SW3_STATE_TOPIC = "thaitechzone/v2_board/state/switch3";
const char* MQTT_STATUS_TOPIC = "thaitechzone/v2_board/status";
```

#### c. Instantiate MQTT Client Objects

Just below the `Adafruit_SSD1306` object, add the MQTT client instances.

```cpp
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient espClient; // <<< ADD THIS LINE
PubSubClient mqttClient(espClient); // <<< ADD THIS LINE
```

#### d. Create the Core MQTT Functions

These are two new, critical functions. Place them before the `setup()` function.

1.  **`mqttCallback()`**: This function processes messages that arrive from the MQTT broker (i.e., commands from your dashboard).
2.  **`reconnectMQTT()`**: This function handles connecting and reconnecting to the broker, ensuring a stable connection.

<!-- end list -->

```cpp
// ===== Function Declarations =====
void updateOledDisplay(String test_item, String wifi_info);
void turnOffAllOutputs();
// ... (existing declarations)
void mqttCallback(char* topic, byte* payload, unsigned int length); // <<< ADD THIS
void reconnectMQTT(); // <<< ADD THIS
String getAQIQuality(int aqi);

// ===== MQTT Callback Function =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.toUpperCase(); // Standardize to uppercase

  // Determine which relay to control based on the topic
  if (strcmp(topic, MQTT_RELAY1_COMMAND_TOPIC) == 0) {
    if (message == "ON") {
      digitalWrite(RL1_PIN, LOW); // Active Low
      mqttClient.publish(MQTT_RELAY1_STATE_TOPIC, "ON", true);
    } else if (message == "OFF") {
      digitalWrite(RL1_PIN, HIGH); // Active Low
      mqttClient.publish(MQTT_RELAY1_STATE_TOPIC, "OFF", true);
    }
  }
  else if (strcmp(topic, MQTT_RELAY2_COMMAND_TOPIC) == 0) {
    if (message == "ON") {
      digitalWrite(RL2_PIN, LOW);
      mqttClient.publish(MQTT_RELAY2_STATE_TOPIC, "ON", true);
    } else if (message == "OFF") {
      digitalWrite(RL2_PIN, HIGH);
      mqttClient.publish(MQTT_RELAY2_STATE_TOPIC, "OFF", true);
    }
  }
    else if (strcmp(topic, MQTT_RELAY3_COMMAND_TOPIC) == 0) {
    if (message == "ON") {
      digitalWrite(RL3_PIN, LOW);
      mqttClient.publish(MQTT_RELAY3_STATE_TOPIC, "ON", true);
    } else if (message == "OFF") {
      digitalWrite(RL3_PIN, HIGH);
      mqttClient.publish(MQTT_RELAY3_STATE_TOPIC, "OFF", true);
    }
  }
}

// ===== MQTT Reconnect Function =====
void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Subscribe to command topics
      mqttClient.subscribe(MQTT_RELAY1_COMMAND_TOPIC);
      mqttClient.subscribe(MQTT_RELAY2_COMMAND_TOPIC);
      mqttClient.subscribe(MQTT_RELAY3_COMMAND_TOPIC);
      // Publish online status
      mqttClient.publish(MQTT_STATUS_TOPIC, "online", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
```

#### e. Modify the `setup()` Function

After connecting to WiFi, set up the MQTT client.

```cpp
// In setup(), after the connectWiFi() call
connectWiFi();

// Configure MQTT Client
mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
mqttClient.setCallback(mqttCallback);
```

#### f. Modify the `loop()` Function

This is a key change. We will replace the automatic `runTestCycle()` with logic that listens for MQTT commands and reports switch status.

1.  **Remove** the automatic test cycle.
2.  **Add** calls to `reconnectMQTT()` and `mqttClient.loop()`.
3.  **Add** logic to read switches and publish their state *only when a change is detected*.

<!-- end list -->

```cpp
// ===== Main Loop =====
void loop() {
  // 1. Ensure MQTT is connected
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
    mqttClient.loop(); // This is essential for MQTT client to process messages
  }

  // 2. Handle Switch State Publishing (on change)
  static bool last_sw_states[3] = {true, true, true}; // Start with OFF state (HIGH for PULLUP)
  bool current_sw1 = digitalRead(SW1_PIN);
  bool current_sw2 = digitalRead(SW2_PIN);
  bool current_sw3 = digitalRead(SW3_PIN);

  if (current_sw1 != last_sw_states[0]) {
    last_sw_states[0] = current_sw1;
    // Remember: Active Low, so LOW means "ON"
    mqttClient.publish(MQTT_SW1_STATE_TOPIC, current_sw1 ? "OFF" : "ON", true);
  }
  if (current_sw2 != last_sw_states[1]) {
    last_sw_states[1] = current_sw2;
    mqttClient.publish(MQTT_SW2_STATE_TOPIC, current_sw2 ? "OFF" : "ON", true);
  }
  if (current_sw3 != last_sw_states[2]) {
    last_sw_states[2] = current_sw3;
    mqttClient.publish(MQTT_SW3_STATE_TOPIC, current_sw3 ? "OFF" : "ON", true);
  }

  // NOTE: The automatic runTestCycle() is now REMOVED as control is via MQTT.
  // We can keep the test name variable to show MQTT status instead.
  String main_status = mqttClient.connected() ? "MQTT: Connected" : "MQTT: Disconn.";

  // 3. Update WiFi Status
  String wifi_info = getWiFiStatus();

  // ... (Keep the periodic Weather and AQI updates) ...

  // 5. Update OLED Display
  updateOledDisplay(main_status, wifi_info);

  // Small delay for stability
  delay(10);
}
```

#### g. Modify `updateOledDisplay()` Function

Change the top line of the display to show the MQTT connection status instead of the old test cycle item.

```cpp
// In updateOledDisplay() function
// Change the first section to display MQTT status
// Line 1: Main Status (Large Text)
display.setTextSize(2);
display.setTextColor(SSD1_WHITE);
display.setCursor(0, 0);
display.println(test_item); // This will now display the "MQTT: Connected" status
```

```
```