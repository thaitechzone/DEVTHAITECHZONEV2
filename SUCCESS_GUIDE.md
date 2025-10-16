# 🎉 ThingsBoard Integration - SUCCESS!

## ✅ สถานะปัจจุบัน

### การเชื่อมต่อ
- ✅ WiFi Connected
- ✅ MQTT Connected to demo.thingsboard.io
- ✅ **Latest Telemetry มีข้อมูลแล้ว!**
- ✅ RPC Subscribed (พร้อมรับคำสั่งควบคุม)

### ข้อมูล Telemetry ที่ส่ง (ทุก 10 วินาที)
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
  "iso_input2": false,
  "temperature": 28,
  "humidity": 75,
  "pm2_5": 12.5,
  "aqi": 2
}
```

---

## 📊 ขั้นตอนถัดไป: สร้าง Dashboard

### Step 1: Import Dashboard (แนะนำ)

ใช้ไฟล์ `dashboard_config/esp32_dashboard.json` ที่สร้างไว้แล้ว:

1. **เปิด ThingsBoard**
   - ไปที่ `Dashboards` (เมนูซ้าย)

2. **Import Dashboard**
   - กดปุ่ม `+` มุมขวาล่าง
   - เลือก `Import dashboard`

3. **เลือกไฟล์**
   - Browse → `dashboard_config/esp32_dashboard.json`
   - กด `Import`

4. **ตรวจสอบ Entity Alias**
   - เปิด Dashboard
   - กด `Edit` (ไอคอนดินสอ)
   - เปิด `Entity aliases`
   - ตรวจสอบว่า Device Name = `ESP32_ThaiTechZone_V2`

5. **Save และทดสอบ**
   - กด `Apply changes`
   - ดูข้อมูลแสดงผลใน Widgets

---

### Step 2: สร้าง Dashboard เอง (ถ้าต้องการปรับแต่ง)

#### Widget 1: System Status Card
```
Type: Cards → Entities table
Entities: ESP32_ThaiTechZone_V2
Columns:
- uptime (Uptime)
- mode (Mode)
- wifi_rssi (WiFi Signal)
- free_heap (Free Memory)
```

#### Widget 2: Temperature & Humidity Chart
```
Type: Charts → Time series - Line
Datasources:
- temperature (label: Temperature °C)
- humidity (label: Humidity %)
Time window: Last 1 hour
```

#### Widget 3: PM2.5 Gauge
```
Type: Analogue gauges → Radial gauge
Datasource: pm2_5
Min: 0, Max: 100
Units: µg/m³
Color ranges:
- 0-25: Green (Good)
- 25-50: Yellow (Moderate)
- 50-100: Red (Unhealthy)
```

#### Widget 4: Relay Control Switches
```
Type: Control widgets → Switch control
RPC Method: setRelay
Parameters:
{
  "relay": 1,
  "state": true/false
}
```

สร้าง 3 widgets สำหรับ Relay 1, 2, 3

#### Widget 5: AUX Control Buttons
```
Type: Control widgets → Basic button
RPC Method: setAux
Parameters:
{
  "aux": 1,
  "state": true
}
```

สร้าง 4 widgets สำหรับ AUX 1-4

#### Widget 6: Mode Switch
```
Type: Control widgets → Switch control
RPC Method: setMode
Parameters for ON:
{
  "mode": "auto"
}
Parameters for OFF:
{
  "mode": "manual"
}
```

#### Widget 7: Input Status
```
Type: Cards → Entities table
Datasources:
- switch1, switch2, switch3
- iso_input1, iso_input2
Display as: Boolean (ON/OFF)
```

#### Widget 8: Emergency Stop
```
Type: Control widgets → Basic button
RPC Method: turnOffAll
Label: "🛑 EMERGENCY STOP"
Color: Red
```

---

## 🧪 การทดสอบ RPC Control

### Test 1: เปลี่ยนโหมด
1. Dashboard → Mode Switch Widget
2. Toggle ระหว่าง Auto/Manual
3. ดู Serial Monitor:
   ```
   RPC Method: setMode
   Switched to MANUAL mode
   ```

### Test 2: ควบคุม Relay (ต้องอยู่ใน Manual Mode)
1. Switch to Manual Mode ก่อน
2. กด Relay 1 Switch → ON
3. ดู Serial Monitor:
   ```
   Relay 1 set to ON
   ```
4. ดูที่บอร์ด → Relay 1 ควร ON (LED ติด)

### Test 3: ควบคุม AUX
1. กดปุ่ม AUX 1
2. ดู Serial Monitor:
   ```
   AUX 1 set to ON
   ```
3. ดูที่บอร์ด → AUX1_PIN (GPIO 12) ควรเป็น HIGH

### Test 4: Emergency Stop
1. กดปุ่ม "🛑 EMERGENCY STOP"
2. ดู Serial Monitor:
   ```
   All outputs turned off
   ```
3. ทุก Relay และ AUX ควร OFF ทันที

---

## 📈 การตรวจสอบข้อมูล

### ใน ThingsBoard Web UI:

1. **Latest Telemetry**
   - Device → Latest telemetry tab
   - ดูข้อมูลปัจจุบัน (อัพเดททุก 10 วินาที)

2. **Telemetry History**
   - Device → Attributes & telemetry → กด telemetry key
   - เลือก Time range
   - ดูกราฟประวัติ

3. **Audit Logs**
   - Device → Audit logs
   - ดูประวัติการเชื่อมต่อและคำสั่ง RPC

### ใน Serial Monitor:

```
╔════════════════════════════════════════════════╗
║         ESP32 BOARD STATUS REPORT              ║
╠════════════════════════════════════════════════╣
║ Mode: AUTO | Uptime: 1234 sec
║ WiFi: -65 dBm | IP: 192.168.1.100
║ MQTT: ✅ CONNECTED | Broker: demo.thingsboard.io
╠════════════════════════════════════════════════╣
║ OUTPUT STATES:
║   Relay 1: ON  | Relay 2: OFF | Relay 3: OFF
║   AUX 1: OFF | AUX 2: ON  | AUX 3: OFF | AUX 4: OFF
...
```

---

## 🎯 Features ที่ใช้งานได้

### ✅ Monitoring (การติดตาม)
- [x] System uptime
- [x] WiFi signal strength (RSSI)
- [x] Free heap memory
- [x] Operation mode (Auto/Manual)
- [x] Relay states (3 relays)
- [x] AUX output states (4 outputs)
- [x] Input states (3 switches + 2 isolated inputs)
- [x] Temperature & Humidity (เมื่อมีข้อมูล)
- [x] Air Quality - PM2.5 & AQI (เมื่อมีข้อมูล)

### ✅ Control (การควบคุม)
- [x] Switch mode (Auto ↔ Manual)
- [x] Control Relay 1-3 (Manual mode only)
- [x] Control AUX 1-4 (Manual mode only)
- [x] Emergency stop (Turn off all)
- [x] Get current status

### ✅ Data Visualization (การแสดงผล)
- [x] Real-time telemetry (10-second updates)
- [x] Historical data charts
- [x] Gauges & Cards
- [x] Status indicators

---

## 🔧 การปรับแต่งเพิ่มเติม

### เพิ่ม Widget ใหม่

**Widget: Weather Card**
```
Type: Cards → Simple card
Datasources:
- temperature
- humidity
Template:
"🌡️ {temperature}°C | 💧 {humidity}%"
```

**Widget: AQI Card**
```
Type: Cards → Simple card
Datasources:
- pm2_5
- aqi
Template:
"PM2.5: {pm2_5} | AQI: {aqi}"
Color function:
if (pm2_5 < 25) return 'green';
else if (pm2_5 < 50) return 'yellow';
else return 'red';
```

### เพิ่ม Alarm/Notification

1. **High PM2.5 Alarm**
   ```
   Device Profile → Alarm Rules → Add
   Type: Create alarm
   Name: High PM2.5
   Condition: pm2_5 > 50
   Severity: Major
   ```

2. **Low Heap Memory Warning**
   ```
   Condition: free_heap < 50000
   Severity: Warning
   ```

3. **WiFi Disconnect Alert**
   ```
   Condition: wifi_rssi == 0 OR wifi_rssi < -90
   Severity: Critical
   ```

---

## 📱 Mobile App (Optional)

### ThingsBoard Mobile App

1. **ดาวน์โหลด**
   - iOS: App Store → "ThingsBoard Live"
   - Android: Play Store → "ThingsBoard Live"

2. **เชื่อมต่อ**
   - Server: `https://demo.thingsboard.io`
   - Username/Password: (บัญชี ThingsBoard ของคุณ)

