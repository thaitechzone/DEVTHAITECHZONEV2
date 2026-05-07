# ESP32 Devkit V2

โปรเจกต์ PlatformIO สำหรับบอร์ด ESP32 Devkit V2 ที่เชื่อมต่อ WiFi, MQTT, เวลา NTP, OpenWeatherMap Weather/Air Quality และแสดงสถานะหลักบน OLED SSD1306 128x64

## สถานะระบบปัจจุบัน

- Framework: Arduino บน PlatformIO
- Board: `esp32dev`
- MQTT broker: `broker.hivemq.com:1883`
- Board ID: `esp32-devkit-01`
- Telemetry interval: ทุก 10 วินาที
- Weather/AQI interval: ทุก 10 นาที
- OLED: แสดง WiFi, MQTT, Relay 1-3, Weather, AQI, PM2.5 และสถานะ publish

## Hardware

| กลุ่ม | GPIO | หมายเหตุ |
|---|---:|---|
| Relay 1 | 17 | Active Low |
| Relay 2 | 16 | Active Low |
| Relay 3 | 4 | Active Low |
| AUX 1 | 12 | TTL 3.3V output |
| AUX 2 | 13 | TTL 3.3V output |
| AUX 3 | 14 | TTL 3.3V output |
| AUX 4 | 15 | TTL 3.3V output |
| SW1 | 34 | Active Low input |
| SW2 | 35 | Active Low input |
| SW3 | 32 | Active Low input |
| ISO IN 1 | 33 | Active Low input |
| ISO IN 2 | 27 | Active Low input |
| Onboard LED | 2 | Status LED |

## OLED Wiring

```text
OLED SDA -> GPIO 21
OLED SCL -> GPIO 22
OLED VCC -> 3.3V
OLED GND -> GND
```

OLED เป็นอุปกรณ์เสริม ถ้าไม่พบจอ โปรแกรมจะทำงานต่อและดูสถานะผ่าน Serial Monitor ได้

## Configuration

แก้ค่าหลักใน [src/main.cpp](src/main.cpp)

```cpp
const char* WIFI_SSID = "your-wifi";
const char* WIFI_PASSWORD = "your-password";

const char* OPENWEATHERMAP_API_KEY = "your-api-key";
const char* WEATHER_CITY = "Tha Sala,Nakhon Si Thammarat,TH";
```

`WEATHER_CITY` ใช้ข้อความเมืองแบบปกติ โปรแกรมจะแปลงช่องว่างเป็น `%20` ตอนสร้าง URL เอง และจะใช้พิกัด `coord.lat/lon` จาก Weather API ไปเรียก Air Pollution API อัตโนมัติ

ค่า fallback ใช้เฉพาะกรณีดึง weather ไม่สำเร็จ:

```cpp
const float DEFAULT_WEATHER_LAT = 8.4304;
const float DEFAULT_WEATHER_LON = 99.9631;
```

## MQTT Topics

| ประเภท | Topic |
|---|---|
| Telemetry | `device/esp32-devkit-01/telemetry` |
| Online status | `device/esp32-devkit-01/status` |
| Control Relay 1 | `device/esp32-devkit-01/control/relay/1` |
| Control Relay 2 | `device/esp32-devkit-01/control/relay/2` |
| Control Relay 3 | `device/esp32-devkit-01/control/relay/3` |
| Control Relay 1-3 | `device/esp32-devkit-01/control/relay/all` |
| Relay feedback | `device/esp32-devkit-01/relay/+/status` |

Payload สำหรับ relay ใช้ได้ทั้ง JSON และข้อความสั้น:

```json
{"state": true}
```

หรือ `true`, `false`, `on`, `off`, `1`, `0`

## Telemetry Example

```json
{
  "aqi": 2,
  "pm2_5": 12.4,
  "pm10": 20.1,
  "quality": "Fair",
  "temperature": 29,
  "humidity": 74,
  "weather_lat": 8.667,
  "weather_lon": 99.917,
  "relay1": false,
  "relay2": true,
  "relay3": false,
  "timestamp": "2026-05-07 21:30:00",
  "rssi": -54,
  "uptime_seconds": 3600
}
```

## OLED Layout

```text
ESP32 MQTT      21:30
WiFi:-54dBm     MQ:OK
RLY:010 Topic:/relay
T:29C H:74% AQI:2
PM2.5:12
PUB:10s
```

Relay state ใช้ `1` = ON และ `0` = OFF

## Build And Upload

```bash
pio run
pio run --target upload
pio device monitor
```

ถ้า `pio` ไม่อยู่ใน PATH บนเครื่องนี้ สามารถใช้ PlatformIO executable ใน user profile:

```powershell
& $env:USERPROFILE\.platformio\penv\Scripts\platformio.exe run
```

## Related Docs

- [MQTT_TESTING_GUIDE.md](MQTT_TESTING_GUIDE.md) - วิธีทดสอบ MQTT ด้วย MQTT Explorer และ mosquitto
- [BluePrint.md](BluePrint.md) - สรุปสถาปัตยกรรม firmware
- [dashboard_config/README.md](dashboard_config/README.md) - สถานะ dashboard config และ telemetry mapping
