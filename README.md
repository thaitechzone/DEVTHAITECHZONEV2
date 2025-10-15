# ESPThaiTechZone V2.0 - โปรแกรมทดสอบบอร์ด

![ESP32](https://img.shields.io/badge/ESP32-DevModule-blue)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange)
![License](https://img.shields.io/badge/License-MIT-green)

โปรแกรมทดสอบบอร์ด ESP32 สำหรับ **ESPThaiTechZone V2.0** พร้อมฟีเจอร์ครบครัน รองรับการทดสอบ Output อัตโนมัติ, WiFi, NTP, Weather API และ OLED Display

---

## 📋 สารบัญ

- [คุณสมบัติ](#-คุณสมบัติ)
- [ฮาร์ดแวร์ที่ต้องใช้](#-ฮาร์ดแวร์ที่ต้องใช้)
- [การติดตั้ง](#-การติดตั้ง)
- [การกำหนดค่า](#-การกำหนดค่า)
- [โครงสร้างโปรเจกต์](#-โครงสร้างโปรเจกต์)
- [การใช้งาน](#-การใช้งาน)
- [คำอธิบาย Code](#-คำอธิบาย-code)
- [การแก้ปัญหา](#-การแก้ปัญหา)
- [License](#-license)

---

## 🎯 คุณสมบัติ

### ✨ ฟีเจอร์หลัก

- ✅ **ทดสอบ Output อัตโนมัติ** - วนทดสอบ Relay 3 ตัว และ AUX 4 ตัว ทุกๆ 1.5 วินาที
- ✅ **รองรับ Active Low** - Relay และ Input ทำงานแบบ Active Low
- ✅ **เชื่อมต่อ WiFi** - แสดงสถานะและความแรงสัญญาณ (RSSI)
- ✅ **ซิงค์เวลาจาก NTP** - เวลาแม่นยำจาก Internet (GMT+7 กรุงเทพฯ)
- ✅ **ข้อมูลสภาพอากาศ** - อุณหภูมิและความชื้นปัจจุบันจาก OpenWeatherMap
- ✅ **ตรวจสอบคุณภาพอากาศ (PM2.5)** - ค่า PM2.5, AQI และระดับคุณภาพอากาศ
- ✅ **แสดงผลบน OLED** - จอ 128x64 แสดงข้อมูลแบบ Real-time
- ✅ **ทำงานต่อได้แม้ OLED หลุด** - โปรแกรมไม่หยุดทำงานถ้าจอไม่ต่อ
- ✅ **Serial Monitor** - แสดงสถานะทุก 3 วินาทีสำหรับ Debug

### 🔌 Input/Output ที่รองรับ

**Inputs (5 ช่อง - Active Low):**
- SW1-3: Toggle Switches (GPIO 34, 35, 32)
- ISOIN1-2: Isolated Inputs (GPIO 33, 27)

**Outputs (8 ช่อง):**
- RL1-3: Relays - Active Low (GPIO 17, 16, 4)
- AUX1-4: TTL 3.3V Outputs (GPIO 12, 13, 14, 15)
- LED: Onboard LED (GPIO 2)

---

## 🛠️ ฮาร์ดแวร์ที่ต้องใช้

### อุปกรณ์หลัก
- ESP32 Dev Module (หรือบอร์ด ESPThaiTechZone V2.0)
- OLED Display SSD1306 128x64 I2C (Address: 0x3C) - *ไม่บังคับ*
- สาย USB สำหรับอัปโหลดโปรแกรม

### อุปกรณ์ทดสอบ (ตัวเลือก)
- Relay Module x3 (ต่อกับ RL1, RL2, RL3)
- LED หรือวงจรทดสอบสำหรับ AUX1-4
- Toggle Switch x3 (ต่อกับ SW1, SW2, SW3)
- Input Signal สำหรับ ISOIN1-2

### การเชื่อมต่อ I2C (OLED)
```
OLED SDA → GPIO 21 (SDA)
OLED SCL → GPIO 22 (SCL)
OLED VCC → 3.3V
OLED GND → GND
```

---

## 📦 การติดตั้ง

### ขั้นตอนที่ 1: ติดตั้ง PlatformIO

เลือกวิธีใดวิธีหนึ่ง:

**วิธีที่ 1: ใช้ VS Code (แนะนำ)**
1. ดาวน์โหลด [Visual Studio Code](https://code.visualstudio.com/)
2. เปิด VS Code → Extensions (Ctrl+Shift+X)
3. ค้นหา "PlatformIO IDE" และกด Install
4. รอให้ติดตั้งเสร็จ (อาจใช้เวลา 5-10 นาที)

**วิธีที่ 2: ใช้ Command Line**
```bash
pip install platformio
```

### ขั้นตอนที่ 2: Clone โปรเจกต์

```bash
git clone https://github.com/YOUR_USERNAME/DEVTHAITECHZONEV2.git
cd DEVTHAITECHZONEV2
```

หรือดาวน์โหลด ZIP และแตกไฟล์

### ขั้นตอนที่ 3: เปิดโปรเจกต์

**ใน VS Code:**
1. File → Open Folder
2. เลือกโฟลเดอร์ `DEVTHAITECHZONEV2`
3. PlatformIO จะติดตั้ง Library อัตโนมัติ

**Command Line:**
```bash
cd DEVTHAITECHZONEV2
pio lib install
```

### ขั้นตอนที่ 4: ตรวจสอบ Library

Library ที่จำเป็น (ระบุใน `platformio.ini`):
- ✅ Adafruit GFX Library
- ✅ Adafruit SSD1306
- ✅ ArduinoJson@^6.19.4

PlatformIO จะติดตั้งอัตโนมัติเมื่อ Build ครั้งแรก

---

## ⚙️ การกำหนดค่า

### 1. ตั้งค่า WiFi

แก้ไขในไฟล์ `src/main.cpp` (บรรทัดที่ ~30-31):

```cpp
const char* WIFI_SSID = "ชื่อ_WiFi_ของคุณ";
const char* WIFI_PASSWORD = "รหัสผ่าน_WiFi";
```

### 2. ตั้งค่า Weather API (ตัวเลือก)

**ขอ API Key ฟรี:**
1. ไปที่ [OpenWeatherMap](https://openweathermap.org/api)
2. สมัครสมาชิก (ฟรี)
3. ไปที่ API Keys และคัดลอก Key

**แก้ไขในโค้ด** (บรรทัดที่ ~43-44):

```cpp
const char* OPENWEATHERMAP_API_KEY = "API_KEY_ของคุณ";
const char* WEATHER_CITY = "เมือง,จังหวัด,TH";  // ตัวอย่าง: "Bangkok,Bangkok,TH"
```

### 3. ตั้งค่า Air Quality Location (ตัวเลือก)

**แก้ไขพิกัด GPS สำหรับตรวจสอบคุณภาพอากาศ** (บรรทัดที่ ~47-48):

```cpp
const float NAKHON_SI_THAMMARAT_LAT = 8.4304;   // Latitude
const float NAKHON_SI_THAMMARAT_LON = 99.9631;  // Longitude
```

**หาพิกัดของคุณ:**
- ใช้ Google Maps → คลิกขวาที่ตำแหน่ง → คัดลอกพิกัด
- API Key เดียวกับ Weather API (ไม่ต้องสมัครเพิ่ม)

### 4. ตั้งค่า Timezone (ถ้าต้องการเปลี่ยน)

แก้ไขในไฟล์ `src/main.cpp` (บรรทัดที่ ~40):

```cpp
const long GMT_OFFSET_SEC = 7 * 3600;  // GMT+7 สำหรับประเทศไทย
```

---

## 📁 โครงสร้างโปรเจกต์

```
DEVTHAITECHZONEV2/
│
├── src/
│   └── main.cpp              # โค้ดหลักของโปรแกรม
│
├── include/
│   └── README                # สำหรับ header files (ถ้ามี)
│
├── lib/
│   └── README                # สำหรับ custom libraries
│
├── test/
│   └── README                # สำหรับ unit tests
│
├── platformio.ini            # ไฟล์กำหนดค่า PlatformIO
├── BluePrint.md              # พิมพ์เขียวโปรเจกต์
└── README.md                 # คู่มือนี้
```

---

## 🚀 การใช้งาน

### วิธีที่ 1: ใช้ VS Code + PlatformIO

**Build โปรแกรม:**
1. กด `Ctrl+Alt+B` หรือคลิก ✓ (Build) ที่แถบล่าง
2. รอให้ compile เสร็จ

**Upload ไปยังบอร์ด:**
1. เสียบสาย USB เชื่อมต่อ ESP32
2. กด `Ctrl+Alt+U` หรือคลิก → (Upload) ที่แถบล่าง
3. รอให้ upload เสร็จ

**เปิด Serial Monitor:**
1. กด `Ctrl+Shift+M` หรือคลิก 🔌 (Serial Monitor)
2. Baud rate จะถูกตั้งเป็น 115200 อัตโนมัติ

### วิธีที่ 2: ใช้ Command Line

```bash
# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor
```

### การดู Output

**บน OLED Display (ถ้าต่อ):**
```
-> RELAY 1
──────────────────
SW:000 ISO:00
WiFi:-45dBm 14:30:25
16/10/25 28C H75%
PM35 AQI2 Fair
```

**บน Serial Monitor:**
```
========== Board Status ==========
Test: -> RELAY 1
Inputs: SW:000 ISO:00
WiFi: WiFi:-45dBm
Time: 14:30:25
Date: 16/10/2025
Weather: 28C H75%
AirQuality: PM2.5=35.2 AQI=2 (Fair)
==================================
```

---

## 💡 คำอธิบาย Code

### โครงสร้างหลักของโปรแกรม

#### 1. **ส่วน Include Libraries**
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
```
- **Arduino.h** - ฟังก์ชันพื้นฐาน ESP32
- **Wire.h** - สำหรับการสื่อสาร I2C
- **Adafruit_GFX.h, Adafruit_SSD1306.h** - สำหรับควบคุม OLED
- **WiFi.h, WiFiClientSecure.h** - สำหรับเชื่อมต่อ WiFi และ HTTPS
- **ArduinoJson.h** - สำหรับแปลงข้อมูล JSON จาก Weather API
- **time.h** - สำหรับจัดการเวลาจาก NTP

#### 2. **ส่วน Pin Definitions**
```cpp
#define SW1_PIN 34
#define SW2_PIN 35
// ... และอื่นๆ
```
กำหนด GPIO ที่ใช้สำหรับ Input/Output ทั้งหมด

#### 3. **ส่วน Configuration**
```cpp
const char* WIFI_SSID = "myHome_2.4GHz";
const char* NTP_SERVER1 = "pool.ntp.org";
// ... และอื่นๆ
```
ตั้งค่า WiFi, NTP Server, Weather API

#### 4. **ส่วน Global Variables**
```cpp
int current_test_step = 0;
bool oled_available = false;
bool time_synced = false;
// ... และอื่นๆ
```
ตัวแปรสำหรับเก็บสถานะต่างๆ

### ฟังก์ชันสำคัญ

#### **setup()**
ทำงานครั้งเดียวตอนเริ่มต้น:
1. เริ่มต้น Serial (115200 baud)
2. ตั้งค่า GPIO ทั้งหมด (Input/Output)
3. เริ่มต้น OLED Display
4. เชื่อมต่อ WiFi
5. Sync เวลาจาก NTP
6. ดึงข้อมูลสภาพอากาศครั้งแรก

#### **loop()**
ทำงานซ้ำไปเรื่อยๆ (Non-blocking):
1. เรียก `runTestCycle()` - ทดสอบ Output
2. อัปเดตสถานะ WiFi
3. อัปเดตข้อมูลสภาพอากาศ (ทุก 10 นาที)
4. อัปเดตข้อมูลคุณภาพอากาศ (ทุก 10 นาที)
5. อัปเดต Display

#### **runTestCycle()**
วนทดสอบ Output ทุก 1.5 วินาที:
- ปิด Output ทั้งหมด
- เปิด Output ที่กำลังทดสอบ
- วนไป RELAY 1 → RELAY 2 → RELAY 3 → AUX 1 → AUX 2 → AUX 3 → AUX 4 → RELAY 1...

#### **connectWiFi()**
เชื่อมต่อ WiFi:
- แสดงสถานะบน OLED และ Serial
- รอเชื่อมต่อสูงสุด 10 วินาที
- แสดง IP Address และ RSSI เมื่อเชื่อมต่อสำเร็จ

#### **syncNTPTime()**
ซิงค์เวลาจาก NTP Server:
- ใช้เวลาจาก pool.ntp.org, time.nist.gov, time.google.com
- ตั้งค่า Timezone GMT+7 สำหรับประเทศไทย
- อัปเดตเวลาทุกวินาที

#### **fetchWeatherData()**
ดึงข้อมูลสภาพอากาศ:
- เชื่อมต่อ HTTPS กับ OpenWeatherMap API
- แปลง JSON เป็นข้อมูล
- เก็บอุณหภูมิและความชื้นปัจจุบัน
- อัปเดตทุก 10 นาที

#### **fetchAirQualityData()**
ดึงข้อมูลคุณภาพอากาศ:
- เชื่อมต่อ HTTPS กับ OpenWeatherMap Air Pollution API
- ดึงข้อมูล PM2.5, PM10 และ AQI
- แปลงค่า AQI เป็นคำอธิบาย (Good, Fair, Moderate, Poor, VeryPoor)
- อัปเดตทุก 10 นาที

#### **getAQIQuality()**
แปลงค่า AQI เป็นคำอธิบาย:
- AQI 1 = Good (ดี)
- AQI 2 = Fair (ปานกลาง)
- AQI 3 = Moderate (พอใช้)
- AQI 4 = Poor (แย่)
- AQI 5 = VeryPoor (แย่มาก)

#### **updateOledDisplay()**
แสดงผลบน OLED และ Serial:
- แสดงบน Serial ทุก 3 วินาที
- แสดงบน OLED ทุก 1 วินาที (สำหรับเวลา)
- อัปเดตเมื่อข้อมูลเปลี่ยนเท่านั้น (ป้องกันกะพริบ)
- บรรทัดที่ 5: วันที่ (ย่อ) + อุณหภูมิ + ความชื้น
- บรรทัดที่ 6: PM2.5 + AQI + คำอธิบาย

### Active Low Configuration

**Relays (RL1-3):**
```cpp
digitalWrite(RL1_PIN, HIGH);  // OFF (ไม่ทำงาน)
digitalWrite(RL1_PIN, LOW);   // ON (ทำงาน)
```

**Inputs (SW1-3, ISOIN1-2):**
```cpp
pinMode(SW1_PIN, INPUT_PULLUP);  // เปิด Pull-up resistor ภายใน
// เมื่อกดหรือต่อ Ground → อ่านค่าได้ LOW
// เมื่อไม่กดหรือไม่ต่อ → อ่านค่าได้ HIGH
```

แสดงผลแบบ Inverted:
```cpp
digitalRead(SW1_PIN) ? "0" : "1";  // LOW แสดงเป็น "1", HIGH แสดงเป็น "0"
```

---

## 🔧 การแก้ปัญหา

### ปัญหาที่พบบ่อย

#### 1. **OLED ไม่แสดงผล**

**วิธีแก้:**
- ตรวจสอบการเชื่อมต่อ I2C (SDA=GPIO21, SCL=GPIO22)
- ตรวจสอบ I2C Address ด้วย I2C Scanner
- โปรแกรมจะทำงานต่อได้แม้ OLED ไม่ต่อ (ดูผ่าน Serial Monitor)

**LED จะกระพริบ 3 ครั้งถ้า OLED ไม่เจอ**

#### 2. **WiFi เชื่อมต่อไม่ได้**

**วิธีแก้:**
- ตรวจสอบ SSID และ Password ถูกต้อง
- ตรวจสอบว่า WiFi เป็น 2.4GHz (ESP32 ไม่รองรับ 5GHz)
- ตรวจสอบความแรงสัญญาณ
- ดูข้อความ Error ใน Serial Monitor

#### 3. **เวลาไม่ตรง**

**วิธีแก้:**
- ตรวจสอบว่า WiFi เชื่อมต่อแล้ว
- รอให้ NTP Sync (ประมาณ 5-10 วินาที)
- ตรวจสอบ GMT_OFFSET_SEC ตั้งค่าถูกต้อง (GMT+7 = 7*3600)

#### 4. **Weather API ไม่แสดงข้อมูล**

**วิธีแก้:**
- ตรวจสอบ API Key ถูกต้อง
- ตรวจสอบชื่อเมืองถูกต้อง (รูปแบบ: "City,State,TH")
- รอ 10 นาทีสำหรับการ Activate API Key (กรณี Key ใหม่)
- ตรวจสอบใน Serial Monitor ว่ามี Error อะไร

#### 5. **Relay ทำงานกลับด้าน**

**ตรวจสอบ:**
- Relay Module เป็นแบบ Active High หรือ Active Low
- ถ้าเป็น Active High ให้แก้โค้ด:
```cpp
digitalWrite(RL1_PIN, LOW);   // OFF
digitalWrite(RL1_PIN, HIGH);  // ON
```

#### 6. **Upload Error**

**วิธีแก้:**
- กดปุ่ม BOOT บนบอร์ดค้างไว้ระหว่าง Upload
- เปลี่ยน USB Port
- ตรวจสอบ Driver CP2102/CH340 ติดตั้งแล้ว
- ลอง Upload อีกครั้ง

#### 7. **Air Quality API ไม่แสดงข้อมูล**

**วิธีแก้:**
- ตรวจสอบ API Key ถูกต้อง (ใช้ Key เดียวกับ Weather)
- ตรวจสอบพิกัด GPS ถูกต้อง
- รอ 10 นาทีสำหรับการอัปเดตข้อมูล
- ตรวจสอบใน Serial Monitor ว่ามี Error อะไร
- ดูสถานะ: "AQI:Connecting...", "AQI:Timeout", etc.

---

## 📊 ตัวอย่างการใช้งาน

### กรณีที่ 1: ทดสอบบอร์ดพื้นฐาน
- ต่อ OLED Display
- Upload โปรแกรม
- สังเกตการทำงานของ LED และ Display
- ทดสอบกด Switch ดูการเปลี่ยนแปลงบน Display

### กรณีที่ 2: ทดสอบพร้อม WiFi + Weather + Air Quality
- ตั้งค่า WiFi และ API Key ให้ถูกต้อง
- Upload โปรแกรม
- ดูเวลา, อุณหภูมิ, ความชื้น และ PM2.5 บน Display
- ตรวจสอบ Serial Monitor เพื่อดูรายละเอียดครบถ้วน

### กรณีที่ 3: ใช้งานจริงกับ Relay
- ต่อ Relay Module กับ RL1, RL2, RL3
- ต่อโหลดกับ Relay (หลอดไฟ, มอเตอร์, ฯลฯ)
- Upload โปรแกรม
- สังเกตการทำงานของ Relay ทุก 1.5 วินาที

### กรณีที่ 4: ใช้งานโดยไม่มี OLED
- ถอด OLED ออก
- Upload โปรแกรม
- LED จะกระพริบ 3 ครั้ง
- เปิด Serial Monitor เพื่อดูสถานะ
- โปรแกรมจะทำงานต่อได้ปกติ

### กรณีที่ 5: ตรวจสอบคุณภาพอากาศ
- ดูค่า PM2.5 บน Display หรือ Serial Monitor
- เปรียบเทียบกับมาตรฐาน WHO:
  - PM2.5 < 12 µg/m³ = ดี
  - PM2.5 12-35 µg/m³ = ปานกลาง
  - PM2.5 35-55 µg/m³ = ไม่ดีต่อสุขภาพกลุ่มเสี่ยง
  - PM2.5 > 55 µg/m³ = ไม่ดีต่อสุขภาพ

---

## 🎓 เรียนรู้เพิ่มเติม

### สำหรับผู้เริ่มต้น
1. ศึกษา Arduino Basics
2. ทำความเข้าใจ GPIO, Input/Output
3. เรียนรู้เกี่ยว I2C Communication
4. ทดลองแก้ค่าต่างๆ ในโค้ด

### สำหรับผู้ใช้ขั้นสูง
1. เพิ่มการควบคุมผ่าน Web Server
2. เพิ่ม MQTT สำหรับ IoT
3. บันทึกข้อมูล Log ลง SD Card
4. เพิ่ม Automation Rules

### แหล่งข้อมูลที่เป็นประโยชน์
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- [OpenWeatherMap API](https://openweathermap.org/api)

---

## 🤝 การมีส่วนร่วม

หากต้องการพัฒนาเพิ่มเติม:
1. Fork repository นี้
2. สร้าง Branch ใหม่ (`git checkout -b feature/AmazingFeature`)
3. Commit การเปลี่ยนแปลง (`git commit -m 'Add some AmazingFeature'`)
4. Push ไปยัง Branch (`git push origin feature/AmazingFeature`)
5. เปิด Pull Request

---

## 📝 Changelog

### Version 2.1 (Development - Current)
- ✅ เพิ่ม Air Quality (PM2.5) monitoring
- ✅ เพิ่ม OpenWeatherMap Air Pollution API
- ✅ แสดง PM2.5, AQI และระดับคุณภาพอากาศ
- ✅ ย่อรูปแบบวันที่เป็น DD/MM/YY
- ✅ รวมวันที่ + อุณหภูมิ + ความชื้น ในบรรทัดเดียว
- ✅ แสดงคุณภาพอากาศพร้อมคำอธิบาย

### Version 2.0
- ✅ เพิ่ม WiFi connectivity
- ✅ เพิ่ม NTP time synchronization
- ✅ เพิ่ม Weather API integration
- ✅ เพิ่ม OLED fault tolerance
- ✅ เพิ่ม Serial Monitor output
- ✅ ปรับปรุง Active Low configuration
- ✅ ลบ RS485 communication

### Version 1.0
- ✅ Basic board testing
- ✅ OLED display
- ✅ GPIO testing

---

## 📄 License

โปรเจกต์นี้เผยแพร่ภายใต้ MIT License - ดูรายละเอียดในไฟล์ [LICENSE](LICENSE)

---

## 👨‍💻 ผู้พัฒนา

**ESPThaiTechZone Team**

- 📧 Email: nattapholj@gmail.com
- 🌐 Website: https://www.facebook.com/thaitechzone
- 💬 Support: LINE: thaitechzone

---

## 🙏 ขอบคุณ

ขอบคุณทุกท่านที่ใช้งานและพัฒนาโปรเจกต์นี้

**Libraries ที่ใช้:**
- Adafruit GFX Library
- Adafruit SSD1306
- ArduinoJson
- Arduino ESP32 Core

**Services:**
- OpenWeatherMap API
- NTP Pool Project

---

<div align="center">

**⭐ ถ้าชอบโปรเจกต์นี้ กด Star ให้เราด้วยนะครับ! ⭐**

Made with ❤️ by ESPThaiTechZone Team

</div>
