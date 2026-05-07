# Manual Dashboard Guide

คู่มือนี้เป็นแนวทางสร้าง dashboard ใหม่ด้วย MQTT topic ปัจจุบัน ไม่ผูกกับแพลตฟอร์มใดแพลตฟอร์มหนึ่ง

## Data Source

ตั้ง MQTT connection:

```text
Host: broker.hivemq.com
Port: 1883
Auth: none
Protocol: MQTT
```

## Subscribe Topics

```text
device/esp32-devkit-01/telemetry
device/esp32-devkit-01/status
device/esp32-devkit-01/relay/+/status
```

## Publish Topics

```text
device/esp32-devkit-01/control/relay/1
device/esp32-devkit-01/control/relay/2
device/esp32-devkit-01/control/relay/3
device/esp32-devkit-01/control/relay/all
```

## Suggested Layout

```text
┌──────────────────────────────────────────────┐
│ ESP32 MQTT Status                            │
├───────────────┬───────────────┬──────────────┤
│ Temperature   │ Humidity      │ AQI          │
├───────────────┴───────────────┴──────────────┤
│ PM2.5 / PM10                                  │
├───────────────┬───────────────┬──────────────┤
│ Relay 1       │ Relay 2       │ Relay 3      │
├───────────────┴───────────────┴──────────────┤
│ Relay All ON/OFF                              │
└──────────────────────────────────────────────┘
```

## Widget Mapping

| Widget | Topic/Field |
|---|---|
| Device online | `device/esp32-devkit-01/status` |
| Temperature | telemetry `temperature` |
| Humidity | telemetry `humidity` |
| AQI | telemetry `aqi` |
| PM2.5 | telemetry `pm2_5` |
| PM10 | telemetry `pm10` |
| Weather coordinates | telemetry `weather_lat`, `weather_lon` |
| Relay 1 state | telemetry `relay1` or feedback `relay/1/status` |
| Relay 2 state | telemetry `relay2` or feedback `relay/2/status` |
| Relay 3 state | telemetry `relay3` or feedback `relay/3/status` |

## Relay Controls

Relay 1 ON:

```text
Topic: device/esp32-devkit-01/control/relay/1
Payload: {"state": true}
```

Relay 1 OFF:

```text
Topic: device/esp32-devkit-01/control/relay/1
Payload: {"state": false}
```

Relay 1-3 ON:

```text
Topic: device/esp32-devkit-01/control/relay/all
Payload: true
```

Relay 1-3 OFF:

```text
Topic: device/esp32-devkit-01/control/relay/all
Payload: false
```

## Node-RED Dashboard Hint

ถ้าใช้ Node-RED:

- ใช้ `mqtt in` สำหรับ telemetry และ feedback
- ใช้ `json` node เพื่อ parse telemetry
- ใช้ `ui_gauge` หรือ `ui_chart` สำหรับ temperature, humidity, AQI, PM2.5
- ใช้ `ui_switch` แล้วต่อ `mqtt out` ไปยัง relay command topic
- Relay all ใช้ switch แยก topic `control/relay/all`

## Validation Checklist

| ตรวจสอบ | ผลที่ควรได้ |
|---|---|
| Telemetry subscribe | ได้ JSON ทุก 30 วินาที |
| Relay switch | Relay เปลี่ยนสถานะจริง |
| Relay feedback | ได้ status topic กลับมา |
| Relay all | Relay 1-3 เปลี่ยนพร้อมกัน |
| Weather/AQI | มี temperature, humidity, AQI, PM2.5 |
| OLED | แสดง WiFi, MQTT, RLY, Weather, AQI |
