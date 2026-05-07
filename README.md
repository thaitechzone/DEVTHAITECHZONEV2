# ESPThaiTechZone V2.0 Board Test

เฟิร์มแวร์ทดสอบฮาร์ดแวร์สำหรับ ESP32 (PlatformIO) เน้นตรวจสอบอินพุต/เอาต์พุตพื้นฐานแบบเสถียร ใช้งานได้ทั้งกรณีมีและไม่มีจอ OLED

## สถานะเวอร์ชันนี้

- ไม่มีการใช้งาน DHT22
- ไม่มี dependency ของ DHT ในโปรเจกต์
- GPIO13 ไม่ถูกใช้เป็น DHT data pin

## ฟีเจอร์หลัก

- ควบคุมรีเลย์ 3 ช่องแบบ Active Low
- รับสวิตช์ 3 ช่องแบบ Active Low (INPUT_PULLUP)
- รับ isolated input 2 ช่องแบบ Active Low
- LED บนบอร์ดติดเมื่อ ISOIN1 หรือ ISOIN2 active
- มี startup sequence (รีเลย์ ON 4 วินาที, OFF 2 วินาที)
- หลัง startup สามารถ toggle รีเลย์ด้วย SW1-SW3
- แสดงสถานะทาง Serial ทุก 2 วินาที
- รองรับ OLED SSD1306 (I2C 0x3C) แบบ optional

## ผังขา (ตามโค้ดปัจจุบัน)

- SW1: GPIO34
- SW2: GPIO35
- SW3: GPIO32
- ISOIN1: GPIO33
- ISOIN2: GPIO27
- RELAY1: GPIO17 (Active Low)
- RELAY2: GPIO16 (Active Low)
- RELAY3: GPIO4 (Active Low)
- LED: GPIO2

## การติดตั้ง

1. ติดตั้ง VS Code + PlatformIO IDE
2. เปิดโฟลเดอร์โปรเจกต์นี้
3. Build:

```bash
pio run
```

4. Upload:

```bash
pio run --target upload
```

5. เปิด Serial Monitor:

```bash
pio device monitor -b 115200
```

## Library ที่ใช้

กำหนดไว้ใน platformio.ini:
- Adafruit GFX Library
- Adafruit SSD1306
- ArduinoJson

หมายเหตุ: แม้ ArduinoJson จะยังคงอยู่ใน dependencies แต่เฟิร์มแวร์ปัจจุบันเน้นทดสอบ IO และไม่ได้อ่าน DHT22

## พฤติกรรมระบบ

- เริ่มต้นระบบ: รีเลย์ทุกช่อง ON 4 วินาที
- จากนั้นรีเลย์ทุกช่อง OFF 2 วินาที
- เข้าสู่โหมดทำงานปกติ: รีเลย์ทุกช่อง ON และรอรับคำสั่งสวิตช์
- กด SW1/SW2/SW3 เพื่อ toggle RELAY1/2/3
- เมื่อ ISOIN1 หรือ ISOIN2 เป็น LOW จะเปิด LED

## โครงสร้างไฟล์

- src/main.cpp: โค้ดหลัก
- platformio.ini: ค่าคอนฟิกและ dependencies
- BluePrint.md: เอกสารสเปก/แนวทางพัฒนา

## หมายเหตุการพัฒนา

หากต้องการเพิ่มเซนเซอร์หรือฟีเจอร์เครือข่ายในรอบถัดไป แนะนำให้เพิ่มแบบ non-blocking ด้วย millis() เพื่อคงความเสถียรของงานทดสอบ IO
