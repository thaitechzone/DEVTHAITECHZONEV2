# MQTT Testing Guide

คู่มือทดสอบ MQTT สำหรับ ESP32 Devkit V2 firmware ปัจจุบัน

## Connection

| รายการ | ค่า |
|---|---|
| Broker | `broker.hivemq.com` |
| Port | `1883` |
| Protocol | MQTT v3.1.1 |
| Auth | ไม่ใช้ |
| Board ID | `esp32-devkit-01` |

## Tools

- MQTT Explorer: <https://mqtt-explorer.com/>
- Mosquitto CLI: `mosquitto_sub`, `mosquitto_pub`
- Node-RED MQTT nodes

## MQTT Explorer Steps

1. เปิด MQTT Explorer แล้วกด `+` เพื่อสร้าง connection ใหม่
2. ตั้งค่า `Name` เป็น `HiveMQ Public`
3. ตั้ง `Host` เป็น `broker.hivemq.com`
4. ตั้ง `Port` เป็น `1883`
5. ไม่ต้องใส่ Username และ Password
6. กด Connect
7. ดู topic tree ด้านซ้าย ควรเห็น `device/esp32-devkit-01/...` หลังบอร์ด online

### ดู Telemetry ด้วย MQTT Explorer

1. เลือก topic `device/esp32-devkit-01/telemetry`
2. รอ payload JSON ล่าสุดจากบอร์ด
3. ตรวจสอบค่า `aqi`, `temperature`, `humidity`, `relay1`, `relay2`, `relay3`
4. ยืนยันว่า timestamp และ RSSI อัปเดตตามรอบ publish

### สั่ง Relay ด้วย MQTT Explorer

1. เลือก topic `device/esp32-devkit-01/control/relay/1`
2. กดปุ่ม publish หรือ Add message
3. ใส่ payload เป็น `true` แล้วส่ง
4. ตรวจสอบว่า Relay 1 ทำงาน และมี feedback ที่ `device/esp32-devkit-01/relay/1/status`
5. ทดสอบซ้ำด้วย payload `false`
6. ทำซ้ำกับ relay `2`, `3` หรือ topic `device/esp32-devkit-01/control/relay/all`

### ค่าที่ควรเห็นเมื่อทดสอบผ่าน MQTT Explorer

- Topic `telemetry` มี JSON ใหม่ตามรอบเวลา
- Topic `status` แสดง board online
- Topic `relay/1/status`, `relay/2/status`, `relay/3/status` ตอบกลับหลังสั่งงาน
- สถานะ relay บนจอ OLED และ Serial ตรงกับ MQTT

## Serial Check

หลังบอร์ดต่อ WiFi และ MQTT สำเร็จ ควรเห็นข้อความประมาณนี้ใน Serial Monitor:

```text
MQTT Connected!
Board ID: esp32-devkit-01
Subscribed to relay control topics
Relay all topic: device/esp32-devkit-01/control/relay/all
```

## Subscribe Telemetry

Topic:

```text
device/esp32-devkit-01/telemetry
```

Mosquitto:

```bash
mosquitto_sub -h broker.hivemq.com -t "device/esp32-devkit-01/telemetry" -v
```

ตัวอย่าง payload:

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

## Relay Control Topics

| Relay | Topic |
|---|---|
| Relay 1 | `device/esp32-devkit-01/control/relay/1` |
| Relay 2 | `device/esp32-devkit-01/control/relay/2` |
| Relay 3 | `device/esp32-devkit-01/control/relay/3` |
| Relay 1-3 พร้อมกัน | `device/esp32-devkit-01/control/relay/all` |

Payload ที่รองรับ:

```json
{"state": true}
```

หรือ:

```text
true
false
on
off
1
0
```

## Command Examples

เปิด Relay 1:

```bash
mosquitto_pub -h broker.hivemq.com -t "device/esp32-devkit-01/control/relay/1" -m "true"
```

ปิด Relay 1:

```bash
mosquitto_pub -h broker.hivemq.com -t "device/esp32-devkit-01/control/relay/1" -m "false"
```

เปิด Relay ทั้ง 3 ตัว:

```bash
mosquitto_pub -h broker.hivemq.com -t "device/esp32-devkit-01/control/relay/all" -m "true"
```

ปิด Relay ทั้ง 3 ตัว:

```bash
mosquitto_pub -h broker.hivemq.com -t "device/esp32-devkit-01/control/relay/all" -m "false"
```

## Feedback

Subscribe:

```bash
mosquitto_sub -h broker.hivemq.com -t "device/esp32-devkit-01/relay/+/status" -v
```

ตัวอย่าง:

```text
device/esp32-devkit-01/relay/1/status {"state":true}
device/esp32-devkit-01/relay/all/status {"relay1":true,"relay2":true,"relay3":true}
```

## Online Status

Topic:

```text
device/esp32-devkit-01/status
```

Payload:

```json
{"board_id":"esp32-devkit-01","status":"online"}
```

## MQTT Explorer Tree

```text
device
└── esp32-devkit-01
    ├── telemetry
    ├── status
    ├── control
    │   └── relay
    │       ├── 1
    │       ├── 2
    │       ├── 3
    │       └── all
    └── relay
        ├── 1
        │   └── status
        ├── 2
        │   └── status
        ├── 3
        │   └── status
        └── all
            └── status
```

## Checklist

| ทดสอบ | วิธี | ผลที่คาดหวัง |
|---|---|---|
| MQTT connect | ดู Serial | เห็น `MQTT Connected!` |
| Telemetry | Subscribe telemetry | ได้ JSON ทุก 10 วินาที |
| Relay 1 ON/OFF | Publish topic relay/1 | Relay 1 เปลี่ยนสถานะและมี feedback |
| Relay 2 ON/OFF | Publish topic relay/2 | Relay 2 เปลี่ยนสถานะและมี feedback |
| Relay 3 ON/OFF | Publish topic relay/3 | Relay 3 เปลี่ยนสถานะและมี feedback |
| Relay ALL | Publish topic relay/all | Relay 1-3 เปลี่ยนพร้อมกัน |
| OLED | ดูหน้าจอ | WiFi, MQTT, RLY, Weather, AQI ตรงกับระบบ |

## Troubleshooting

| อาการ | ตรวจสอบ |
|---|---|
| MQTT ไม่ต่อ | WiFi, port 1883, broker `broker.hivemq.com` |
| Relay ไม่ตอบสนอง | Topic ต้องตรง, payload ต้องเป็นค่าที่รองรับ |
| ไม่มี telemetry | MQTT connection, `mqttClient.loop()`, interval 10 วินาที |
| AQI ไม่มีข้อมูล | Weather API key, internet, fallback lat/lon |
| OLED ไม่ขึ้น | I2C wiring, address `0x3C`; firmware ยังทำงานต่อได้ |
