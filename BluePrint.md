# ESP32 Devkit V2 Blueprint

เอกสารนี้สรุปภาพรวม firmware ปัจจุบันของ `src/main.cpp`

## Architecture

โปรแกรมทำงานแบบ loop หลักบน Arduino framework โดยแบ่งงานตาม timer:

- WiFi connection และ reconnect ผ่าน `connectWiFi()`
- NTP sync สำหรับเวลา Bangkok GMT+7
- Weather update ทุก 10 นาทีผ่าน OpenWeatherMap Current Weather API
- AQI update ทุก 10 นาทีผ่าน OpenWeatherMap Air Pollution API
- MQTT reconnect ทุก 5 วินาทีเมื่อหลุด
- Telemetry publish ทุก 10 วินาที
- OLED refresh เมื่อข้อมูลสำคัญเปลี่ยน หรือทุก 1 วินาทีเมื่อมีเวลา NTP

## Libraries

```ini
lib_deps =
  adafruit/Adafruit GFX Library
  adafruit/Adafruit SSD1306
  bblanchon/ArduinoJson@^6.19.4
  knolleary/PubSubClient@^2.8
```

## Pin Map

```cpp
#define SW1_PIN 34
#define SW2_PIN 35
#define SW3_PIN 32

#define ISOIN1_PIN 33
#define ISOIN2_PIN 27

#define LED_PIN 2
#define RL1_PIN 17
#define RL2_PIN 16
#define RL3_PIN 4

#define AUX1_PIN 12
#define AUX2_PIN 13
#define AUX3_PIN 14
#define AUX4_PIN 15
```

Relay และ input เป็น Active Low:

- Relay ON = `LOW`
- Relay OFF = `HIGH`
- Input active/pressed = `LOW`

## Configuration

```cpp
const char* WIFI_SSID = "myHome_2.4GHz";
const char* WIFI_PASSWORD = "0939391546";

const char* OPENWEATHERMAP_API_KEY = "...";
const char* WEATHER_CITY = "Tha Sala,Nakhon Si Thammarat,TH";

const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_BOARD_ID = "esp32-devkit-01";
```

`WEATHER_CITY` เก็บเป็นข้อความปกติ และใช้ `encodeUrlSpaces()` แปลงช่องว่างเป็น `%20` เฉพาะตอนสร้าง URL

## Weather And AQI Flow

1. `fetchWeatherData()` เรียก Current Weather API ด้วย `WEATHER_CITY`
2. อ่านค่า `name`, `main.temp`, `main.humidity`, `weather[0].description`, `coord.lat`, `coord.lon`
3. เก็บพิกัดใน `current_weather.latitude` และ `current_weather.longitude`
4. `fetchAirQualityData()` ใช้พิกัดจาก weather เพื่อเรียก Air Pollution API
5. ถ้า weather ยังไม่สำเร็จ จะใช้ `DEFAULT_WEATHER_LAT/LON` เป็น fallback

## MQTT Flow

เมื่อเชื่อมต่อ MQTT สำเร็จ โปรแกรม subscribe topic:

```text
device/{boardID}/control/relay/1
device/{boardID}/control/relay/2
device/{boardID}/control/relay/3
device/{boardID}/control/relay/all
```

Command payload:

```json
{"state": true}
```

รองรับข้อความสั้น `true`, `false`, `on`, `off`, `1`, `0`

Feedback:

```text
device/{boardID}/relay/1/status
device/{boardID}/relay/2/status
device/{boardID}/relay/3/status
device/{boardID}/relay/all/status
```

## OLED Layout

จอ OLED 128x64 แสดงเฉพาะข้อมูลที่ใช้งานจริง:

```text
ESP32 MQTT      HH:MM
WiFi:-54dBm     MQ:OK
RLY:010 Topic:/relay
T:29C H:74% AQI:2
PM2.5:12
PUB:10s
```

ตัด AUX, Switch และ ISO input ออกจาก OLED แล้ว แต่ข้อมูล hardware เหล่านี้ยังอยู่ใน firmware สำหรับใช้งานต่อได้ในอนาคต

## Telemetry

Topic:

```text
device/{boardID}/telemetry
```

Fields หลัก:

- `aqi`, `pm2_5`, `pm10`, `quality`
- `temperature`, `humidity`
- `weather_lat`, `weather_lon`
- `relay1`, `relay2`, `relay3`
- `timestamp`
- `rssi`
- `uptime_seconds`

## Build Verification

คำสั่งตรวจสอบ:

```bash
pio run
```

หรือบนเครื่องนี้:

```powershell
& $env:USERPROFILE\.platformio\penv\Scripts\platformio.exe run
```
