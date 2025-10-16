# 📊 Dashboard Configuration - Updated

## ✅ การปรับปรุงที่ทำแล้ว (2025-10-16)

### การเปลี่ยนแปลงหลัก:

#### 1. **Weather Card** - ลดจำนวน dataKeys
**เดิม (ส่ง 4 ค่า):**
- `temperature`
- `humidity`
- `weather_city` ❌ ไม่ส่งแล้ว
- `weather_description` ❌ ไม่ส่งแล้ว

**ใหม่ (ส่ง 2 ค่า):**
- `temperature` ✅
- `humidity` ✅

**การแสดงผล:**
```html
🌡️
28°C

💧
Humidity: 75%

Updated every 10 minutes
```

---

#### 2. **AQI Card** - ลดจำนวน dataKeys
**เดิม (ส่ง 3 ค่า):**
- `aqi`
- `pm2_5`
- `air_quality` ❌ ไม่ส่งแล้ว (text description)

**ใหม่ (ส่ง 2 ค่า):**
- `aqi` ✅
- `pm2_5` ✅

**การแสดงผล:**
```html
💨
2
Air Quality Index

PM2.5: 12.5 µg/m³

1=Good, 2=Fair, 3=Moderate, 4=Poor, 5=VeryPoor
```

---

### Widgets ที่ไม่เปลี่ยนแปลง:

✅ **System Status Card**
- `uptime`, `mode`, `wifi_rssi`, `free_heap`

✅ **Temperature & Humidity Chart**
- `temperature`, `humidity`

✅ **PM2.5 Gauge**
- `pm2_5`

✅ **Mode Control Switch**
- RPC: `setMode`

✅ **Relay 1-3 Switches**
- RPC: `setRelay`

✅ **AUX 1-4 Controls**
- RPC: `setAux`

✅ **Input Status**
- `switch1-3`, `iso_input1-2`

✅ **Emergency Stop**
- RPC: `turnOffAll`

---

## 📋 Telemetry Data Structure (ปัจจุบัน)

### ข้อมูลที่ส่งเสมอ (ทุก 10 วินาที):
```json
{
  "uptime": 123,
  "mode": "auto",
  "wifi_rssi": -65,
  "free_heap": 250000,
  "relay1": false,
  "relay2": true,
  "relay3": false,
  "aux1": false,
  "aux2": true,
  "aux3": false,
  "aux4": false,
  "switch1": false,
  "switch2": false,
  "switch3": false,
  "iso_input1": false,
  "iso_input2": false
}
```

### ข้อมูล Weather (เมื่อมีข้อมูล):
```json
{
  "temperature": 28,
  "humidity": 75
}
```

### ข้อมูล Air Quality (เมื่อมีข้อมูล):
```json
{
  "pm2_5": 12.5,
  "aqi": 2
}
```

---

## 🎨 Dashboard Layout (24 columns)

```
┌─────────────┬──────────────────────────┬────────────┐
│             │                          │            │
│   System    │  Temperature & Humidity  │  PM2.5     │
│   Status    │        Chart             │  Gauge     │
│   (6x6)     │        (12x8)            │  (6x6)     │
│             │                          │            │
├─────┬───────┼─────┬───────┬───────────┴────────────┤
│Mode │Relay1 │Relay2│Relay3 │                        │
│(4x3)│ (4x3) │ (4x3)│ (4x3) │                        │
├─────┴───────┼──────┴───────┼────────────────────────┤
│             │              │                        │
│    AUX      │    Input     │      Weather           │
│  Controls   │    Status    │        Card            │
│   (8x4)     │    (8x4)     │       (8x4)            │
│             │              │                        │
├─────────────┼──────────────┴────────────────────────┤
│             │                                       │
│    AQI      │       Emergency Stop                  │
│    Card     │         (6x3)                         │
│   (6x3)     │                                       │
└─────────────┴───────────────────────────────────────┘
```

---

## 📦 วิธีใช้งาน Dashboard

### Option 1: Import JSON File (แนะนำ)

1. **เปิด ThingsBoard**
   ```
   https://demo.thingsboard.io
   ```

2. **Import Dashboard**
   ```
   Dashboards → + → Import dashboard
   → Browse → เลือก esp32_dashboard.json
   → Import
   ```

3. **ตั้งค่า Entity Alias**
   ```
   Edit Dashboard → Entity aliases
   → esp32_device → Configure
   → Device name: ESP32_ThaiTechZone_V2
   → Apply
   ```

