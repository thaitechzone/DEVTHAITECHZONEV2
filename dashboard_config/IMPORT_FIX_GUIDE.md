# Dashboard Import Fix Guide

เอกสารเดิมสำหรับแก้ import ThingsBoard ไม่ตรงกับ firmware ปัจจุบันแล้ว เพราะ firmware เปลี่ยนมาใช้ MQTT topics โดยตรงแทน RPC ของ ThingsBoard

## What Changed

เดิม:

- ใช้ ThingsBoard RPC เช่น `setRelay`, `setAux`, `setMode`
- ใช้ telemetry แบบ ThingsBoard
- มี manual/auto mode ใน dashboard

ปัจจุบัน:

- ใช้ MQTT broker `broker.hivemq.com`
- ควบคุม Relay ผ่าน topic `device/{boardID}/control/relay/{1|2|3|all}`
- รับ telemetry ผ่าน topic `device/{boardID}/telemetry`
- OLED แสดงเฉพาะสถานะหลัก ไม่แสดง AUX/Switch/ISO แล้ว
- AQI ใช้พิกัดที่ได้จาก `WEATHER_CITY`

## If You Still Import Old Dashboard

หลัง import dashboard เก่า ให้ตรวจสอบและแก้เอง:

| รายการเก่า | แนวทางใหม่ |
|---|---|
| RPC `setRelay` | Publish MQTT topic `control/relay/{n}` |
| RPC `setAux` | ยังไม่มี topic command ใน firmware ปัจจุบัน |
| RPC `setMode` | ไม่มี manual/auto mode ใน firmware ปัจจุบัน |
| Input status widget | ไม่ได้อยู่ใน telemetry ปัจจุบัน |
| Weather city text | ใช้ `temperature`, `humidity`, `weather_lat`, `weather_lon` |
| AQI text | ใช้ `aqi`, `pm2_5`, `pm10`, `quality` |

## Minimal Dashboard Test

1. Subscribe telemetry:

```text
device/esp32-devkit-01/telemetry
```

2. Publish Relay 1 ON:

```text
Topic: device/esp32-devkit-01/control/relay/1
Payload: true
```

3. Publish Relay all OFF:

```text
Topic: device/esp32-devkit-01/control/relay/all
Payload: false
```

4. Subscribe feedback:

```text
device/esp32-devkit-01/relay/+/status
```

## Recommendation

สร้าง dashboard ใหม่จาก MQTT topic mapping ใน [README.md](README.md) ของโฟลเดอร์นี้ จะปลอดภัยกว่าแก้ import ThingsBoard เดิมทีละจุด
