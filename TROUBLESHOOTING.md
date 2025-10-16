# 🔧 ThingsBoard Telemetry Troubleshooting Guide

## ปัญหา: Latest Telemetry ไม่แสดงค่า

### ✅ สิ่งที่ทำงานแล้ว:
- MQTT Connected สำเร็จ (State Code: 0)
- Access Token ถูกต้อง: `37u5JARLNBIgKmTUnmDM`
- WiFi เชื่อมต่อปกติ
- RPC Subscribe สำเร็จ

### ❌ ปัญหาที่พบ:
Latest Telemetry ใน ThingsBoard ไม่แสดงข้อมูล

---

## 🔍 สาเหตุที่เป็นไปได้

### 1. **MQTT Buffer Size ไม่พอ**
```cpp
// ปัจจุบัน: 1024 bytes
const int MQTT_BUFFER_SIZE = 1024;

// JSON Size: ~600-700 bytes (รวม Weather + AQI)
```

**วิธีเช็ค**: ดู Serial Monitor
```
│ JSON Size: 687 bytes (Buffer: 1024 bytes)
```

### 2. **QoS Level ไม่ถูกต้อง**
PubSubClient ใช้ QoS 0 เป็น default = ส่งแล้วลืม (ไม่รับประกัน)

### 3. **Topic Format ผิด**
ThingsBoard ต้องการ Topic แบบนี้:
```
v1/devices/me/telemetry  ✅ ถูกต้อง
```

### 4. **JSON Format ไม่ถูกต้อง**
ThingsBoard ต้องการ **Flat JSON** ไม่ใช่ Nested

**❌ ผิด (Nested)**:
```json
{
  "sensors": {
    "temperature": 28,
    "humidity": 75
  }
}
```

**✅ ถูก (Flat)**:
```json
{
  "temperature": 28,
  "humidity": 75
}
```

### 5. **String Object ใน JSON**
ThingsBoard อาจมีปัญหากับ String ที่มี value "N/A"

---

## 🛠️ วิธีแก้ปัญหา

### Solution 1: ลด JSON Size (แนะนำ)
ส่งเฉพาะข้อมูลสำคัญ ไม่ส่ง Weather/AQI ถ้ายังไม่มีข้อมูล

```cpp
// ไม่ส่ง Weather/AQI ถ้า invalid
if (weather_data_valid) {
  doc["temperature"] = current_weather.temp_current;
  doc["humidity"] = current_weather.humidity;
  doc["weather_city"] = current_weather.city_name;
  doc["weather_description"] = current_weather.description;
}

if (aqi_data_valid) {
  doc["pm2_5"] = air_quality.pm2_5;
  doc["pm10"] = air_quality.pm10;
  doc["aqi"] = air_quality.aqi;
  doc["air_quality"] = air_quality.quality;
}
```

### Solution 2: แยกส่ง Telemetry
ส่ง System Data แยกจาก Weather/AQI

```cpp
// Message 1: System Data (จำเป็น)
{
  "uptime": 123,
  "mode": "auto",
  "wifi_rssi": -65,
  "relay1": false,
  "relay2": true,
  ...
}

// Message 2: Weather (optional)
{
  "temperature": 28,
  "humidity": 75
}

// Message 3: AQI (optional)
{
  "pm2_5": 12.5,
  "aqi": 2
}
```

### Solution 3: เพิ่ม QoS Level
```cpp
// QoS 1 = รับประกันว่าส่งถึง
mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str(), true);
//                                                        ^^^^ QoS 1
```

### Solution 4: ลด String Overhead
```cpp
// แทนที่ String ด้วย const char*
doc["air_quality"] = aqi_data_valid ? air_quality.quality.c_str() : "";
```

---

## 🧪 วิธีทดสอบ

### Test 1: ส่ง JSON ขนาดเล็ก
```cpp
void testTelemetry() {
  DynamicJsonDocument doc(256);
  doc["test"] = "hello";
  doc["value"] = 123;
  
  String output;
  serializeJson(doc, output);
  
  bool sent = mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str());
  Serial.print("Test telemetry sent: ");
  Serial.println(sent ? "SUCCESS" : "FAILED");
}
```

### Test 2: เช็คว่า ThingsBoard รับได้หรือไม่
1. ไปที่ Device → Latest Telemetry
2. กด Refresh
3. ถ้าเห็น "test" = "hello" แสดงว่าใช้งานได้

### Test 3: ดู MQTT Debug
```cpp
// เพิ่มใน setup()
mqttClient.setDebug(true);  // ถ้า library รองรับ
```

---

## 📊 ตัวอย่างการแก้ไข

### แก้ไข publishMQTTStatus() ให้ส่งเฉพาะข้อมูลที่มี