4. **Save และทดสอบ**
   ```
   Apply changes → Save dashboard
   → ดูข้อมูล real-time
   ```

---

### Option 2: สร้าง Widget ทีละตัว

#### Widget 1: System Status
```
Type: Cards → Simple card
Datasources:
  - uptime (Uptime, sec)
  - mode (Mode)
  - wifi_rssi (WiFi Signal, dBm)
  - free_heap (Free RAM, bytes)
Time window: Last 1 minute
```

#### Widget 2: Temperature & Humidity Chart
```
Type: Charts → Time series - Line
Datasources:
  - temperature (°C, red line)
  - humidity (%, blue line)
Time window: Last 1 hour
Y-axis: 0-100
```

#### Widget 3: PM2.5 Gauge
```
Type: Analogue gauges → Radial gauge
Datasource: pm2_5
Min: 0, Max: 100
Units: µg/m³
Highlights:
  0-12: Green (Good)
  12-35: Yellow (Moderate)
  35-55: Orange (Unhealthy)
  55-100: Red (Hazardous)
```

#### Widget 4: Mode Control
```
Type: Control widgets → Switch control
RPC Method: setMode
Parameters: {"mode": "auto"} / {"mode": "manual"}
Labels: AUTO / MANUAL
Colors: Blue / Orange
```

#### Widget 5-7: Relay Controls
```
Type: Control widgets → Switch control
RPC Method: setRelay
Parameters ON: {"relay": 1, "state": true}
Parameters OFF: {"relay": 1, "state": false}
```

#### Widget 8: AUX Controls
```
Type: Control widgets → Button row
8 buttons:
  - AUX 1 ON/OFF
  - AUX 2 ON/OFF
  - AUX 3 ON/OFF
  - AUX 4 ON/OFF
RPC Method: setAux
Parameters: {"aux": 1-4, "state": true/false}
```

#### Widget 9: Input Status
```
Type: Cards → Entity list
Datasources:
  - switch1-3
  - iso_input1-2
Display: Boolean (ON/OFF icons)
```

#### Widget 10: Weather Card (HTML)
```
Type: Cards → HTML card
Datasources: temperature, humidity
HTML Template:
<div style="text-align:center;padding:20px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border-radius:8px;">
  <div style="font-size:64px;">🌡️</div>
  <h1 style="font-size:48px;">${temperature}°C</h1>
  <div style="font-size:32px;">💧</div>
  <p style="font-size:24px;">Humidity: ${humidity}%</p>
</div>
```

#### Widget 11: AQI Card (HTML)
```
Type: Cards → HTML card
Datasources: aqi, pm2_5
HTML Template:
<div style="text-align:center;padding:20px;">
  <div style="font-size:48px;">💨</div>
  <h1 style="font-size:64px;color:#4CAF50;">${aqi}</h1>
  <p>PM2.5: ${pm2_5} µg/m³</p>
  <p style="font-size:12px;">1=Good, 2=Fair, 3=Moderate, 4=Poor, 5=VeryPoor</p>
</div>
```

#### Widget 12: Emergency Stop
```
Type: Control widgets → Button
RPC Method: turnOffAll
Label: 🚨 TURN OFF ALL
Color: Red
Confirm: Yes
```

---

## 🔄 RPC Methods รองรับ

### 1. setMode - เปลี่ยนโหมด
```json
{
  "method": "setMode",
  "params": {
    "mode": "auto"  // หรือ "manual"
  }
}
```

**Response:**
```json
{
  "mode": "auto"
}
```

### 2. setRelay - ควบคุม Relay
```json
{
  "method": "setRelay",
  "params": {
    "relay": 1,     // 1, 2, หรือ 3
    "state": true   // true=ON, false=OFF
  }
}
```

**Response:**
```json
{
  "relay": 1,
  "state": true
}
```

**หมายเหตุ:** ต้องอยู่ใน Manual Mode ก่อน

### 3. setAux - ควบคุม AUX
```json
{
  "method": "setAux",
  "params": {
    "aux": 1,       // 1, 2, 3, หรือ 4
    "state": true   // true=ON, false=OFF
  }
}
```

**Response:**
```json
{
  "aux": 1,
  "state": true
}
```

**หมายเหตุ:** ต้องอยู่ใน Manual Mode ก่อน

