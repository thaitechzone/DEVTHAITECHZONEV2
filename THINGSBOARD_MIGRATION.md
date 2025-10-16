# Migration จาก Public MQTT เป็น ThingsBoard
## ESP32 ThaiTechZone V2.0 - ThingsBoard Integration

---

## 📋 สรุปการเปลี่ยนแปลง

### เปลี่ยนจาก
- ❌ **Public MQTT Broker** (HiveMQ)
- ❌ Custom Topics (`thaitechzone/v2/...`)
- ❌ ไม่มี Dashboard
- ❌ ไม่มี Device Management
- ❌ ไม่เก็บประวัติข้อมูล

### เปลี่ยนเป็น
- ✅ **ThingsBoard IoT Platform**
- ✅ Standard Topics (ThingsBoard format)
- ✅ มี Dashboard สวยงาม
- ✅ Device Management
- ✅ เก็บ Time-series data
- ✅ Rule Engine & Alarms
- ✅ Mobile App

---

## 🔄 การเปลี่ยนแปลงในโค้ด

### 1. MQTT Configuration

#### ก่อน (Public Broker):
```cpp
const char* MQTT_BROKER = "broker.hivemq.com";
const char* MQTT_USER = "";
const char* MQTT_PASSWORD = "";

// Custom Topics
const char* MQTT_TOPIC_STATUS = "thaitechzone/v2/status";
const char* MQTT_TOPIC_RELAY_CMD = "thaitechzone/v2/relay/cmd";
```

#### หลัง (ThingsBoard):
```cpp
const char* MQTT_BROKER = "demo.thingsboard.io";
const char* MQTT_USER = "YOUR_DEVICE_ACCESS_TOKEN";
const char* MQTT_PASSWORD = "";

// ThingsBoard Standard Topics
const char* TB_TOPIC_TELEMETRY = "v1/devices/me/telemetry";
const char* TB_TOPIC_RPC_REQUEST = "v1/devices/me/rpc/request/+";
```

---

### 2. MQTT Topics Structure

#### ก่อน (Custom Topics):
```
Publish:
  - thaitechzone/v2/status          → System status
  - thaitechzone/v2/inputs          → Input states
  - thaitechzone/v2/weather         → Weather data
  - thaitechzone/v2/airquality      → Air quality

Subscribe:
  - thaitechzone/v2/mode/cmd        → Mode control
  - thaitechzone/v2/relay/cmd       → Relay control
  - thaitechzone/v2/aux/cmd         → AUX control
```

#### หลัง (ThingsBoard):
```
Publish:
  - v1/devices/me/telemetry         → All telemetry (unified)
  - v1/devices/me/attributes        → Device attributes

Subscribe:
  - v1/devices/me/rpc/request/+     → All RPC commands
  
Respond:
  - v1/devices/me/rpc/response/{id} → RPC response
```

---

### 3. Data Format

#### ก่อน (Nested JSON):
```json
{
  "relays": {
    "rl1": true,
    "rl2": false,
    "rl3": false
  },
  "aux": {
    "aux1": false
  }
}
```

#### หลัง (Flat JSON - ThingsBoard format):
```json
{
  "relay1": true,
  "relay2": false,
  "relay3": false,
  "aux1": false,
  "temperature": 28,
  "humidity": 75,
  "pm2_5": 35.2
}
```

**เหตุผล:** ThingsBoard ใช้ Flat JSON เพื่อให้ Query และสร้าง Widget ได้ง่ายขึ้น

---

### 4. Control Commands

#### ก่อน (Custom Commands):
```json
Topic: thaitechzone/v2/relay/cmd
Payload: {"relay": 1, "state": true}
```

#### หลัง (RPC - Remote Procedure Call):
```json
Topic: v1/devices/me/rpc/request/123
Payload: {
  "method": "setRelay",
  "params": {
    "relay": 1,
    "state": true
  }
}

Response Topic: v1/devices/me/rpc/response/123
Response: {
  "relay": 1,
  "state": true
}
```

