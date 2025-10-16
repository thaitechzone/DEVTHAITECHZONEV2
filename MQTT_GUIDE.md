# คู่มือการใช้งาน MQTT Control
## ESP32 ThaiTechZone V2.0 - MQTT IoT Dashboard

---

## 📋 สารบัญ
1. [ภาพรวม](#ภาพรวม)
2. [การตั้งค่า MQTT Broker](#การตั้งค่า-mqtt-broker)
3. [MQTT Topics](#mqtt-topics)
4. [โหมดการทำงาน](#โหมดการทำงาน)
5. [การควบคุมผ่าน MQTT](#การควบคุมผ่าน-mqtt)
6. [ตัวอย่างการใช้งาน](#ตัวอย่างการใช้งาน)
7. [Troubleshooting](#troubleshooting)

---

## 🎯 ภาพรวม

โปรเจคนี้รองรับการสื่อสารผ่าน MQTT Protocol เพื่อให้สามารถ:
- **ควบคุม** Relay และ AUX outputs จากระยะไกล
- **ตรวจสอบ** สถานะ Input switches และ Isolated inputs
- **รับข้อมูล** อากาศ (Temperature, Humidity) และคุณภาพอากาศ (PM2.5, AQI)
- **สลับโหมด** ระหว่าง Auto (Test Cycle) และ Manual (MQTT Control)

---

## 🔧 การตั้งค่า MQTT Broker

### ค่าเริ่มต้น (Default)
```cpp
MQTT_BROKER = "broker.hivemq.com"  // Free public broker
MQTT_PORT = 1883
MQTT_CLIENT_ID = "ESP32_ThaiTechZone_V2"
```

### MQTT Broker ที่แนะนำ

#### 1. **HiveMQ Public Broker** (ฟรี)
- Host: `broker.hivemq.com`
- Port: `1883`
- ข้อดี: ฟรี, ไม่ต้องลงทะเบียน
- ข้อจำกัด: ไม่มีการรักษาความปลอดภัย, ใช้ทดสอบเท่านั้น

#### 2. **Mosquitto (Self-hosted)**
- Host: `<Your_Server_IP>`
- Port: `1883`
- ข้อดี: ควบคุมเต็มที่, ปลอดภัย
- ต้องติดตั้ง: `sudo apt-get install mosquitto`

#### 3. **CloudMQTT / HiveMQ Cloud**
- Host: `<instance-url>`
- Port: `1883` หรือ `8883` (TLS)
- ข้อดี: มี Dashboard, รองรับ Authentication

### การแก้ไขค่า Broker ในโค้ด
แก้ไขไฟล์ `src/main.cpp` บรรทัด 31-33:
```cpp
const char* MQTT_BROKER = "your_broker_address";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "ESP32_ThaiTechZone_V2";
```

---

## 📡 MQTT Topics

### 1. Status Topics (Device → Dashboard)
| Topic | Type | Description | Example Payload |
|-------|------|-------------|-----------------|
| `thaitechzone/v2/status` | JSON | สถานะระบบทั้งหมด | `{"uptime":12345,"mode":"auto","wifi_rssi":-45,"relays":{"rl1":true,"rl2":false,"rl3":false},"aux":{"aux1":false,"aux2":false,"aux3":false,"aux4":false}}` |
| `thaitechzone/v2/inputs` | JSON | สถานะ Input ทั้งหมด | `{"sw1":false,"sw2":false,"sw3":false,"iso1":false,"iso2":false}` |
| `thaitechzone/v2/weather` | JSON | ข้อมูลอากาศ | `{"city":"Tha Sala","temperature":28,"humidity":75,"description":"ท้องฟ้าแจ่มใส"}` |
| `thaitechzone/v2/airquality` | JSON | ข้อมูลคุณภาพอากาศ | `{"pm2_5":35.2,"pm10":45.3,"aqi":2,"quality":"Fair"}` |
| `thaitechzone/v2/mode` | JSON | โหมดการทำงานปัจจุบัน | `{"mode":"auto"}` หรือ `{"mode":"manual"}` |
| `thaitechzone/v2/relay` | JSON | สถานะ Relay (เมื่อมีการเปลี่ยนแปลง) | `{"relay":1,"state":true}` |
| `thaitechzone/v2/aux` | JSON | สถานะ AUX (เมื่อมีการเปลี่ยนแปลง) | `{"aux":1,"state":true}` |

### 2. Command Topics (Dashboard → Device)
| Topic | Type | Description | Example Payload |
|-------|------|-------------|-----------------|
| `thaitechzone/v2/mode/cmd` | JSON | สั่งเปลี่ยนโหมด | `{"mode":"manual"}` หรือ `{"mode":"auto"}` |
| `thaitechzone/v2/relay/cmd` | JSON | ควบคุม Relay (Manual mode เท่านั้น) | `{"relay":1,"state":true}` |
| `thaitechzone/v2/aux/cmd` | JSON | ควบคุม AUX (Manual mode เท่านั้น) | `{"aux":2,"state":false}` |

### การอัปเดตข้อมูล
- **Status/Inputs/Weather/AirQuality**: อัปเดตทุก 5 วินาที
- **Weather**: อัปเดตทุก 10 นาที
- **AirQuality**: อัปเดตทุก 10 นาที
- **Relay/AUX**: อัปเดตทันทีเมื่อมีการเปลี่ยนแปลง

---

## 🔄 โหมดการทำงาน

### 1. **Auto Mode** (โหมดอัตโนมัติ)
- เป็นโหมดเริ่มต้น
- รัน Test Cycle อัตโนมัติ (เปิด-ปิด Relay/AUX ทีละตัว)
- ยังคง Publish สถานะผ่าน MQTT
- **ไม่สามารถควบคุมผ่าน MQTT ได้**
- แสดง "-> RELAY 1", "-> RELAY 2", ฯลฯ บน OLED

### 2. **Manual Mode** (โหมดควบคุมด้วยมือ)
- หยุด Test Cycle
- สามารถควบคุม Relay/AUX ผ่าน MQTT ได้
- ปิด Output ทั้งหมดเมื่อสลับเข้าโหมดนี้
- แสดง "MANUAL MODE" บน OLED

### การสลับโหมด
ส่งคำสั่งผ่าน MQTT:
```json
// สลับเป็น Manual Mode
Topic: thaitechzone/v2/mode/cmd
Payload: {"mode":"manual"}

// สลับเป็น Auto Mode
Topic: thaitechzone/v2/mode/cmd
Payload: {"mode":"auto"}
```

---

## 🎮 การควบคุมผ่าน MQTT

### ⚠️ ข้อกำหนดสำคัญ
- **ต้องอยู่ใน Manual Mode ก่อน** ถึงจะควบคุม Relay/AUX ได้
- Relay ใช้ Active Low (LOW = ON, HIGH = OFF)
- AUX ใช้ Active High (HIGH = ON, LOW = OFF)

### ควบคุม Relay
```json
// เปิด Relay 1
Topic: thaitechzone/v2/relay/cmd
Payload: {"relay":1,"state":true}

// ปิด Relay 2
Topic: thaitechzone/v2/relay/cmd
Payload: {"relay":2,"state":false}

// Relay options: 1, 2, 3
// State: true (ON) หรือ false (OFF)
```

### ควบคุม AUX Outputs
```json
// เปิด AUX 3
Topic: thaitechzone/v2/aux/cmd
Payload: {"aux":3,"state":true}

// ปิด AUX 4
Topic: thaitechzone/v2/aux/cmd
Payload: {"aux":4,"state":false}

// AUX options: 1, 2, 3, 4
// State: true (ON) หรือ false (OFF)
```

---

## 💡 ตัวอย่างการใช้งาน

### ตัวอย่างที่ 1: ควบคุมด้วย MQTT Explorer
1. ดาวน์โหลด [MQTT Explorer](http://mqtt-explorer.com/)
2. เชื่อมต่อไปยัง Broker (`broker.hivemq.com`)
3. Subscribe: `thaitechzone/v2/#` (รับข้อมูลทั้งหมด)
4. สลับเป็น Manual Mode:
   - Publish to: `thaitechzone/v2/mode/cmd`
   - Payload: `{"mode":"manual"}`
5. เปิด Relay 1:
   - Publish to: `thaitechzone/v2/relay/cmd`
   - Payload: `{"relay":1,"state":true}`

### ตัวอย่างที่ 2: ควบคุมด้วย Python (paho-mqtt)
```python
import paho.mqtt.client as mqtt
import json
import time

# เชื่อมต่อ MQTT Broker
client = mqtt.Client()
client.connect("broker.hivemq.com", 1883, 60)

# ฟังก์ชันรับข้อมูลสถานะ
def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}")
    print(f"Payload: {msg.payload.decode()}\n")

client.on_message = on_message
client.subscribe("thaitechzone/v2/status")
client.loop_start()

# สลับเป็น Manual Mode
client.publish("thaitechzone/v2/mode/cmd", json.dumps({"mode":"manual"}))
time.sleep(1)

# เปิด Relay 1
client.publish("thaitechzone/v2/relay/cmd", json.dumps({"relay":1,"state":True}))
time.sleep(5)

# ปิด Relay 1
client.publish("thaitechzone/v2/relay/cmd", json.dumps({"relay":1,"state":False}))
time.sleep(5)

# สลับกลับเป็น Auto Mode
client.publish("thaitechzone/v2/mode/cmd", json.dumps({"mode":"auto"}))

client.loop_stop()
client.disconnect()
```

### ตัวอย่างที่ 3: ควบคุมด้วย Node-RED
1. ติดตั้ง Node-RED: `npm install -g node-red`
2. เริ่ม Node-RED: `node-red`
3. เปิดเบราว์เซอร์: `http://localhost:1880`
4. เพิ่ม MQTT nodes:
   - **MQTT In**: Subscribe `thaitechzone/v2/status`
   - **MQTT Out**: Publish `thaitechzone/v2/relay/cmd`
   - **Inject Node**: สร้างปุ่มควบคุม
   - **Function Node**: สร้าง JSON payload

### ตัวอย่างที่ 4: สร้าง Dashboard ด้วย Home Assistant
```yaml
# configuration.yaml
mqtt:
  broker: broker.hivemq.com
  port: 1883

switch:
  - platform: mqtt
    name: "Relay 1"
    state_topic: "thaitechzone/v2/status"
    command_topic: "thaitechzone/v2/relay/cmd"
    value_template: "{{ value_json.relays.rl1 }}"
    payload_on: '{"relay":1,"state":true}'
    payload_off: '{"relay":1,"state":false}'
    state_on: true
    state_off: false

sensor:
  - platform: mqtt
    name: "Temperature"
    state_topic: "thaitechzone/v2/weather"
    value_template: "{{ value_json.temperature }}"
    unit_of_measurement: "°C"
  
  - platform: mqtt
    name: "PM2.5"
    state_topic: "thaitechzone/v2/airquality"
    value_template: "{{ value_json.pm2_5 }}"
    unit_of_measurement: "µg/m³"
```

---

## 🔍 Troubleshooting

### ปัญหา: MQTT Connection Failed
**สาเหตุ:**
- Broker address ไม่ถูกต้อง
- Broker ไม่ online
- WiFi ไม่เชื่อมต่อ

**แก้ไข:**
1. ตรวจสอบ Serial Monitor: `MQTT Connection Failed, rc=-2`
2. ทดสอบ Broker ด้วย MQTT Explorer
3. ตรวจสอบ WiFi connection
4. ลองเปลี่ยน Client ID (ต้องไม่ซ้ำกับ client อื่น)

### ปัญหา: ควบคุม Relay ไม่ได้
**สาเหตุ:**
- ยังอยู่ใน Auto Mode
- JSON format ไม่ถูกต้อง
- Relay number ไม่ถูกต้อง (ต้องเป็น 1-3)

**แก้ไข:**
1. สลับเป็น Manual Mode ก่อน:
   ```json
   {"mode":"manual"}
   ```
2. ตรวจสอบ JSON payload ถูกต้อง:
   ```json
   {"relay":1,"state":true}  // ✅ ถูกต้อง
   {relay:1,state:true}      // ❌ ผิด (ไม่มี quotes)
   ```

### ปัญหา: ไม่เห็นข้อมูลบน Dashboard
**สาเหตุ:**
- Subscribe topic ไม่ถูกต้อง
- ข้อมูลยังไม่อัปเดต (รอ 5 วินาที)

**แก้ไข:**
1. Subscribe: `thaitechzone/v2/#` (รับข้อมูลทั้งหมด)
2. ตรวจสอบ Serial Monitor ว่ามีการ Publish ข้อมูลหรือไม่
3. ตรวจสอบ MQTT connected status

### ปัญหา: Multiple Clients ทำให้ Disconnect
**สาเหตุ:**
- MQTT_CLIENT_ID ซ้ำกัน

**แก้ไข:**
- แก้ไข Client ID ให้ไม่ซ้ำกัน:
  ```cpp
  const char* MQTT_CLIENT_ID = "ESP32_ThaiTechZone_V2_001";  // เพิ่มหมายเลขท้าย
  ```

---

## 📊 MQTT Message Flow

```
┌─────────────────┐         ┌──────────────┐         ┌─────────────────┐
│   ESP32 Device  │◄────────│ MQTT Broker  │────────►│  Dashboard/App  │
└─────────────────┘         └──────────────┘         └─────────────────┘
         │                         │                          │
         │ Publish Status          │                          │
         │────────────────────────►│                          │
         │ (Every 5 seconds)       │ Forward to Subscribers   │
         │                         │─────────────────────────►│
         │                         │                          │
         │                         │ Publish Command          │
         │                         │◄─────────────────────────│
         │ Receive Command         │                          │
         │◄────────────────────────│                          │
         │                         │                          │
         │ Execute & Publish State │                          │
         │────────────────────────►│─────────────────────────►│
         │                         │                          │
```

---

## 🔐 ข้อควรระวังด้านความปลอดภัย

### 1. Public Broker (HiveMQ)
⚠️ **อย่าใช้ในระบบจริง!**
- ทุกคนสามารถรับ/ส่งข้อมูลได้
- ไม่มีการเข้ารหัส
- เหมาะสำหรับทดสอบเท่านั้น

### 2. Private Broker แนะนำ
✅ **ใช้สำหรับระบบจริง**
- ตั้ง Username/Password
- ใช้ TLS/SSL (Port 8883)
- จำกัด IP ที่เชื่อมต่อได้

### 3. Topic Security
- ใช้ Base Topic ที่ซับซ้อน: `company/location/device_id/...`
- ไม่ใช้ข้อมูลส่วนตัวใน Topic name

---

## 📝 สรุป

การเพิ่ม MQTT ทำให้บอร์ด ESP32 ThaiTechZone V2.0 กลายเป็นอุปกรณ์ IoT ที่สมบูรณ์:

✅ ควบคุมได้จากทุกที่ผ่าน Internet  
✅ รองรับ Dashboard/Mobile App  
✅ สลับระหว่าง Auto/Manual Mode ได้  
✅ รับข้อมูล Sensor แบบ Real-time  
✅ รองรับ Home Automation Platform  

**ขั้นตอนต่อไป:**
1. ทดสอบด้วย MQTT Explorer
2. สร้าง Dashboard ด้วย Node-RED หรือ Home Assistant
3. Deploy ระบบจริงด้วย Private MQTT Broker

---

**เอกสารนี้จัดทำโดย:** ThaiTechZone Team  
**เวอร์ชัน:** 2.1 (MQTT Integration)  
**อัปเดตล่าสุด:** 16 ตุลาคม 2025