```cpp
void publishMQTTStatus() {
  if (!mqttClient.connected()) {
    Serial.println("⚠️  MQTT not connected");
    return;
  }
  
  // สร้าง JSON ขนาดเล็ก
  DynamicJsonDocument doc(512);  // ลดจาก 2048
  
  // System Data (จำเป็น - ส่งเสมอ)
  doc["uptime"] = millis() / 1000;
  doc["mode"] = auto_mode ? "auto" : "manual";
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();
  
  // Output States
  doc["relay1"] = !digitalRead(RL1_PIN);
  doc["relay2"] = !digitalRead(RL2_PIN);
  doc["relay3"] = !digitalRead(RL3_PIN);
  doc["aux1"] = digitalRead(AUX1_PIN);
  doc["aux2"] = digitalRead(AUX2_PIN);
  doc["aux3"] = digitalRead(AUX3_PIN);
  doc["aux4"] = digitalRead(AUX4_PIN);
  
  // Input States
  doc["switch1"] = !digitalRead(SW1_PIN);
  doc["switch2"] = !digitalRead(SW2_PIN);
  doc["switch3"] = !digitalRead(SW3_PIN);
  doc["iso_input1"] = !digitalRead(ISOIN1_PIN);
  doc["iso_input2"] = !digitalRead(ISOIN2_PIN);
  
  // Weather - ส่งเฉพาะเมื่อมีข้อมูล
  if (weather_data_valid) {
    doc["temperature"] = current_weather.temp_current;
    doc["humidity"] = current_weather.humidity;
  }
  
  // AQI - ส่งเฉพาะเมื่อมีข้อมูล
  if (aqi_data_valid) {
    doc["pm2_5"] = air_quality.pm2_5;
    doc["aqi"] = air_quality.aqi;
  }
  
  // Serialize
  String output;
  size_t jsonSize = serializeJson(doc, output);
  
  Serial.print("📤 JSON Size: ");
  Serial.print(jsonSize);
  Serial.println(" bytes");
  
  // Publish
  bool published = mqttClient.publish(TB_TOPIC_TELEMETRY, output.c_str());
  
  if (published) {
    Serial.println("✅ Sent!");
  } else {
    Serial.println("❌ Failed!");
    Serial.println(output);  // แสดง JSON ที่ส่งไม่สำเร็จ
  }
}
```

---

## 🎯 ขั้นตอนแก้ไขแบบละเอียด

### Step 1: แก้ไขเฉพาะส่วน publishMQTTStatus()
ใช้โค้ดด้านบนแทนที่ฟังก์ชันเดิม

### Step 2: Upload และทดสอบ
```bash
platformio run --target upload
```

### Step 3: เปิด Serial Monitor
```bash
platformio device monitor
```

### Step 4: ดูผลลัพธ์
```
📤 JSON Size: 285 bytes
✅ Sent!
```

### Step 5: เช็ค ThingsBoard
- Device → Latest Telemetry
- กด Refresh
- ควรเห็นข้อมูล uptime, mode, relay1-3, aux1-4

---

## 📝 Checklist

- [ ] MQTT Connected = ✅
- [ ] JSON Size < 1024 bytes
- [ ] ไม่มี JSON overflow error
- [ ] Serial Monitor แสดง "✅ Sent!"
- [ ] ThingsBoard Latest Telemetry แสดงข้อมูล
- [ ] ถ้ายังไม่แสดง → ลองส่ง JSON ขนาดเล็ก (test message)
- [ ] ถ้ายังไม่แสดง → ลบ Device และสร้างใหม่
- [ ] ถ้ายังไม่แสดง → ใช้ ThingsBoard Cloud แทน Demo

---

## 🆘 ถ้ายังไม่ได้

### Option 1: ใช้ Smaller JSON
ส่งเฉพาะ 5-6 ค่าก่อน
```json
{
  "uptime": 123,
  "relay1": false,
  "relay2": true,
  "wifi_rssi": -65,
  "mode": "auto"
}
```

### Option 2: ลองใช้ ThingsBoard Cloud
```cpp
const char* MQTT_BROKER = "thingsboard.cloud";  // แทน demo.thingsboard.io
```

### Option 3: ตรวจสอบ Device Profile
1. Device → Device Profile
2. เช็คว่า Transport = MQTT
3. เช็คว่าไม่มี Validation rules ที่ block data

### Option 4: ดู Audit Logs
1. ThingsBoard → Audit Logs
2. Filter by Entity = Device
3. ดูว่ามี Error อะไร

---

## 📞 ติดต่อสอบถาม
ถ้าทำตามทุกขั้นตอนแล้วยังไม่ได้ ให้ส่งข้อมูลเหล่านี้มา:
- Serial Monitor output (ทั้งหมด)
- ThingsBoard Device Audit Log
- JSON ที่ส่ง (copy จาก Serial)
- Screenshots จาก ThingsBoard