**ข้อดี:** 
- มี Request ID สำหรับ tracking
- มี Response เพื่อ confirm
- รองรับ Multiple methods

---

## 🎯 RPC Methods ที่เพิ่ม

| Method | Description | Parameters | Response |
|--------|-------------|------------|----------|
| `setMode` | สลับ Auto/Manual | `mode`: "auto"/"manual" | `{"mode": "manual"}` |
| `setRelay` | ควบคุม Relay | `relay`: 1-3, `state`: bool | `{"relay": 1, "state": true}` |
| `setAux` | ควบคุม AUX | `aux`: 1-4, `state`: bool | `{"aux": 2, "state": false}` |
| `getValue` | อ่านสถานะ | - | `{"mode": "manual", "uptime": 12345, ...}` |
| `turnOffAll` | ปิดทั้งหมด | - | `{"message": "All outputs turned off"}` |

---

## 📊 Telemetry Keys

### ข้อมูลที่ส่งไปยัง ThingsBoard

```cpp
// System Info
"uptime"              → เวลาทำงาน (seconds)
"mode"                → "auto" หรือ "manual"
"wifi_rssi"           → ความแรงสัญญาณ WiFi (dBm)
"free_heap"           → RAM ว่าง (bytes)

// Outputs
"relay1", "relay2", "relay3"        → สถานะ Relay (boolean)
"aux1", "aux2", "aux3", "aux4"      → สถานะ AUX (boolean)

// Inputs
"switch1", "switch2", "switch3"     → สถานะ Switch (boolean)
"iso_input1", "iso_input2"          → สถานะ Isolated Input (boolean)

// Weather
"temperature"         → อุณหภูมิ (°C)
"humidity"            → ความชื้น (%)
"weather_city"        → ชื่อเมือง
"weather_description" → คำอธิบายอากาศ

// Air Quality
"pm2_5"               → PM2.5 (µg/m³)
"pm10"                → PM10 (µg/m³)
"aqi"                 → Air Quality Index (1-5)
"air_quality"         → คำอธิบาย (Good, Fair, ...)
```

**Update Interval:** ทุก 5 วินาที

---

## 🔧 ขั้นตอนการใช้งาน

