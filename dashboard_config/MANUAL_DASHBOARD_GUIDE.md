# 🎨 คู่มือสร้าง Dashboard แบบ Manual

## ⚠️ หมายเหตุสำคัญ

ไฟล์ JSON ที่ export จาก ThingsBoard มีโครงสร้างซับซ้อนมาก แนะนำให้**สร้าง Dashboard ด้วยมือ**แทน จะได้ผลดีกว่าและปรับแต่งง่ายกว่า

---

## 📋 ขั้นตอนสร้าง Dashboard

### ขั้นตอนที่ 1: สร้าง Dashboard ใหม่

1. **Login เข้า ThingsBoard**
   ```
   https://demo.thingsboard.io
   ```

2. **สร้าง Dashboard**
   ```
   เมนูซ้าย → Dashboards
   → กดปุ่ม + มุมขวาล่าง
   → "Create new dashboard"
   → ชื่อ: "ESP32 ThaiTechZone V2 Control Panel"
   → Add
   ```

3. **เปิด Edit Mode**
   ```
   เปิด Dashboard ที่สร้าง
   → กดไอคอนดินสอ (Edit) มุมขวาล่าง
   ```

---

### ขั้นตอนที่ 2: ตั้งค่า Entity Alias

1. **เปิด Entity Aliases**
   ```
   กดปุ่ม "Entity aliases" (ไอคอน 🔗)
   ```

2. **สร้าง Alias**
   ```
   → Add alias
   → Alias name: "ESP32_Device"
   → Filter type: "Single entity"
   → Type: "Device"
   → Device: เลือก "ESP32_ThaiTechZone_V2"
   → Add
   → Save
   ```

---

### ขั้นตอนที่ 3: เพิ่ม Widgets

## Widget 1: 📊 System Status Card

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Cards → Entities table
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add keys:
   - uptime (Uptime, sec)
   - mode (Mode)  
   - wifi_rssi (WiFi Signal, dBm)
   - free_heap (Free Memory, bytes)
   
   → Add
   ```

3. **Appearance**
   ```
   Title: "System Status"
   ✓ Show title
   ✓ Drop shadow
   ✓ Enable fullscreen
   
   Size: 8 columns x 4 rows
   Position: Row 0, Col 0
   
   → Add
   ```

---

## Widget 2: 📈 Temperature & Humidity Chart

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Charts → Timeseries - Line
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add keys:
   - temperature
     - Label: "Temperature"
     - Units: "°C"
     - Color: #FF5722 (Red)
     - Decimals: 1
   
   - humidity
     - Label: "Humidity"
     - Units: "%"
     - Color: #2196F3 (Blue)
     - Decimals: 1
   
   → Add
   ```

3. **Appearance**
   ```
   Title: "Temperature & Humidity"
   Time window: Last 1 hour
   ✓ Show legend
   
   Y-axis: Min 0, Max 100
   
   Size: 10 columns x 6 rows
   Position: Row 0, Col 8
   
   → Add
   ```

---