3. **เข้าถึง Dashboard**
   - เปิด Dashboard ที่สร้างไว้
   - ควบคุมอุปกรณ์จากมือถือได้เลย!

---

## 🚀 Advanced Features (ขั้นสูง)

### 1. Rule Chains (การตั้งกฎอัตโนมัติ)

**ตัวอย่าง: Auto Turn ON Relay เมื่ออุณหภูมิสูง**
```
Rule Chain:
1. Filter: temperature > 30
2. Action: RPC call "setRelay"
   Parameters: {"relay": 1, "state": true}
```

### 2. Webhooks (การแจ้งเตือนภายนอก)

**ตัวอย่าง: ส่ง Line Notify เมื่อ PM2.5 สูง**
```
Integration → HTTP → Line Notify API
Trigger: pm2_5 > 50
Message: "⚠️ High PM2.5: {pm2_5} µg/m³"
```

### 3. Data Export

**ส่งออกข้อมูลเป็น CSV**
```
Device → Telemetry → Export
Format: CSV
Time range: Last 24 hours
```

### 4. Custom Widget (สร้าง Widget เอง)

```javascript
// Custom widget for relay control with animation
self.onInit = function() {
  // Widget initialization code
}

self.onDataUpdated = function() {
  // Update display when data changes
}
```