### 1. สร้าง ThingsBoard Account
- Demo: [https://demo.thingsboard.io](https://demo.thingsboard.io)
- Cloud: [https://thingsboard.cloud](https://thingsboard.cloud)

### 2. สร้าง Device
```
Name: ESP32_ThaiTechZone_V2
Device profile: Default
```

### 3. Copy Access Token
```
Example: A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6
```

### 4. แก้ไขโค้ด
```cpp
// src/main.cpp line 36
const char* MQTT_USER = "A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6";
```

### 5. Upload โค้ด
```bash
PlatformIO: Upload
```

### 6. ตรวจสอบ Serial Monitor
```
ThingsBoard Connected!
Subscribed to RPC commands
✅ Telemetry sent to ThingsBoard
```

### 7. ตรวจสอบ ThingsBoard
- Device Details → Latest telemetry
- ดูข้อมูลแบบ Real-time

### 8. สร้าง Dashboard
- Dashboards → Add new dashboard
- เพิ่ม Widgets (Charts, Gauges, Controls)

---

## 📱 ตัวอย่าง Dashboard Widgets

### Widget 1: Current Values
```
Type: Entities table
Telemetry:
  - temperature
  - humidity
  - pm2_5
  - aqi
  - wifi_rssi
  - uptime
```

### Widget 2: Temperature Chart
```
Type: Timeseries - Line Chart
Datasource: temperature
Timewindow: Last 24 hours
```

### Widget 3: Relay Control Switch
```
Type: Switch Control
RPC method: setRelay
Params (ON): {"relay": 1, "state": true}
Params (OFF): {"relay": 1, "state": false}
```

### Widget 4: PM2.5 Gauge
```
Type: Radial gauge
Datasource: pm2_5
Min: 0, Max: 100
Thresholds:
  - 0-12: Green (Good)
  - 12-35: Yellow (Moderate)
  - 35-55: Orange (Unhealthy)
  - 55+: Red (Hazardous)
```

---

## 🚨 Rule Engine Example

### Alarm: PM2.5 สูงเกิน 35

```javascript
// Rule Chain: PM2.5 Alert
// Script Node
if (msg.pm2_5 > 35) {
  return {
    msg: msg,
    metadata: metadata,
    msgType: "PM2.5_HIGH_ALARM"
  };
}
return {msg: msg, metadata: metadata, msgType: "OK"};
```

**Actions:**
- Create Alarm (Severity: WARNING)
- Send Email notification
- Send Push notification to Mobile

---

## 🔍 Troubleshooting

### ❌ Problem: Connection Failed, rc=-2

**สาเหตุ:**
- Access Token ไม่ถูกต้อง

**แก้ไข:**
1. Login ThingsBoard
2. Devices → ESP32_ThaiTechZone_V2 → Details
3. Copy access token
4. แก้ไขในโค้ด:
   ```cpp
   const char* MQTT_USER = "CORRECT_TOKEN_HERE";
   ```
5. Upload ใหม่

---

### ❌ Problem: ไม่เห็นข้อมูลใน Dashboard

**สาเหตุ:**
- Widget ไม่ได้ระบุ Device
- Telemetry key ไม่ตรง

**แก้ไข:**
1. Edit Widget
2. เลือก Device: `ESP32_ThaiTechZone_V2`
3. เลือก Telemetry key ที่ถูกต้อง (เช่น `temperature`)
4. Save Widget

---

### ❌ Problem: RPC ไม่ทำงาน

**สาเหตุ:**
- อยู่ใน Auto mode

**แก้ไข:**
1. ส่ง RPC: `setMode` → `{"mode":"manual"}`
2. จากนั้นค่อยควบคุม Relay/AUX

---

## 📈 Performance Comparison

### Before (Public MQTT)
```
- Dashboard: None
- Data Storage: None
- Visualization: None
- Alarms: None
- Mobile App: None
- Rule Engine: None
- Security: Public (no authentication)
```

### After (ThingsBoard)
```
- Dashboard: ✅ Customizable, Responsive
- Data Storage: ✅ Time-series database
- Visualization: ✅ Charts, Gauges, Maps
- Alarms: ✅ Real-time alerts
- Mobile App: ✅ Android/iOS
- Rule Engine: ✅ Automation
- Security: ✅ Access Token, SSL/TLS
```

---

## 🎓 สิ่งที่เรียนรู้

### ThingsBoard Concepts
- ✅ Device Management
- ✅ Telemetry vs Attributes
- ✅ RPC (Remote Procedure Call)
- ✅ Rule Engine
- ✅ Alarms & Notifications
- ✅ Dashboard creation

### MQTT Best Practices
- ✅ Flat JSON format
- ✅ Request/Response pattern
- ✅ Access Token authentication
- ✅ QoS levels
- ✅ Retained messages

---

## 🏆 ข้อดีของ ThingsBoard

### ✅ Professional IoT Platform
- Enterprise-grade
- Production-ready
- Scalable (หลายพัน devices)

### ✅ Complete Solution
- Dashboard + Mobile + API
- All-in-one platform

### ✅ Data Management
- Time-series database
- Query historical data
- Export to CSV/JSON

### ✅ Automation
- Rule Engine
- Alarms
- Notifications (Email, SMS, Push)

### ✅ Security
- Access Token per device
- SSL/TLS support
- Role-based access control

---

## 📚 เอกสารที่เกี่ยวข้อง

1. **THINGSBOARD_SETUP.md** - คู่มือการตั้งค่าโดยละเอียด
2. **MQTT_GUIDE.md** - คู่มือ MQTT (ฉบับเดิม)
3. **MQTT_CHANGELOG.md** - สรุปการเปลี่ยนแปลง
4. **README.md** - เอกสารหลัก
5. **BluePrint.md** - Technical spec

---

## 🚀 Next Steps

### Immediate (ทำได้ทันที)
- [ ] สร้าง ThingsBoard account
- [ ] เพิ่ม Device และ copy token
- [ ] แก้ไขโค้ด ESP32
- [ ] Upload และทดสอบ
- [ ] สร้าง Basic Dashboard

### Short-term (1-2 สัปดาห์)
- [ ] สร้าง Dashboard สมบูรณ์
- [ ] ตั้งค่า Alarms สำหรับ PM2.5
- [ ] สร้าง Rule Chain
- [ ] ทดสอบควบคุมผ่าน Mobile App

### Long-term (1-3 เดือน)
- [ ] ใช้ ThingsBoard Cloud (แทน Demo)
- [ ] เพิ่ม Device หลายตัว
- [ ] Integration กับระบบอื่น (REST API)
- [ ] สร้าง Custom Widget
- [ ] ตั้งค่า Email/SMS notification

---

## 💰 Cost Comparison

### Public MQTT Broker (HiveMQ)
```
Cost: ฟรี
Limitations: 
  - ไม่มี Dashboard
  - ไม่เก็บข้อมูล
  - Public (ไม่ปลอดภัย)
  - ไม่มี Support
```

### ThingsBoard Demo
```
Cost: ฟรี
Limitations:
  - ข้อมูลอาจถูกลบ
  - ใช้ทดสอบเท่านั้น
```

### ThingsBoard Cloud - Maker Plan
```
Cost: ฟรี
Features:
  - 50 Devices
  - 10 Dashboards
  - 1 GB Storage
  - Email Support
```

### ThingsBoard Cloud - Prototype Plan
```
Cost: $20/month
Features:
  - 500 Devices
  - Unlimited Dashboards
  - 10 GB Storage
  - Priority Support
```

**คำแนะนำ:**
- เริ่มต้น: Demo Server (ทดสอบ)
- โปรเจคเล็ก: Maker Plan (ฟรี)
- โปรเจคกลาง-ใหญ่: Prototype Plan ($20/month)
- Enterprise: Self-hosted (ติดตั้งเอง)

---

## 📞 Support & Resources

### Official Documentation
- [ThingsBoard Docs](https://thingsboard.io/docs/)
- [MQTT API](https://thingsboard.io/docs/reference/mqtt-api/)
- [RPC API](https://thingsboard.io/docs/user-guide/rpc/)

### Community
- [ThingsBoard Forum](https://groups.google.com/forum/#!forum/thingsboard)
- [GitHub Issues](https://github.com/thingsboard/thingsboard)
- [YouTube Channel](https://www.youtube.com/c/Thingsboard)

### ThaiTechZone
- Email: support@thaitechzone.com
- Line: @thaitechzone
- Facebook: /thaitechzone

---

## ✅ Checklist การ Migration

- [x] ปรับ MQTT Configuration
- [x] เปลี่ยนจาก Custom Topics เป็น ThingsBoard Topics
- [x] เปลี่ยน Data Format เป็น Flat JSON
- [x] เพิ่ม RPC Methods
- [x] ปรับ Telemetry publishing
- [x] ทดสอบ Compilation
- [ ] สร้าง ThingsBoard Account
- [ ] เพิ่ม Device
- [ ] Copy Access Token
- [ ] แก้ไขโค้ดด้วย Token จริง
- [ ] Upload ไปยัง ESP32
- [ ] ตรวจสอบการเชื่อมต่อ
- [ ] สร้าง Dashboard
- [ ] ทดสอบ RPC Control
- [ ] ตั้งค่า Alarms

---

**สรุป:**
การย้ายจาก Public MQTT ไปยัง ThingsBoard เป็นการ upgrade ที่คุ้มค่ามาก ให้ความสามารถในการจัดการอุปกรณ์ IoT แบบมืออาชีพ พร้อม Dashboard, Data Storage, Alarms และ Automation ที่สมบูรณ์แบบ!

---

**เอกสารนี้จัดทำโดย:** ThaiTechZone Team  
**Version:** 3.0 (ThingsBoard Migration)  
**วันที่:** 16 ตุลาคม 2025  
**Status:** ✅ Ready for ThingsBoard!