## Widget 3: 🌡️ PM2.5 Gauge

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Analogue gauges → Radial gauge
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add key:
   - pm2_5
     - Label: "PM2.5"
     - Units: "µg/m³"
     - Decimals: 1
   
   → Add
   ```

3. **Appearance**
   ```
   Title: "PM2.5 Air Quality"
   
   Settings:
   - Min value: 0
   - Max value: 100
   - Start angle: 45
   - Ticks angle: 270
   
   Highlights:
   - 0-12: Green (#4CAF50)
   - 12-35: Yellow (#FFEB3B)
   - 35-55: Orange (#FF9800)
   - 55-100: Red (#F44336)
   
   Size: 6 columns x 6 rows
   Position: Row 0, Col 18
   
   → Add
   ```

---

## Widget 4: 🔄 Mode Control Switch

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Control widgets → Switch control
   → Next
   ```

2. **Target device**
   ```
   Entity alias: ESP32_Device
   ```

3. **RPC Settings**
   ```
   RPC set value method: setMode
   
   Switch settings:
   - Label ON: "MANUAL"
   - Label OFF: "AUTO"
   - Initial state: OFF
   ```

4. **Advanced - RPC Command**
   ```
   On value function:
   return { method: 'setMode', params: { mode: 'manual' } };
   
   Off value function:
   return { method: 'setMode', params: { mode: 'auto' } };
   ```

5. **Appearance**
   ```
   Title: "Mode Control"
   
   Size: 6 columns x 3 rows
   Position: Row 4, Col 0
   
   → Add
   ```

---

## Widget 5-7: ⚡ Relay Controls (สร้าง 3 widgets)

### Relay 1:

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Control widgets → Switch control
   → Next
   ```

2. **Target device**
   ```
   Entity alias: ESP32_Device
   ```

3. **RPC Settings**
   ```
   RPC set value method: setRelay
   
   Switch settings:
   - Label ON: "ON"
   - Label OFF: "OFF"
   - Initial state: OFF
   ```

4. **Advanced - RPC Command**
   ```
   On value function:
   return { method: 'setRelay', params: { relay: 1, state: true } };
   
   Off value function:
   return { method: 'setRelay', params: { relay: 1, state: false } };
   ```

5. **Appearance**
   ```
   Title: "Relay 1"
   
   Size: 6 columns x 3 rows
   Position: Row 6, Col 0
   
   → Add
   ```

**ทำซ้ำสำหรับ Relay 2 และ Relay 3:**
- Relay 2: Position Row 6, Col 6 (เปลี่ยน params: { relay: 2, ... })
- Relay 3: Position Row 6, Col 12 (เปลี่ยน params: { relay: 3, ... })

---

## Widget 8: 🔌 AUX Controls

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Control widgets → Basic button
   → Next
   ```

2. **Target device**
   ```
   Entity alias: ESP32_Device
   ```

3. **Button 1 - AUX 1 ON**
   ```
   Label: "AUX 1 ON"
   Icon: flash_on
   Color: Green
   RPC method: setAux
   RPC params: {"aux": 1, "state": true}
   ```

4. **เพิ่มปุ่มเพิ่มเติม** (8 ปุ่มทั้งหมด)
   ```
   - AUX 1 OFF: {"aux": 1, "state": false}
   - AUX 2 ON: {"aux": 2, "state": true}
   - AUX 2 OFF: {"aux": 2, "state": false}
   - AUX 3 ON: {"aux": 3, "state": true}
   - AUX 3 OFF: {"aux": 3, "state": false}
   - AUX 4 ON: {"aux": 4, "state": true}
   - AUX 4 OFF: {"aux": 4, "state": false}
   ```

5. **Appearance**
   ```
   Title: "AUX Controls"
   Layout: 4 rows x 2 columns
   
   Size: 8 columns x 6 rows
   Position: Row 9, Col 0
   
   → Add
   ```

---

## Widget 9: 📥 Input Status

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Cards → Entities table
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add keys:
   - switch1 (Switch 1)
   - switch2 (Switch 2)
   - switch3 (Switch 3)
   - iso_input1 (ISO Input 1)
   - iso_input2 (ISO Input 2)
   
   → Add
   ```

3. **Appearance**
   ```
   Title: "Input Status"
   
   Display settings:
   ✓ Show boolean icon
   - Icon true: check_circle
   - Icon false: cancel
   - Color true: Green
   - Color false: Grey
   
   Size: 6 columns x 6 rows
   Position: Row 9, Col 8
   
   → Add
   ```

---

## Widget 10: 🛑 Emergency Stop

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Control widgets → Basic button
   → Next
   ```

2. **Target device**
   ```
   Entity alias: ESP32_Device
   ```

3. **Button settings**
   ```
   Label: "🚨 EMERGENCY STOP"
   Icon: power_settings_new
   Color: Red (#F44336)
   
   RPC method: turnOffAll
   RPC params: {}
   
   ✓ Confirm required
   Confirm message: "Turn off all outputs?"
   ```

4. **Appearance**
   ```
   Title: "Emergency Control"
   
   Size: 6 columns x 3 rows
   Position: Row 4, Col 6
   
   → Add
   ```

---

## Widget 11: 🌤️ Weather Card (HTML)

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Cards → HTML card
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add keys:
   - temperature
   - humidity
   
   → Add
   ```

3. **HTML Template**
   ```html
   <div style="text-align:center;padding:20px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border-radius:8px;height:100%;">
     <div style="font-size:48px;margin:10px 0;">🌡️</div>
     <h1 style="margin:0;font-size:42px;">${temperature}°C</h1>
     <div style="font-size:32px;margin:15px 0;">💧</div>
     <p style="margin:10px 0;font-size:20px;">Humidity: ${humidity}%</p>
     <p style="margin:10px 0;font-size:11px;opacity:0.7;">Weather updates every 10 min</p>
   </div>
   ```

4. **Appearance**
   ```
   Title: "Weather"
   ✓ Show title
   
   Size: 6 columns x 6 rows
   Position: Row 9, Col 14
   
   → Add
   ```

---

## Widget 12: 💨 AQI Card (HTML)

1. **เพิ่ม Widget**
   ```
   กด + Add new widget
   → Cards → HTML card
   → Next
   ```

2. **Datasource**
   ```
   Entity alias: ESP32_Device
   
   Add keys:
   - aqi
   - pm2_5
   
   → Add
   ```

3. **HTML Template**
   ```html
   <div style="text-align:center;padding:20px;height:100%;">
     <div style="font-size:48px;margin:10px 0;">💨</div>
     <h1 style="margin:0;font-size:52px;color:#4CAF50;">${aqi}</h1>
     <p style="margin:10px 0;font-size:16px;font-weight:bold;">Air Quality Index</p>
     <div style="margin-top:15px;padding:12px;background:#f5f5f5;border-radius:5px;">
       <p style="margin:5px 0;font-size:14px;color:#666;">PM2.5: ${pm2_5} µg/m³</p>
     </div>
     <p style="margin:10px 0;font-size:10px;opacity:0.6;">1=Good, 2=Fair, 3=Moderate, 4=Poor, 5=VeryPoor</p>
   </div>
   ```

4. **Appearance**
   ```
   Title: "Air Quality"
   ✓ Show title
   
   Size: 6 columns x 6 rows
   Position: Row 9, Col 20
   
   → Add
   ```

---

## ขั้นตอนที่ 4: Save Dashboard

```
กดปุ่ม "Apply changes" (✓)
→ กดปุ่ม "Save" ไอคอน💾
```

---

## 🎨 Dashboard Layout Preview

```
┌────────────┬─────────────────────┬──────────┐
│  System    │  Temp & Humidity    │  PM2.5   │
│  Status    │      Chart          │  Gauge   │
│  (8x4)     │      (10x6)         │  (6x6)   │
│            │                     │          │
├────────────┼─────────────────────┴──────────┤
│  Mode      │  Emergency Stop                │
│  Control   │  (6x3)                         │
│  (6x3)     │                                │
├────────────┴──────────┬────────┬────────────┤
│  Relay 1   │ Relay 2  │ Relay 3│           │
│   (6x3)    │  (6x3)   │ (6x3)  │           │
├────────────┴──────────┴────────┴───────────┤
│  AUX Controls  │ Input  │Weather │  AQI    │
│    (8x6)       │ Status │ (6x6)  │ (6x6)   │
│                │ (6x6)  │        │         │
└────────────────┴────────┴────────┴─────────┘
```

---

## ✅ Testing Checklist

หลัง Save Dashboard แล้ว ให้ทดสอบ:

### Monitoring Widgets:
- [ ] System Status แสดง uptime, mode, wifi_rssi, free_heap
- [ ] Temperature & Humidity Chart แสดงกราฟ
- [ ] PM2.5 Gauge แสดงค่าและสี
- [ ] Input Status แสดง switch1-3, iso_input1-2
- [ ] Weather Card แสดง temperature & humidity
- [ ] AQI Card แสดง aqi & pm2_5

### Control Widgets:
- [ ] Mode Control เปลี่ยน Auto/Manual ได้
- [ ] Relay 1-3 ควบคุมได้ใน Manual mode
- [ ] AUX 1-4 ควบคุมได้ใน Manual mode
- [ ] Emergency Stop ปิดทุกอย่างได้

### Serial Monitor:
- [ ] แสดง "RPC received" เมื่อกดปุ่ม
- [ ] แสดง "Relay X set to ON/OFF"
- [ ] แสดง "Switched to MANUAL/AUTO mode"

---

## 🐛 Troubleshooting

### ปัญหา 1: Widget ไม่แสดงข้อมูล

**สาเหตุ:** Entity Alias ไม่ถูกต้อง

**วิธีแก้:**
```
Edit Dashboard → Entity aliases
→ ตรวจสอบ Device name = "ESP32_ThaiTechZone_V2"
→ ลอง Query แล้วต้องเจอ Device
```

### ปัญหา 2: RPC ไม่ทำงาน

**สาเหตุ:** ไม่ได้อยู่ใน Manual Mode หรือ MQTT disconnect

**วิธีแก้:**
```
1. Switch to Manual mode ก่อน
2. ดู Serial Monitor ว่า MQTT connected
3. ทดสอบด้วย RPC Debug tab
```

### ปัญหา 3: Chart ไม่แสดงกราฟ

**สาเหตุ:** Time window หรือข้อมูลยังไม่พอ

**วิธีแก้:**
```
1. รอให้ข้อมูลส่งมาอย่างน้อย 2-3 ครั้ง
2. ตรวจสอบ Time window = Last 1 hour
3. ดู Latest telemetry ว่ามีข้อมูลหรือไม่
```

---

## 💡 Tips & Tricks

### Tip 1: ใช้ Dashboard Templates
ThingsBoard มี Templates สำเร็จรูป สามารถ Import แล้วแก้ไข:
```
Dashboards → + → Import dashboard → Browse templates
```

### Tip 2: Clone Widgets
แทนที่จะสร้างใหม่ทุกครั้ง:
```
Widget → ... (menu) → Clone widget
→ แก้ไข title และ parameters
```

### Tip 3: Dashboard Groups
จัดกลุ่ม Widgets:
```
Edit → Select multiple widgets (Ctrl+Click)
→ Right-click → Group
```

### Tip 4: Export Dashboard
สำรอง Dashboard:
```
Dashboard → ... (menu) → Export dashboard
→ เก็บไฟล์ JSON ไว้
```

### Tip 5: Share Dashboard
แชร์ให้คนอื่น:
```
Dashboard → Share → Generate public link
หรือ Add customers/users
```

---

## 📱 Mobile Dashboard (Optional)

### ปรับ Layout สำหรับมือถือ:

1. **Create Mobile Layout**
   ```
   Edit Dashboard → Mobile layout
   → + Add mobile layout
   ```

2. **จัดเรียง Widgets ใหม่**
   ```
   ลาก Widget ให้เรียงตามลำดับ:
   1. System Status (full width)
   2. Mode Control (full width)
   3. Relay Controls (full width)
   4. Temperature Chart (full width)
   5. AQI Card (half width)
   6. Weather Card (half width)
   ```

3. **Save Layout**
   ```
   Apply changes → Save
   ```

---

## 🎓 Video Tutorial (แนะนำ)

หากต้องการเรียนรู้เพิ่มเติม ดู Official ThingsBoard Tutorial:
- https://thingsboard.io/docs/user-guide/ui/dashboards/
- YouTube: "ThingsBoard Dashboard Tutorial"

---

## ✅ สรุป

**ขั้นตอนหลัก:**
1. ✅ สร้าง Dashboard ใหม่
2. ✅ ตั้งค่า Entity Alias
3. ✅ เพิ่ม Widgets ทีละตัว (12 widgets)
4. ✅ Save และทดสอบ

**ใช้เวลา:** ประมาณ 30-45 นาที

**ผลลัพธ์:** Dashboard ที่ใช้งานได้จริง พร้อมควบคุม ESP32 แบบ Real-time!

---

**มีคำถามหรือต้องการความช่วยเหลือ สามารถถามได้เลยครับ!** 🚀