---

## 📊 การวิเคราะห์ข้อมูล

### Query ตัวอย่าง (Analytics)

**Average Temperature per Hour**
```
Device → Analytics → Aggregation
Key: temperature
Function: AVG
Interval: 1 hour
```

**Max PM2.5 per Day**
```
Key: pm2_5
Function: MAX
Interval: 1 day
```

**Relay ON Time (Total Duration)**
```
Key: relay1
Function: SUM (where value = true)
```

---

## 🔐 Security Best Practices

### 1. เปลี่ยน Access Token
```cpp
// ไม่ควรใช้ token เดียวกันนานๆ
const char* MQTT_USER = "NEW_ACCESS_TOKEN_HERE";
```

### 2. ใช้ MQTT over SSL/TLS
```cpp
// ใน production ควรใช้ port 8883 (secure)
const int MQTT_PORT = 8883;
```

### 3. IP Whitelist (ถ้าใช้ Self-hosted)
```
ThingsBoard → Security Settings
Allow only specific IP ranges
```

### 4. User Permissions
```
Dashboard → Share
Set permissions: View Only / Full Access
```

---

## 🆘 Troubleshooting (แก้ปัญหาเพิ่มเติม)

### ปัญหา: Telemetry หยุดอัพเดท

**สาเหตุ:**
- WiFi ขาดหาย
- MQTT disconnect
- ESP32 restart/crash

**วิธีแก้:**
1. ดู Serial Monitor → MQTT status
2. ตรวจสอบ WiFi signal
3. เช็ค uptime (ถ้า reset = มีปัญหา)

### ปัญหา: RPC ไม่ทำงาน

**สาเหตุ:**
- อยู่ใน Auto mode (ต้องเป็น Manual)
- MQTT not subscribed
- JSON format ผิด

**วิธีแก้:**
1. เปลี่ยนเป็น Manual mode ก่อน
2. ดู Serial Monitor → "Subscribed to RPC commands"
3. ทดสอบด้วย RPC tab ใน Device

### ปัญหา: Weather/AQI ไม่มีข้อมูล

**สาเหตุ:**
- API Key หมดอายุ
- City name ผิด
- Network timeout

**วิธีเช็ค:**
```
Serial Monitor:
║ WEATHER DATA:
║   Status: Timeout (Waiting for data...)
```

**วิธีแก้:**
1. ตรวจสอบ API Key
2. ทดสอบ API ด้วย Browser
3. เพิ่ม timeout value

---

## 📚 เอกสารเพิ่มเติม

### ไฟล์ที่เกี่ยวข้อง:
- `TROUBLESHOOTING.md` - คู่มือแก้ปัญหาแบบละเอียด
- `DASHBOARD_GUIDE.md` - คู่มือสร้าง Dashboard
- `THINGSBOARD_SETUP.md` - คู่มือติดตั้ง ThingsBoard
- `dashboard_config/esp32_dashboard.json` - Dashboard config (พร้อม import)

### ThingsBoard Documentation:
- Official Docs: https://thingsboard.io/docs/
- API Reference: https://thingsboard.io/docs/api/
- Community: https://groups.google.com/forum/#!forum/thingsboard

---

## 🎯 Checklist สำเร็จ

- [x] WiFi Connected
- [x] MQTT Connected
- [x] Telemetry ส่งข้อมูลสำเร็จ
- [x] Latest Telemetry แสดงข้อมูล
- [ ] Dashboard ถูกสร้างแล้ว
- [ ] Widgets ทำงานถูกต้อง
- [ ] RPC Control ทดสอบแล้ว
- [ ] Weather/AQI มีข้อมูล (optional)
- [ ] Alarms ตั้งค่าแล้ว (optional)
- [ ] Mobile App เชื่อมต่อแล้ว (optional)

---

## 🎊 สรุป

**คุณได้ทำสำเร็จแล้ว:**
✅ ESP32 เชื่อมต่อ ThingsBoard
✅ ส่ง Telemetry ทุก 10 วินาที
✅ พร้อมรับคำสั่ง RPC
✅ ส่งข้อมูล 15+ sensors
✅ รองรับ Weather & Air Quality
✅ Serial Monitor แสดงผลสวยงาม

**ขั้นตอนถัดไป:**
1. Import Dashboard (ใช้ไฟล์ JSON ที่มี)
2. ทดสอบ RPC Control
3. ตั้งค่า Alarms (optional)
4. ติดตั้ง Mobile App (optional)

---

**🎉 ยินดีด้วย! โปรเจค IoT ของคุณใช้งานได้แล้ว! 🎉**

หากมีคำถามหรือต้องการเพิ่มฟีเจอร์เพิ่มเติม สามารถสอบถามได้เลยครับ!
