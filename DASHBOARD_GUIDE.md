# สร้าง Dashboard ใน ThingsBoard - คู่มือทีละขั้นตอน
## ESP32 ThaiTechZone V2.0 - Dashboard Creation Guide

---

## 📋 สารบัญ
1. [เตรียมความพร้อม](#เตรียมความพร้อม)
2. [สร้าง Dashboard พื้นฐาน](#สร้าง-dashboard-พื้นฐาน)
3. [เพิ่ม Widgets แต่ละประเภท](#เพิ่ม-widgets-แต่ละประเภท)
4. [Import Dashboard สำเร็จรูป](#import-dashboard-สำเร็จรูป)
5. [Mobile Dashboard](#mobile-dashboard)

---

## 🎯 เตรียมความพร้อม

### ขั้นตอนที่ 1: Login เข้า ThingsBoard

1. เปิดเบราว์เซอร์ไปที่:
   - **Demo**: https://demo.thingsboard.io/login
   - **Cloud**: https://thingsboard.cloud/login

2. Login ด้วย:
   - Email: `your_email@example.com`
   - Password: `your_password`

### ขั้นตอนที่ 2: ตรวจสอบ Device

1. ไปที่ **Entities** → **Devices**
2. ค้นหา: `ESP32_ThaiTechZone_V2`
3. คลิกที่ Device
4. เปิดแท็บ **"Latest telemetry"**
5. ตรวจสอบว่ามีข้อมูลเหล่านี้:
   ```
   ✅ temperature
   ✅ humidity
   ✅ pm2_5
   ✅ aqi
   ✅ relay1, relay2, relay3
   ✅ aux1, aux2, aux3, aux4
   ✅ wifi_rssi
   ✅ uptime
   ```

⚠️ **ถ้ายังไม่มีข้อมูล:**
- ตรวจสอบ ESP32 เชื่อมต่อ WiFi แล้ว
- ตรวจสอบ Access Token ถูกต้อง
- ดู Serial Monitor: `✅ Telemetry sent to ThingsBoard`

---

## 📊 สร้าง Dashboard พื้นฐาน

### ขั้นตอนที่ 1: สร้าง Dashboard ใหม่

1. ไปที่ **Dashboards** (เมนูซ้าย)
2. คลิก **"+"** (Add dashboard)
3. กรอกข้อมูล:
   ```
   Title: ESP32 Control Panel
   Description: IoT Control Dashboard for ThaiTechZone V2
   ```
4. คลิก **"Add"**
5. คลิกที่ Dashboard ที่สร้างเพื่อเปิด

### ขั้นตอนที่ 2: เข้าสู่โหมดแก้ไข

1. คลิกปุ่ม **"Edit mode"** (ไอคอนดินสอ ✏️) มุมขวาล่าง
2. ตอนนี้คุณสามารถเพิ่ม Widgets ได้แล้ว

---

## 🎨 เพิ่ม Widgets แต่ละประเภท

## Widget 1: System Status Card 📊

**วัตถุประสงค์:** แสดงข้อมูลระบบแบบตาราง

### ขั้นตอน:

1. คลิก **"+"** (Add new widget) → **"Create new widget"**

2. เลือก **Widget Bundle**: `Cards`

3. เลือก **Widget Type**: `Entities table`

4. **Add datasource:**
   - Type: `Entity`
   - Device: `ESP32_ThaiTechZone_V2`

5. **เลือก Keys to show:**
   ```
   ☑ uptime
   ☑ mode
   ☑ wifi_rssi
   ☑ free_heap
   ☑ temperature
   ☑ humidity
   ☑ pm2_5
   ☑ aqi
   ```

6. **Widget Settings:**
   - Title: `System Status`
   - Enable search: ☑
   - Display pagination: ☑

7. คลิก **"Add"**

### ผลลัพธ์:
```
┌─────────────────────────────┐
│   System Status             │
├──────────────┬──────────────┤
│ uptime       │ 12345 s      │
│ mode         │ auto         │
│ wifi_rssi    │ -45 dBm      │
│ temperature  │ 28 °C        │
│ humidity     │ 75 %         │
│ pm2_5        │ 35.2 µg/m³   │
│ aqi          │ 2            │
└──────────────┴──────────────┘
```

---

## Widget 2: Temperature & Humidity Chart 📈

**วัตถุประสงค์:** แสดงกราฟอุณหภูมิและความชื้นย้อนหลัง

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Charts`

3. เลือก **Widget Type**: `Timeseries - Line Chart`

4. **Add datasources:**
   
   **Datasource 1:**
   - Type: `Entity`
   - Device: `ESP32_ThaiTechZone_V2`
   - Keys: `temperature`
   - Label: `Temperature`
   - Color: `#FF5722` (สีส้ม)
   
   **Datasource 2:**
   - Type: `Entity`
   - Device: `ESP32_ThaiTechZone_V2`
   - Keys: `humidity`
   - Label: `Humidity`
   - Color: `#2196F3` (สีน้ำเงิน)

5. **Widget Settings:**
   - Title: `Temperature & Humidity History`
   - Show legend: ☑
   - Enable data export: ☑

6. **Time window:**
   - Realtime - Last: `1 hour` (หรือ `24 hours`)

7. **Advanced Settings:**
   - Y-axis min: `0`
   - Y-axis max: `100`
   - Grid: ☑

8. คลิก **"Add"**

### ผลลัพธ์:
```
Temperature & Humidity History
100 ┤                    
 80 ┤    ╭─╮  ╭─╮       Humidity (blue)
 60 ┤  ╭╯   ╰╯   ╰╮     
 40 ┤╭╯           ╰╮    
 20 ┼─────────────────  Temperature (orange)
  0 ┴────────────────
    12h  8h   4h   Now
```

---

## Widget 3: PM2.5 Gauge 🎯

**วัตถุประสงค์:** แสดงค่า PM2.5 แบบมาตรวัด

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Analogue gauges`

3. เลือก **Widget Type**: `Radial gauge`

4. **Add datasource:**
   - Type: `Entity`
   - Device: `ESP32_ThaiTechZone_V2`
   - Keys: `pm2_5`

5. **Widget Settings:**
   - Title: `PM2.5 Air Quality`
   - Units: `µg/m³`
   - Decimals: `1`

6. **Gauge Settings:**
   - Min value: `0`
   - Max value: `100`
   - Major ticks: `[0, 25, 50, 75, 100]`

7. **Thresholds (Color zones):**
   ```
   0-12:   Green   (#4CAF50) - Good
   12-35:  Yellow  (#FFEB3B) - Moderate
   35-55:  Orange  (#FF9800) - Unhealthy
   55-100: Red     (#F44336) - Hazardous
   ```

8. **Advanced:**
   - Animation: ☑
   - Needle: ☑

9. คลิก **"Add"**

### ผลลัพธ์:
```
       PM2.5 Air Quality
           ╭───╮
      100 ─┤   │
       75 ─┤   │     ╱
       50 ─┤  ╱│    (35.2 µg/m³)
       25 ─│╱  │
        0 ─┴───┘
      Moderate (Yellow zone)
```

---

## Widget 4: Relay Control Switches 🔌

**วัตถุประสงค์:** ควบคุม Relay แบบ Toggle Switch

### ขั้นตอน Relay 1:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Control widgets`

3. เลือก **Widget Type**: `Switch control`

4. **Target device:**
   - Type: `Entity`
   - Device: `ESP32_ThaiTechZone_V2`

5. **Widget Settings:**
   - Title: `Relay 1`
   - Initial state: อ่านจาก `relay1` telemetry

6. **RPC Settings:**
   
   **Set value method:**
   - RPC method: `setRelay`
   
   **Request body (ON):**
   ```json
   {
     "relay": 1,
     "state": true
   }
   ```
   
   **Request body (OFF):**
   ```json
   {
     "relay": 1,
     "state": false
   }
   ```

7. **Get value method:**
   - RPC method: `getValue`
   - Response key: `relays.rl1` (หรืออ่านจาก telemetry `relay1`)

8. **Appearance:**
   - Switch color: `#4CAF50` (Green)
   - Label: `Relay 1`
   - Icon: `power` (ถ้ามี)

9. คลิก **"Add"**

### ทำซ้ำสำหรับ Relay 2 และ 3:
- Relay 2: เปลี่ยน `"relay": 2`
- Relay 3: เปลี่ยน `"relay": 3`

### ผลลัพธ์:
```
┌──────────┬──────────┬──────────┐
│ Relay 1  │ Relay 2  │ Relay 3  │
│  ●────○  │  ○────●  │  ○────●  │
│   ON     │  OFF     │  OFF     │
└──────────┴──────────┴──────────┘
```

---

## Widget 5: AUX Control Buttons ⚡

**วัตถุประสงค์:** ควบคุม AUX outputs แบบปุ่ม

### ขั้นตอน AUX 1 ON:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Control widgets`

3. เลือก **Widget Type**: `RPC Button`

4. **Target device:**
   - Device: `ESP32_ThaiTechZone_V2`

5. **Widget Settings:**
   - Button label: `AUX 1 ON`
   - Button color: `#4CAF50` (Green)
   - Icon: `flash_on`

6. **RPC Settings:**
   - RPC method: `setAux`
   
   **Request body:**
   ```json
   {
     "aux": 1,
     "state": true
   }
   ```

7. คลิก **"Add"**

### สร้างปุ่ม AUX 1 OFF:
- Button label: `AUX 1 OFF`
- Button color: `#F44336` (Red)
- Icon: `flash_off`
- Request body: `{"aux": 1, "state": false}`

### ทำซ้ำสำหรับ AUX 2, 3, 4

### ผลลัพธ์:
```
┌─────────┬─────────┐  ┌─────────┬─────────┐
│ AUX 1   │ AUX 1   │  │ AUX 2   │ AUX 2   │
│  ON ⚡  │  OFF ⚡ │  │  ON ⚡  │  OFF ⚡ │
└─────────┴─────────┘  └─────────┴─────────┘

┌─────────┬─────────┐  ┌─────────┬─────────┐
│ AUX 3   │ AUX 3   │  │ AUX 4   │ AUX 4   │
│  ON ⚡  │  OFF ⚡ │  │  ON ⚡  │  OFF ⚡ │
└─────────┴─────────┘  └─────────┴─────────┘
```

---

## Widget 6: Mode Control 🔄

**วัตถุประสงค์:** สลับโหมด Auto/Manual

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Control widgets`

3. เลือก **Widget Type**: `Switch control`

4. **Target device:**
   - Device: `ESP32_ThaiTechZone_V2`

5. **Widget Settings:**
   - Title: `Mode Control`
   - Label ON: `MANUAL`
   - Label OFF: `AUTO`

6. **RPC Settings:**
   
   **Set value method (Manual):**
   ```json
   {
     "mode": "manual"
   }
   ```
   
   **Set value method (Auto):**
   ```json
   {
     "mode": "auto"
   }
   ```

7. **Appearance:**
   - Switch color (Manual): `#FF9800` (Orange)
   - Switch color (Auto): `#2196F3` (Blue)

8. คลิก **"Add"**

### ผลลัพธ์:
```
┌──────────────────┐
│  Mode Control    │
│                  │
│  AUTO  ●────○    │
│                  │
│  Currently: AUTO │
└──────────────────┘
```

---

## Widget 7: Input Status Indicators 🔘

**วัตถุประสงค์:** แสดงสถานะ Input switches

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Cards`

3. เลือก **Widget Type**: `Simple card`

4. **Add datasource:**
   - Device: `ESP32_ThaiTechZone_V2`
   - Keys: `switch1, switch2, switch3, iso_input1, iso_input2`

5. **Widget Settings:**
   - Title: `Input Status`
   - Show icon: ☑
   - Icon (ON): `check_circle` - Green
   - Icon (OFF): `cancel` - Gray

6. **Layout:**
   - Display as: `Badge list`

7. คลิก **"Add"**

### ผลลัพธ์:
```
┌─────────────────────────┐
│     Input Status        │
├─────────────────────────┤
│ ● Switch 1      ON      │
│ ○ Switch 2      OFF     │
│ ○ Switch 3      OFF     │
│ ● ISO Input 1   ON      │
│ ○ ISO Input 2   OFF     │
└─────────────────────────┘
```

---

## Widget 8: Weather Information ☁️

**วัตถุประสงค์:** แสดงข้อมูลสภาพอากาศ

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Cards`

3. เลือก **Widget Type**: `Entity count`

4. **Add datasource:**
   - Device: `ESP32_ThaiTechZone_V2`
   - Keys: `temperature, humidity, weather_city, weather_description`

5. **Widget Settings:**
   - Title: `Weather Information`
   - Background: `#2196F3` (Blue gradient)

6. **Custom HTML (Advanced):**
   ```html
   <div style="text-align: center; padding: 20px;">
     <h2 style="margin: 0;">{{temperature}}°C</h2>
     <p style="margin: 5px 0;">Humidity: {{humidity}}%</p>
     <p style="margin: 5px 0; font-size: 12px;">{{weather_city}}</p>
     <p style="margin: 5px 0; font-size: 12px;">{{weather_description}}</p>
   </div>
   ```

7. คลิก **"Add"**

### ผลลัพธ์:
```
┌───────────────────────┐
│  Weather Information  │
│                       │
│        28°C           │
│    Humidity: 75%      │
│                       │
│     Tha Sala          │
│  ท้องฟ้าแจ่มใส       │
└───────────────────────┘
```

---

## Widget 9: Air Quality Index Card 💨

**วัตถุประสงค์:** แสดงคุณภาพอากาศแบบการ์ด

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Cards`

3. เลือก **Widget Type**: `Value card`

4. **Add datasource:**
   - Device: `ESP32_ThaiTechZone_V2`
   - Key: `aqi`

5. **Widget Settings:**
   - Title: `Air Quality Index`
   - Show units: ☑
   - Units: `AQI`

6. **Thresholds:**
   ```
   1: Good      - Green  (#4CAF50)
   2: Fair      - Yellow (#FFEB3B)
   3: Moderate  - Orange (#FF9800)
   4: Poor      - Red    (#F44336)
   5: Very Poor - Purple (#9C27B0)
   ```

7. **Custom HTML:**
   ```html
   <div style="text-align: center;">
     <h1>{{aqi}}</h1>
     <p>{{air_quality}}</p>
     <p style="font-size: 12px;">PM2.5: {{pm2_5}} µg/m³</p>
   </div>
   ```

8. คลิก **"Add"**

### ผลลัพธ์:
```
┌─────────────────────┐
│ Air Quality Index   │
│                     │
│         2           │
│       Fair          │
│                     │
│  PM2.5: 35.2 µg/m³  │
└─────────────────────┘
    (Yellow card)
```

---

## Widget 10: WiFi Signal Gauge 📶

**วัตถุประสงค์:** แสดงความแรงสัญญาณ WiFi

### ขั้นตอน:

1. คลิก **"+"** → **"Create new widget"**

2. เลือก **Widget Bundle**: `Analogue gauges`

3. เลือก **Widget Type**: `Linear gauge`

4. **Add datasource:**
   - Device: `ESP32_ThaiTechZone_V2`
   - Key: `wifi_rssi`

5. **Widget Settings:**
   - Title: `WiFi Signal Strength`
   - Units: `dBm`

6. **Gauge Settings:**
   - Min value: `-100`
   - Max value: `-30`
   - Major ticks: `[-100, -80, -60, -40, -30]`

7. **Thresholds:**
   ```
   -100 to -80: Red    (Weak)
   -80 to -60:  Yellow (Fair)
   -60 to -40:  Green  (Good)
   -40 to -30:  Blue   (Excellent)
   ```

8. คลิก **"Add"**

### ผลลัพธ์:
```
  WiFi Signal Strength
┌────────────────────┐
│ -100  -80  -60  -40│-30
│ ████████████       │ -45 dBm
│ ████████████       │
└────────────────────┘
   Weak → Excellent
```

---

## 🎨 จัดเรียง Dashboard

### Layout แนะนำ:

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32 Control Panel                   │
├────────────────────┬────────────────────┬────────────────┤
│                    │                    │                │
│  System Status     │  Temperature &     │   PM2.5        │
│  (Entities Table)  │  Humidity Chart    │   Gauge        │
│                    │                    │                │
├────────────────────┴────────────────────┴────────────────┤
│                                                           │
│  Mode Control      Relay 1   Relay 2   Relay 3          │
│  [AUTO/MANUAL]     [  ON  ]  [ OFF ]   [ OFF ]          │
│                                                           │
├────────────────────┬────────────────────┬────────────────┤
│                    │                    │                │
│  AUX Controls      │  Input Status      │  Weather       │
│  [AUX1] [AUX2]     │  ● SW1    ○ SW2   │   28°C         │
│  [AUX3] [AUX4]     │  ○ SW3    ● ISO1  │   H: 75%       │
│                    │                    │                │
├────────────────────┴────────────────────┴────────────────┤
│                                                           │
│  Air Quality Index                WiFi Signal            │
│      AQI 2 (Fair)                      -45 dBm           │
│  PM2.5: 35.2 µg/m³                   [████████]          │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

### วิธีจัดเรียง:

1. คลิก **"Edit mode"** ✏️
2. **ลาก Widget** ไปวางตำแหน่งที่ต้องการ
3. **ปรับขนาด** โดยลากมุมของ Widget
4. **จัดกลุ่ม** Widget ที่เกี่ยวข้องกัน
5. คลิก **"Apply changes"** ✓
6. คลิก **"Save"** 💾

---

## 📦 Import Dashboard สำเร็จรูป (JSON)

### Dashboard Configuration JSON

บันทึกไฟล์นี้เป็น `esp32_dashboard.json`:

```json
{
  "title": "ESP32 Control Panel",
  "image": null,
  "mobileHide": false,
  "mobileOrder": null,
  "configuration": {
    "description": "IoT Control Dashboard for ThaiTechZone V2",
    "widgets": {
      "temperature_humidity_chart": {
        "type": "timeseries",
        "datasources": [
          {
            "type": "entity",
            "name": "Temperature",
            "entityAliasId": "device_alias",
            "dataKeys": [
              {
                "name": "temperature",
                "type": "timeseries",
                "label": "Temperature",
                "color": "#FF5722",
                "settings": {
                  "showLines": true,
                  "fillLines": false
                }
              }
            ]
          },
          {
            "type": "entity",
            "name": "Humidity",
            "entityAliasId": "device_alias",
            "dataKeys": [
              {
                "name": "humidity",
                "type": "timeseries",
                "label": "Humidity",
                "color": "#2196F3",
                "settings": {
                  "showLines": true,
                  "fillLines": false
                }
              }
            ]
          }
        ],
        "timewindow": {
          "realtime": {
            "timewindowMs": 3600000
          }
        }
      },
      "relay_control": {
        "type": "rpc",
        "datasources": [
          {
            "type": "entity",
            "entityAliasId": "device_alias"
          }
        ],
        "settings": {
          "requestMethod": "setRelay",
          "requestBody": {
            "relay": 1,
            "state": true
          }
        }
      }
    },
    "states": {
      "default": {
        "name": "Default",
        "root": true,
        "layouts": {
          "main": {
            "widgets": {},
            "gridSettings": {
              "columns": 24,
              "margin": 10,
              "outerMargin": true
            }
          }
        }
      }
    },
    "entityAliases": {
      "device_alias": {
        "id": "device_alias",
        "alias": "ESP32_ThaiTechZone_V2",
        "filter": {
          "type": "singleEntity",
          "resolveMultiple": false,
          "singleEntity": {
            "entityType": "DEVICE",
            "id": "YOUR_DEVICE_ID_HERE"
          }
        }
      }
    },
    "filters": {},
    "timewindow": {
      "displayValue": "",
      "hideInterval": false,
      "hideLastInterval": false,
      "hideQuickInterval": false,
      "hideAggregation": false,
      "hideAggInterval": false,
      "hideTimezone": false,
      "selectedTab": 0,
      "realtime": {
        "realtimeType": 1,
        "interval": 1000,
        "timewindowMs": 60000,
        "quickInterval": "CURRENT_DAY"
      },
      "aggregation": {
        "type": "AVG",
        "limit": 25000
      }
    },
    "settings": {
      "stateControllerId": "entity",
      "showTitle": true,
      "showDashboardsSelect": true,
      "showEntitiesSelect": true,
      "showDashboardTimewindow": true,
      "showDashboardExport": true,
      "toolbarAlwaysOpen": true
    }
  }
}
```

### วิธี Import:

1. ไปที่ **Dashboards**
2. คลิก **"+"** → **"Import dashboard"**
3. คลิก **"Upload file"** หรือวาง JSON
4. เลือกไฟล์ `esp32_dashboard.json`
5. แก้ไข `YOUR_DEVICE_ID_HERE` เป็น Device ID จริง
6. คลิก **"Import"**

---

## 📱 Mobile Dashboard

### ปรับ Dashboard ให้เหมาะกับมือถือ

1. เปิด Dashboard
2. คลิก **"Edit mode"** ✏️
3. คลิก **"Manage layouts"** (ไอคอนมือถือ 📱)
4. เลือก **"Mobile"**

### จัดเรียงสำหรับมือถือ:

```
┌──────────────────────┐
│  Mode Control        │
│  [AUTO / MANUAL]     │
├──────────────────────┤
│  Relay Controls      │
│  [RL1] [RL2] [RL3]   │
├──────────────────────┤
│  Temperature         │
│      28°C            │
├──────────────────────┤
│  PM2.5 Gauge         │
│    [=====> 35]       │
├──────────────────────┤
│  AUX Controls        │
│  [AUX1] [AUX2]       │
│  [AUX3] [AUX4]       │
└──────────────────────┘
```

### Tips สำหรับ Mobile:
- ใช้ Widget ขนาดเล็ก
- จัดเรียงแนวตั้ง (1-2 columns)
- ใช้ Switch/Button แทน Chart
- ลำดับความสำคัญ: Control > Status > Charts

---

## 🎯 Dashboard Templates พร้อมใช้

### Template 1: Monitoring Dashboard (ดูข้อมูล)

**จุดเด่น:** เน้นแสดงข้อมูลและกราฟ

**Widgets:**
- Temperature & Humidity Chart (ใหญ่)
- PM2.5 Chart
- System Status Table
- Weather Card
- WiFi Signal Gauge
- Input Status

**เหมาะสำหรับ:** ติดตามข้อมูล, ดูประวัติ

---

### Template 2: Control Dashboard (ควบคุม)

**จุดเด่น:** เน้นควบคุมอุปกรณ์

**Widgets:**
- Mode Switch (ใหญ่)
- Relay 1-3 Switches
- AUX 1-4 Buttons
- Current Status Card
- Quick Status Indicators

**เหมาะสำหรับ:** ควบคุมจากระยะไกล, Mobile

---

### Template 3: All-in-One Dashboard (รวม)

**จุดเด่น:** ครบทุกฟังก์ชัน

**Widgets:**
- ทั้งหมดข้างบน
- จัดเรียงเป็นหมวดหมู่
- ใช้ Tabs แยกหน้า

**เหมาะสำหรับ:** ผู้ใช้ขั้นสูง, Desktop

---

## 🔥 Advanced Features

### 1. Alarms Widget

เพิ่ม Widget แจ้งเตือน:

```
Type: Alarms table
Filter: 
  - PM2.5 > 35
  - Temperature > 35
  - WiFi RSSI < -80
```

### 2. Entity Hierarchy

จัดกลุ่มหลาย Devices:

```
Building
├─ Floor 1
│  ├─ ESP32_Room_101
│  └─ ESP32_Room_102
└─ Floor 2
   └─ ESP32_ThaiTechZone_V2
```

### 3. Custom Actions

สร้างปุ่ม Custom:

```json
{
  "method": "turnOffAll",
  "params": {}
}
```

Button: **🚨 EMERGENCY STOP**

---

## 📊 Dashboard Statistics

### ข้อมูลที่แนะนำให้เก็บ:

1. **Real-time** (ทุก 5 วินาที)
   - Temperature, Humidity
   - PM2.5, AQI
   - Relay/AUX states

2. **Historical** (เก็บ 1 เดือน)
   - Temperature avg/min/max
   - Air Quality trends
   - Uptime statistics

3. **Events** (เก็บทั้งหมด)
   - Mode changes
   - Relay ON/OFF events
   - Alarms triggered

---

## ✅ Checklist

### Dashboard สำเร็จแล้ว ถ้า:
- [ ] แสดงข้อมูล Real-time ได้
- [ ] ควบคุม Relay ได้ (Manual mode)
- [ ] ควบคุม AUX ได้
- [ ] สลับ Mode ได้
- [ ] แสดงกราฟ Temperature ย้อนหลัง
- [ ] แสดง PM2.5 Gauge
- [ ] แสดงสถานะ Input
- [ ] Responsive บนมือถือ
- [ ] มี Alarms (ถ้าต้องการ)

---

## 🎓 Tips & Tricks

### 1. ทำ Widget ให้สวย
- ใช้สีสอดคล้องกัน (Color palette)
- จัดกลุ่ม Widget ที่เกี่ยวข้อง
- ใช้ Icons เพิ่มความเข้าใจ
- Font size เหมาะสม

### 2. Performance
- อย่าใช้ Widget มากเกิน 15 ตัว/หน้า
- ใช้ Tabs แยกหน้า
- ตั้ง Timewindow ที่เหมาะสม

### 3. Sharing
- สร้าง Public Dashboard (ไม่มี Control)
- Share URL: `https://demo.thingsboard.io/dashboard/xxxxx`
- QR Code สำหรับ Mobile

---

## 📞 ต้องการความช่วยเหลือ?

### ปัญหาที่พบบ่อย:

**Q: Widget ไม่แสดงข้อมูล**
A: ตรวจสอบ Device เลือกถูกต้อง และ Telemetry key ถูกต้อง

**Q: RPC ไม่ทำงาน**
A: ตรวจสอบอยู่ใน Manual mode และ JSON format ถูกต้อง

**Q: Chart ไม่มีข้อมูล**
A: ตรวจสอบ Timewindow และรอข้อมูลสักครู่ (5 วินาที)

---

## 🏆 สรุป

คุณได้เรียนรู้:
✅ สร้าง Dashboard ใน ThingsBoard  
✅ เพิ่ม Widgets 10 ประเภท  
✅ จัดเรียง Layout  
✅ Import Dashboard สำเร็จรูป  
✅ ปรับแต่งสำหรับมือถือ  
✅ Advanced Features  

**Dashboard พร้อมใช้งาน!** 🎉

---

**เอกสารนี้จัดทำโดย:** ThaiTechZone Team  
**Version:** 3.0 (ThingsBoard Dashboard Guide)  
**อัปเดตล่าสุด:** 16 ตุลาคม 2025
