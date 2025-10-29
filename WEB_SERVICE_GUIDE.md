# ESP32 ThaiTechZone V2.0 - Web Service Guide

## 🌐 การเข้าถึง Web Interface

### ขั้นตอนที่ 1: Upload Filesystem
ก่อนใช้งาน Web Service ครั้งแรก ต้อง upload ไฟล์ HTML/CSS/JS ไปยัง LittleFS:

```bash
# ใน PlatformIO Terminal
pio run --target uploadfs
```

หรือใช้ PlatformIO UI:
1. คลิก **PlatformIO Icon** (มด) ในแถบด้านซ้าย
2. เลือก **PROJECT TASKS** → **Platform** → **Upload Filesystem Image**

### ขั้นตอนที่ 2: Upload Firmware
```bash
pio run --target upload
```

### ขั้นตอนที่ 3: เปิด Serial Monitor เพื่อดู IP Address
```bash
pio device monitor
```

หา IP Address จาก log:
```
Web Server Started
Access Dashboard at: http://192.168.1.xxx
```

### ขั้นตอนที่ 4: เข้าใช้งาน Web Dashboard
1. เปิด browser (Chrome, Firefox, Edge)
2. ไปที่ `http://192.168.1.xxx` (ใช้ IP ที่ได้จาก Serial Monitor)
3. Login ด้วย:
   - Username: **admin**
   - Password: **password**

---

## 🎯 Features

### 1. **Dashboard หน้าหลัก**
- แสดงสถานะระบบ (Mode, Uptime, WiFi RSSI, Free Heap)
- แสดงและควบคุม Relay 1-3 (ON/OFF/Toggle)
- แสดงข้อมูล Sensors (DHT22, Weather, Air Quality)
- แสดงสถานะ Inputs (Switches และ Isolated Inputs)
- แสดงสถานะ AUX Outputs

### 2. **Relay Control**
- กดปุ่ม Toggle เพื่อเปิด/ปิด Relay แต่ละตัว
- สถานะจะอัปเดตแบบ real-time ทุก 2 วินาที
- แสดง badge สถานะ (ON = เขียว, OFF = เทา)

### 3. **Mode Switching**
- **AUTO Mode**: ระบบจะ cycle ทดสอบ outputs อัตโนมัติ
- **MANUAL Mode**: ควบคุม relays ผ่าน switches หรือ web interface
- กดปุ่ม "Toggle Mode" เพื่อสลับโหมด

### 4. **Timer Control**
- เลือก Relay (1-3)
- ตั้งเวลา (วินาที)
- กด "Start Timer" เพื่อเปิด relay และปิดอัตโนมัติตามเวลาที่กำหนด

### 5. **Event Logs**
- บันทึกเหตุการณ์ทั้งหมด (relay changes, mode switches, timer events)
- แสดงพร้อม timestamp
- เก็บ log ล่าสุด 50 รายการ
- กดปุ่ม "Clear" เพื่อลบ logs

### 6. **Real-time Updates**
- Dashboard อัปเดตข้อมูลทุก 2 วินาที
- แสดง indicator "Updating..." มุมขวาบน
- Connection status indicator แสดงสถานะการเชื่อมต่อ

---

## 🔧 API Endpoints

ทุก API ต้องใช้ **Basic Authentication** (admin:password)

### GET `/api/status`
ดึงข้อมูลสถานะทั้งหมด
```json
{
  "mode": "AUTO",
  "uptime": 12345,
  "wifi_rssi": -45,
  "free_heap": 250000,
  "relay1": true,
  "relay2": false,
  "relay3": false,
  "aux1": false,
  "aux2": false,
  "aux3": false,
  "aux4": false,
  "switch1": false,
  "switch2": false,
  "switch3": false,
  "iso_input1": false,
  "iso_input2": false,
  "dht_temperature": 25.8,
  "dht_humidity": 62.5,
  "weather_temp": 28,
  "weather_humid": 75,
  "aqi": 2,
  "aqi_quality": "Fair"
}
```

### POST `/api/relay/{1-3}/toggle`
สลับสถานะ relay
```bash
curl -X POST http://192.168.1.xxx/api/relay/1/toggle \
  -u admin:password
```

Response:
```json
{
  "relay": 1,
  "state": true
}
```

### POST `/api/mode/toggle`
สลับโหมด AUTO/MANUAL
```bash
curl -X POST http://192.168.1.xxx/api/mode/toggle \
  -u admin:password
```

