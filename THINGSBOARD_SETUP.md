# คู่มือการตั้งค่า ThingsBoard
## ESP32 ThaiTechZone V2.0 - ThingsBoard IoT Platform Integration

---

## 📋 สารบัญ
1. [ภาพรวม ThingsBoard](#ภาพรวม-thingsboard)
2. [การสร้างบัญชีและ Device](#การสร้างบัญชีและ-device)
3. [การตั้งค่า ESP32](#การตั้งค่า-esp32)
4. [RPC Methods (Remote Control)](#rpc-methods-remote-control)
5. [สร้าง Dashboard](#สร้าง-dashboard)
6. [Rule Engine & Alarms](#rule-engine--alarms)
7. [Troubleshooting](#troubleshooting)

---

## 🎯 ภาพรวม ThingsBoard

### ThingsBoard คืออะไร?
ThingsBoard เป็นแพลตฟอร์ม IoT แบบ Open-source ที่มีความสามารถ:
- ✅ **Device Management**: จัดการอุปกรณ์หลายร้อย-พันตัว
- ✅ **Data Visualization**: Dashboard แบบ Real-time
- ✅ **Rule Engine**: สร้างกฎการทำงานอัตโนมัติ
- ✅ **Alarms**: แจ้งเตือนเมื่อมีเหตุการณ์ผิดปกติ
- ✅ **RPC**: ควบคุมอุปกรณ์จากระยะไกล
- ✅ **RESTful API**: Integration กับระบบอื่น

### ทำไมต้อง ThingsBoard?
เมื่อเทียบกับ Public MQTT Broker (HiveMQ):

| ฟีเจอร์ | Public Broker | ThingsBoard |
|---------|---------------|-------------|
| Dashboard | ❌ | ✅ สวยงาม, Responsive |
| Device Management | ❌ | ✅ จัดการเป็นหมวดหมู่ |
| Data Storage | ❌ | ✅ เก็บประวัติ (Time-series) |
| Visualization | ❌ | ✅ Charts, Gauges, Maps |
| Alarms | ❌ | ✅ แจ้งเตือนแบบ Real-time |
| Rule Engine | ❌ | ✅ Automation ขั้นสูง |
| Security | ⚠️ Public | ✅ Access Token, SSL |
| Mobile App | ❌ | ✅ Android/iOS |

---

## 🚀 การสร้างบัญชีและ Device

### ขั้นตอนที่ 1: สร้างบัญชี ThingsBoard

#### Option 1: ThingsBoard Demo Server (ฟรี - สำหรับทดสอบ)
1. เปิด [https://demo.thingsboard.io/login](https://demo.thingsboard.io/login)
2. คลิก **"Create Account"**
3. กรอกข้อมูล:
   - Email: อีเมลของคุณ
   - First Name: ชื่อ
   - Last Name: นามสกุล
4. คลิก **"Create Account"**
5. ตรวจสอบอีเมลเพื่อ Verify บัญชี

⚠️ **ข้อจำกัด Demo Server:**
- ข้อมูลอาจถูกลบหลัง 3-7 วัน
- ไม่เหมาะสำหรับใช้งานจริง
- ใช้ทดสอบเท่านั้น

#### Option 2: ThingsBoard Cloud (แนะนำสำหรับใช้งานจริง)
1. เปิด [https://thingsboard.cloud](https://thingsboard.cloud)
2. เลือกแผน: **Maker** (ฟรี) หรือ **Professional** (มีค่าใช้จ่าย)
3. สมัครสมาชิก
4. ใช้งานได้ทันที

**แผน Maker (ฟรี):**
- 50 อุปกรณ์
- 10 Dashboard
- 1 GB Storage
- เหมาะสำหรับโปรเจคเล็ก-กลาง

#### Option 3: Self-Hosted (ควบคุมเต็มที่)
สำหรับผู้ที่ต้องการติดตั้งเอง:
```bash
# Ubuntu/Debian
wget https://github.com/thingsboard/thingsboard/releases/download/v3.6/thingsboard-3.6.deb
sudo dpkg -i thingsboard-3.6.deb
sudo systemctl start thingsboard
```

เข้าถึงได้ที่: `http://localhost:9090`
- Username: `tenant@thingsboard.org`
- Password: `tenant`

---

### ขั้นตอนที่ 2: สร้าง Device

1. **Login** เข้า ThingsBoard Dashboard

2. **เปิดเมนู Devices**
   - ไปที่ **Entities** → **Devices**
   - คลิก **"+"** (Add Device)

3. **กรอกข้อมูล Device**
   ```
   Name: ESP32_ThaiTechZone_V2
   Device profile: Default
   Label: IoT Board (ถ้าต้องการ)
   Description: ESP32 Development Board with Relays and Sensors
   ```

4. **คลิก "Add"**

5. **คัดลอก Access Token**
   - คลิกที่ Device ที่สร้าง
   - เปิดแท็บ **"Details"**
   - คลิกที่ **"Copy access token"** หรือ **"Manage credentials"**
   - เก็บ Token ไว้ใช้ในขั้นตอนต่อไป

   ตัวอย่าง Access Token:
   ```
   A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6
   ```

---

## 🔧 การตั้งค่า ESP32

### ขั้นตอนที่ 1: แก้ไข Access Token

เปิดไฟล์ `src/main.cpp` และแก้ไขบรรทัด 36:

```cpp
// ThingsBoard Device Access Token (Get from Device Credentials page)
const char* MQTT_USER = "YOUR_DEVICE_ACCESS_TOKEN";  // <-- แทนที่ตรงนี้
```

**ตัวอย่าง:**
```cpp
const char* MQTT_USER = "A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6";
```

### ขั้นตอนที่ 2: เลือก ThingsBoard Server

ถ้าใช้ Demo Server (ค่าเริ่มต้น):
```cpp
const char* MQTT_BROKER = "demo.thingsboard.io";
```

ถ้าใช้ ThingsBoard Cloud:
```cpp
const char* MQTT_BROKER = "thingsboard.cloud";
```

ถ้าใช้ Self-Hosted:
```cpp
const char* MQTT_BROKER = "192.168.1.100";  // IP ของ Server
```

### ขั้นตอนที่ 3: Compile และ Upload

1. เปิด PlatformIO
2. คลิก **Build** (✓)
3. เชื่อมต่อ ESP32
4. คลิก **Upload** (→)
5. เปิด **Serial Monitor** (🔌)

### ตรวจสอบการเชื่อมต่อ

Serial Monitor ควรแสดง:
```
WiFi Connected!
IP: 192.168.1.xxx
RSSI: -45 dBm

Time Synced!
Date: 16/10/2025
Time: 14:30:25

Connecting to ThingsBoard: demo.thingsboard.io
ThingsBoard Connected!
Subscribed to RPC commands
```

---

## 🎮 RPC Methods (Remote Control)

ThingsBoard ใช้ **RPC (Remote Procedure Call)** สำหรับควบคุมอุปกรณ์จากระยะไกล

### RPC Methods ที่รองรับ

#### 1. **setMode** - สลับโหมด Auto/Manual
```json
{
  "method": "setMode",
  "params": {
    "mode": "manual"
  }
}
```

**Parameters:**
- `mode`: `"auto"` หรือ `"manual"`

**Response:**
```json
{
  "mode": "manual"
}
```

---

#### 2. **setRelay** - ควบคุม Relay (Manual mode เท่านั้น)
```json
{
  "method": "setRelay",
  "params": {
    "relay": 1,
    "state": true
  }
}
```

**Parameters:**
- `relay`: 1, 2, หรือ 3
- `state`: `true` (ON) หรือ `false` (OFF)

**Response:**
```json
{
  "relay": 1,
  "state": true
}
```

---

#### 3. **setAux** - ควบคุม AUX Output (Manual mode เท่านั้น)
```json
{
  "method": "setAux",
  "params": {
    "aux": 2,
    "state": false
  }
}
```

**Parameters:**
- `aux`: 1, 2, 3, หรือ 4
- `state`: `true` (ON) หรือ `false` (OFF)

**Response:**
```json
{
  "aux": 2,
  "state": false
}
```

---

#### 4. **getValue** - อ่านสถานะปัจจุบัน
```json
{
  "method": "getValue",
  "params": {}
}
```

**Response:**
```json
{
  "mode": "manual",
  "uptime": 12345,
  "relays": {
    "rl1": true,
    "rl2": false,
    "rl3": false
  }
}
```

---

#### 5. **turnOffAll** - ปิด Output ทั้งหมด
```json
{
  "method": "turnOffAll",
  "params": {}
}
```

**Response:**
```json
{
  "message": "All outputs turned off"
}
```

---

### ทดสอบ RPC จาก ThingsBoard

1. เปิด **Device Details**
2. คลิกแท็บ **"RPC"**
3. เลือก **"Server-side RPC"**
4. กรอก:
   - Method: `setMode`
   - Params: `{"mode":"manual"}`
5. คลิก **"Send RPC command"**
6. ดูผลลัพธ์ใน **Response**

---

## 📊 สร้าง Dashboard

### ขั้นตอนที่ 1: สร้าง Dashboard ใหม่

1. ไปที่ **Dashboards**
2. คลิก **"+"** → **"Create new dashboard"**
3. กรอกชื่อ: `ESP32 Control Panel`
4. คลิก **"Add"**

---

### ขั้นตอนที่ 2: เพิ่ม Widgets

#### Widget 1: Current Values (แสดงค่าปัจจุบัน)

1. คลิก **"+"** → **"Add new widget"**
2. เลือก **"Cards"** → **"Entities table"**
3. เพิ่ม Device: `ESP32_ThaiTechZone_V2`
4. เลือก Telemetry:
   ```
   - temperature
   - humidity
   - pm2_5
   - aqi
   - wifi_rssi
   - uptime
   ```
5. คลิก **"Add"**

---

#### Widget 2: Temperature Chart (กราฟอุณหภูมิ)

1. คลิก **"+"** → **"Add new widget"**
2. เลือก **"Charts"** → **"Timeseries - Line Chart"**
3. เพิ่ม Datasource:
   - Device: `ESP32_ThaiTechZone_V2`
   - Telemetry: `temperature`
4. ตั้งค่า:
   - Title: `Temperature (°C)`
   - Show legend: ✓
5. คลิก **"Add"**

---

#### Widget 3: Relay Control (ควบคุม Relay)

1. คลิก **"+"** → **"Add new widget"**
2. เลือก **"Control widgets"** → **"RPC Button"**
3. กำหนดค่า:
   - Target device: `ESP32_ThaiTechZone_V2`
   - Button label: `Relay 1 ON`
   - RPC method: `setRelay`
   - Request body:
     ```json
     {
       "relay": 1,
       "state": true
     }
     ```
4. คลิก **"Add"**
5. ทำซ้ำสำหรับ Relay 1 OFF, Relay 2, Relay 3

---

#### Widget 4: Switch Control (สวิตช์ควบคุม)

1. คลิก **"+"** → **"Add new widget"**
2. เลือก **"Control widgets"** → **"Switch Control"**
3. กำหนดค่า:
   - Target device: `ESP32_ThaiTechZone_V2`
   - Title: `Relay 1`
   - RPC set value method: `setRelay`
   - RPC get value method: `getValue`
   - Request body (ON):
     ```json
     {
       "relay": 1,
       "state": true
     }
     ```
   - Request body (OFF):
     ```json
     {
       "relay": 1,
       "state": false
     }
     ```
4. คลิก **"Add"**

---

#### Widget 5: Gauge (มาตรวัด)

สำหรับแสดง PM2.5 แบบ Gauge:

1. เลือก **"Analogue gauges"** → **"Radial gauge"**
2. Datasource: `pm2_5`
3. ตั้งค่า:
   - Min value: 0
   - Max value: 100
   - Units: `µg/m³`
   - Thresholds:
     - 0-12: Green (Good)
     - 12-35: Yellow (Moderate)
     - 35-55: Orange (Unhealthy)
     - 55+: Red (Hazardous)

---

### ขั้นตอนที่ 3: จัดเรียง Dashboard

1. คลิก **"Edit mode"** (ดินสอ)
2. ลาก Widgets ไปวางตามต้องการ
3. ปรับขนาดด้วยการลากมุม
4. คลิก **"Save"** เมื่อเสร็จ

---

## 🚨 Rule Engine & Alarms

### ตัวอย่าง: แจ้งเตือนเมื่อ PM2.5 สูง

#### ขั้นตอนที่ 1: สร้าง Rule Chain

1. ไปที่ **Rule Chains**
2. คลิก **"+"** → **"Create Rule Chain"**
3. ชื่อ: `PM2.5 Alert`

#### ขั้นตอนที่ 2: เพิ่ม Nodes

1. **Input Node**: Message Type Filter
   - Message types: `Post telemetry`

2. **Script Node**: Transformation Script
   ```javascript
   if (msg.pm2_5 > 35) {
     return {
       msg: msg,
       metadata: metadata,
       msgType: "PM2.5_HIGH_ALARM"
     };
   }
   return {msg: msg, metadata: metadata, msgType: "OK"};
   ```

3. **Action Node**: Create Alarm
   - Alarm type: `PM2.5_HIGH`
   - Severity: `WARNING` หรือ `CRITICAL`

4. **Action Node**: Send Email (ถ้าต้องการ)

#### ขั้นตอนที่ 3: เชื่อมต่อ Nodes

1. ลากเส้นเชื่อมจาก Input → Script → Create Alarm
2. คลิก **"Apply changes"**

---

## 🔍 Troubleshooting

### ปัญหา: ThingsBoard Connection Failed, rc=-2

**สาเหตุ:**
- Access Token ไม่ถูกต้อง
- Server address ผิด

**แก้ไข:**
1. ตรวจสอบ Access Token อีกครั้ง
2. Login เข้า ThingsBoard → Devices → Copy Token ใหม่
3. แก้ไขในโค้ด:
   ```cpp
   const char* MQTT_USER = "YOUR_ACTUAL_TOKEN_HERE";
   ```
4. Recompile และ Upload

---

### ปัญหา: ไม่เห็นข้อมูลใน Dashboard

**สาเหตุ:**
- Device ยังไม่ส่งข้อมูล
- Dashboard Widget ไม่ได้ระบุ Device

**แก้ไข:**
1. ตรวจสอบ Serial Monitor ว่ามี `✅ Telemetry sent`
2. ตรวจสอบ **Latest telemetry** ใน Device Details
3. แก้ไข Widget → เลือก Device และ Telemetry key ที่ถูกต้อง

---

### ปัญหา: RPC Command ไม่ทำงาน

**สาเหตุ:**
- อยู่ใน Auto mode (ควบคุม Relay ได้เฉพาะ Manual mode)
- JSON format ผิด

**แก้ไข:**
1. ส่ง RPC `setMode` เป็น `manual` ก่อน
2. ตรวจสอบ JSON ใน RPC Request:
   ```json
   {
     "method": "setRelay",
     "params": {"relay": 1, "state": true}
   }
   ```

---

### ปัญหา: Device Offline

**สาเหตุ:**
- WiFi หลุด
- MQTT connection timeout

**แก้ไข:**
- ESP32 จะ Auto-reconnect ทุก 5 วินาที
- ตรวจสอบ WiFi signal
- ดู Serial Monitor:
  ```
  ThingsBoard Connection Failed, rc=-2
  Check Access Token!
  ```

---

## 📱 ThingsBoard Mobile App

### ดาวน์โหลด
- **Android**: [Google Play Store](https://play.google.com/store/apps/details?id=org.thingsboard.demo.app)
- **iOS**: [App Store](https://apps.apple.com/app/thingsboard-live/id1594355695)

### การใช้งาน
1. เปิด App
2. เลือก Server:
   - Demo: `demo.thingsboard.io`
   - Cloud: `thingsboard.cloud`
3. Login ด้วย Email และ Password
4. เข้าถึง Dashboard ทั้งหมด
5. ควบคุม Relay/AUX ผ่านมือถือ

---

## 🎓 Telemetry Keys ทั้งหมด

| Key | Type | Unit | Description |
|-----|------|------|-------------|
| `uptime` | Number | seconds | เวลาทำงานของ ESP32 |
| `mode` | String | - | โหมด (auto/manual) |
| `wifi_rssi` | Number | dBm | ความแรงสัญญาณ WiFi |
| `free_heap` | Number | bytes | RAM ว่าง |
| `relay1` | Boolean | - | สถานะ Relay 1 |
| `relay2` | Boolean | - | สถานะ Relay 2 |
| `relay3` | Boolean | - | สถานะ Relay 3 |
| `aux1` | Boolean | - | สถานะ AUX 1 |
| `aux2` | Boolean | - | สถานะ AUX 2 |
| `aux3` | Boolean | - | สถานะ AUX 3 |
| `aux4` | Boolean | - | สถานะ AUX 4 |
| `switch1` | Boolean | - | สถานะ Switch 1 |
| `switch2` | Boolean | - | สถานะ Switch 2 |
| `switch3` | Boolean | - | สถานะ Switch 3 |
| `iso_input1` | Boolean | - | สถานะ Isolated Input 1 |
| `iso_input2` | Boolean | - | สถานะ Isolated Input 2 |
| `temperature` | Number | °C | อุณหภูมิ |
| `humidity` | Number | % | ความชื้น |
| `weather_city` | String | - | ชื่อเมือง |
| `weather_description` | String | - | คำอธิบายสภาพอากาศ |
| `pm2_5` | Number | µg/m³ | ค่า PM2.5 |
| `pm10` | Number | µg/m³ | ค่า PM10 |
| `aqi` | Number | 1-5 | ดัชนีคุณภาพอากาศ |
| `air_quality` | String | - | คำอธิบายคุณภาพอากาศ |

---

## 🏆 สรุป

### ข้อดีของ ThingsBoard

✅ **Dashboard สวยงาม** - ปรับแต่งได้ตามต้องการ  
✅ **Device Management** - จัดการหลายอุปกรณ์ได้ง่าย  
✅ **Data Storage** - เก็บประวัติข้อมูล  
✅ **Visualization** - Charts, Gauges, Maps  
✅ **Alarms** - แจ้งเตือนอัตโนมัติ  
✅ **Rule Engine** - Automation ขั้นสูง  
✅ **Mobile App** - ควบคุมผ่านมือถือ  
✅ **RESTful API** - Integration ง่าย  
✅ **Security** - Access Token, SSL/TLS  

### ขั้นตอนต่อไป

1. ✅ สร้าง ThingsBoard Account
2. ✅ เพิ่ม Device และ Copy Access Token
3. ✅ แก้ไขโค้ด ESP32
4. ✅ Upload และทดสอบ
5. ✅ สร้าง Dashboard
6. ⏭️ เพิ่ม Alarms และ Rule Engine
7. ⏭️ ใช้งานจริงกับ ThingsBoard Cloud
8. ⏭️ สร้าง Mobile Dashboard

---

**เอกสารนี้จัดทำโดย:** ThaiTechZone Team  
**เวอร์ชัน:** 3.0 (ThingsBoard Integration)  
**อัปเดตล่าสุด:** 16 ตุลาคม 2025  
**ThingsBoard Version:** 3.6+
