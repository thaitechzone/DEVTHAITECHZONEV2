# Dashboard Config Notes

โฟลเดอร์นี้ยังเก็บไฟล์ JSON dashboard เดิมไว้เป็น reference แต่ firmware ปัจจุบันไม่ได้ใช้ ThingsBoard RPC แล้ว ระบบควบคุมหลักเปลี่ยนเป็น MQTT topic ตรงผ่าน HiveMQ

## Current MQTT Mapping

| งาน | Topic |
|---|---|
| Telemetry | `device/esp32-devkit-01/telemetry` |
| Device status | `device/esp32-devkit-01/status` |
| Relay 1 command | `device/esp32-devkit-01/control/relay/1` |
| Relay 2 command | `device/esp32-devkit-01/control/relay/2` |
| Relay 3 command | `device/esp32-devkit-01/control/relay/3` |
| Relay all command | `device/esp32-devkit-01/control/relay/all` |
| Relay feedback | `device/esp32-devkit-01/relay/+/status` |

## Telemetry Fields

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

## Recommended Dashboard Widgets

สำหรับ dashboard ใหม่ ไม่ว่าจะทำใน Node-RED Dashboard, ThingsBoard custom integration, Grafana หรือ web app เอง แนะนำให้มี:

| Widget | Source |
|---|---|
| MQTT connection/status | `device/esp32-devkit-01/status` |
| Temperature | telemetry `temperature` |
| Humidity | telemetry `humidity` |
| AQI | telemetry `aqi` |
| PM2.5 | telemetry `pm2_5` |
| Relay 1 switch | publish `control/relay/1` |
| Relay 2 switch | publish `control/relay/2` |
| Relay 3 switch | publish `control/relay/3` |
| Relay all switch | publish `control/relay/all` |
| Relay feedback | subscribe `relay/+/status` |

## Relay Command Payloads

เปิด:

```json
{"state": true}
```

ปิด:

```json
{"state": false}
```

ข้อความสั้นที่ firmware รองรับ: `true`, `false`, `on`, `off`, `1`, `0`

## Legacy Files

- `esp32_dashboard.json`
- `esp32_dashboard_v2.json`
- `IMPORT_FIX_GUIDE.md`
- `MANUAL_DASHBOARD_GUIDE.md`

ไฟล์เหล่านี้ควรถือเป็น reference/legacy จนกว่าจะสร้าง dashboard รุ่นใหม่ที่ใช้ MQTT topic ปัจจุบันโดยตรง