Response:
```json
{
  "mode": "MANUAL"
}
```

### POST `/api/timer`
เริ่มต้น timer สำหรับ relay
```bash
curl -X POST http://192.168.1.xxx/api/timer \
  -u admin:password \
  -H "Content-Type: application/json" \
  -d '{"relay": 1, "duration": 10}'
```

Response:
```json
{
  "success": true,
  "relay": 1,
  "duration": 10
}
```

### GET `/api/logs`
ดึง event logs
```bash
curl http://192.168.1.xxx/api/logs \
  -u admin:password
```

Response:
```json
{
  "logs": [
    "14:30:25 - System started",
    "14:30:30 - Web: Relay 1 ON",
    "14:30:35 - Timer started: Relay 1 for 10s"
  ]
}
```

---

## 📱 การใช้งานจากมือถือ

1. เชื่อมต่อ WiFi เดียวกันกับ ESP32
2. เปิด browser บนมือถือ
3. ไปที่ `http://192.168.1.xxx`
4. UI จะ responsive และใช้งานสะดวกบนหน้าจอมือถือ

---

## 🔒 Security

- **Basic Authentication**: Username/Password ต้องตรงตามที่กำหนด
- **LAN Only**: Web Server ทำงานเฉพาะใน Local Network
- **Session Storage**: Credentials เก็บใน localStorage ของ browser

### เปลี่ยน Username/Password

แก้ไขใน `src/main.cpp`:
```cpp
const char* http_username = "admin";      // เปลี่ยนที่นี่
const char* http_password = "password";   // เปลี่ยนที่นี่
```

---

## 🐛 Troubleshooting

### ปัญหา: ไม่สามารถเข้าถึง Web Interface
1. ตรวจสอบ WiFi connection (ดู Serial Monitor)
2. ตรวจสอบ IP Address ให้ถูกต้อง
3. ตรวจสอบว่า computer/phone อยู่ใน network เดียวกัน
4. ลอง ping IP address: `ping 192.168.1.xxx`

### ปัญหา: Login failed
1. ตรวจสอบ username/password
2. Clear browser cache และ localStorage
3. ลองใช้ Incognito/Private mode

### ปัญหา: Web page ไม่แสดง
1. ตรวจสอบว่า upload filesystem แล้ว (`pio run --target uploadfs`)
2. ตรวจสอบ LittleFS mount ใน Serial Monitor
3. ลอง re-upload filesystem

### ปัญหา: Data ไม่อัปเดต
1. เปิด Browser Console (F12) เพื่อดู errors
2. ตรวจสอบ connection status indicator
3. Refresh หน้าเว็บ (Ctrl+F5)

---

## 📊 Performance

- **Memory Usage**: ~60-80KB RAM สำหรับ Web Server
- **Concurrent Clients**: รองรับ 4-6 clients พร้อมกัน
- **Update Rate**: 2 วินาที (ปรับได้ใน `app.js`)
- **SPIFFS Size**: ~100KB สำหรับ web files

---

## 🚀 Advanced Features (Optional)

### 1. เพิ่ม HTTPS
ใช้ ESPAsyncWebServer กับ SSL certificates (ต้อง generate cert)

### 2. WebSocket
เพิ่ม WebSocket สำหรับ real-time updates ที่รวดเร็วกว่า

### 3. OTA Updates
Upload firmware ผ่าน Web Interface

### 4. Chart/Graphs
ใช้ Chart.js เพื่อแสดงกราฟข้อมูล sensor

---

## 📝 Version History

### v2.0 (Current)
- ✅ Web Service with Bootstrap UI
- ✅ Basic Authentication
- ✅ REST API endpoints
- ✅ Timer control
- ✅ Event logging
- ✅ Real-time updates
- ✅ Mobile responsive

---

## 👨‍💻 Developer Notes

### File Structure
```
DEVTHAITECHZONEV2/
├── data/                    # LittleFS filesystem
│   ├── login.html          # Login page
│   ├── index.html          # Dashboard
│   └── app.js              # JavaScript logic
├── src/
│   └── main.cpp            # Main firmware
└── platformio.ini          # Build configuration
```

### Libraries Used
- **ESPAsyncWebServer**: Async web server
- **AsyncTCP**: Async TCP library
- **LittleFS**: Filesystem
- **ArduinoJson**: JSON parsing
- **Bootstrap 5**: UI framework
- **Font Awesome 6**: Icons

---

สนุกกับการใช้งาน ESP32 ThaiTechZone V2.0! 🎉