### 4. getValue - อ่านค่าปัจจุบัน
```json
{
  "method": "getValue",
  "params": {}
}
```

**Response:**
```json
{
  "mode": "auto",
  "uptime": 1234,
  "relays": {
    "rl1": false,
    "rl2": true,
    "rl3": false
  }
}
```

### 5. turnOffAll - ปิดทุกอย่าง
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

## 🎯 Testing Checklist

### ทดสอบ Monitoring:
- [ ] System Status แสดงข้อมูล (uptime, mode, wifi_rssi, free_heap)
- [ ] Temperature & Humidity Chart แสดงกราฟ
- [ ] PM2.5 Gauge แสดงค่า
- [ ] Input Status แสดงสถานะ switches & ISO inputs
- [ ] Weather Card แสดง temperature & humidity
- [ ] AQI Card แสดง AQI & PM2.5

### ทดสอบ Control:
- [ ] Mode Switch เปลี่ยนได้ระหว่าง Auto/Manual
- [ ] Relay 1-3 ควบคุมได้ใน Manual mode
- [ ] AUX 1-4 ควบคุมได้ใน Manual mode
- [ ] Emergency Stop ปิดทุกอย่างได้
- [ ] Serial Monitor แสดง RPC commands
- [ ] Telemetry อัพเดททันทีหลังควบคุม

---

## 🐛 Troubleshooting

### ปัญหา: Widget ไม่แสดงข้อมูล

**สาเหตุ:**
- Entity Alias ไม่ถูกต้อง
- Device name ไม่ตรง
- Telemetry key ผิด

**วิธีแก้:**
1. Edit Dashboard → Entity aliases
2. ตรวจสอบ Device name = `ESP32_ThaiTechZone_V2`
3. ตรวจสอบ Telemetry keys ตรงกับที่โค้ดส่ง

### ปัญหา: RPC ไม่ทำงาน

**สาเหตุ:**
- ไม่ได้อยู่ใน Manual mode
- MQTT disconnect
- Access Token ผิด

**วิธีแก้:**
1. Switch to Manual mode ก่อน
2. ตรวจสอบ MQTT connected (ดู Serial Monitor)
3. ตรวจสอบ Access Token ใน code

### ปัญหา: Weather/AQI ไม่มีข้อมูล

**สาเหตุ:**
- Weather/AQI API ยังไม่อัพเดท (ใช้เวลา ~10 นาที)
- API Key หมดอายุ
- Network timeout

**วิธีเช็ค:**
1. ดู Serial Monitor → Weather Data section
2. รอ 10 นาที หลัง boot
3. ตรวจสอบ API Key ใน code

---

## 📚 Files ที่เกี่ยวข้อง

- `esp32_dashboard.json` - Dashboard configuration (ไฟล์นี้)
- `src/main.cpp` - ESP32 firmware
- `SUCCESS_GUIDE.md` - คู่มือการใช้งานโปรเจค
- `TROUBLESHOOTING.md` - คู่มือแก้ปัญหา

---

## 🔄 Version History

### v2.0 (2025-10-16) - Current
- ✅ ลบ `weather_city` และ `weather_description` 
- ✅ ลบ `air_quality` text description
- ✅ ลดขนาด JSON payload (~285 bytes)
- ✅ เพิ่ม emoji icons ใน cards
- ✅ ปรับ HTML templates ให้สวยงาม

### v1.0 (เดิม)
- ส่ง telemetry 24 keys รวม Weather/AQI
- JSON size ~600 bytes
- มีปัญหา MQTT buffer overflow

---

## ✅ สรุป

**Dashboard นี้พร้อมใช้งานแล้วครับ!**

**การปรับปรุง:**
- ✅ ลด telemetry keys จาก 24 → 19 keys
- ✅ JSON size ลดจาก ~600 → ~285 bytes
- ✅ ส่งเฉพาะข้อมูลที่จำเป็น
- ✅ Weather & AQI ส่งเฉพาะเมื่อมีข้อมูล
- ✅ Dashboard widgets ปรับให้ตรงกับ telemetry structure

**วิธีใช้:**
1. Import `esp32_dashboard.json` เข้า ThingsBoard
2. ตั้งค่า Entity Alias → `ESP32_ThaiTechZone_V2`
3. Save dashboard
4. ใช้งานได้เลย! 🎉

---

**หากมีคำถามหรือต้องการปรับแต่งเพิ่มเติม สอบถามได้เลยครับ!**
