# 🚀 วิธีแก้ปัญหา Dashboard Import ไม่แสดง Widgets

## ❌ ปัญหา
Import JSON แล้วไม่มี widgets แสดง หรือ Dashboard ว่างเปล่า

---

## ✅ วิธีแก้ (3 วิธี)

### วิธีที่ 1: สร้างด้วยมือ (ง่ายที่สุด ⭐ แนะนำ)

#### ขั้นตอนสั้น ๆ (15 นาที):

1. **สร้าง Dashboard**
   ```
   ThingsBoard → Dashboards → + 
   → ชื่อ: "ESP32 Control"
   → Add
   ```

2. **เปิด Dashboard → Edit Mode** (ไอคอนดินสอ)

3. **สร้าง Entity Alias**
   ```
   กด Entity aliases (🔗)
   → + Add alias
   → Name: "MyESP32"
   → Filter: Single entity
   → Type: Device
   → Device: เลือก "ESP32_ThaiTechZone_V2"
   → Add → Save
   ```

4. **เพิ่ม Widget แรก (ทดสอบ)**
   ```
   + Add new widget
   → Cards → Simple card
   → Datasource: MyESP32
   → Add key: temperature
   → Add
   ```

5. **ทดสอบ**
   - เห็นข้อมูล temperature หรือไม่?
   - ✅ ถ้าเห็น = setup ถูกต้อง → เพิ่ม widgets อื่นต่อ
   - ❌ ถ้าไม่เห็น = มีปัญหา Entity alias หรือ telemetry

6. **เพิ่ม Widgets อื่น ๆ**
   - อ่านจาก `MANUAL_DASHBOARD_GUIDE.md`
   - ทำทีละ widget
   - Save เป็นระยะ

---

### วิธีที่ 2: ใช้ ThingsBoard Demo Dashboard

ThingsBoard มี Dashboard ตัวอย่าง:

1. **ไปที่ Dashboards**
   ```
   เมนู Dashboards
   → ดู Dashboard ตัวอย่างที่มีอยู่แล้ว
   ```

2. **Clone Dashboard**
   ```
   เลือก Dashboard ที่ชอบ
   → ... (menu) → Make copy
   → เปลี่ยนชื่อ
   ```

3. **แก้ไข Entity Alias**
   ```
   Edit → Entity aliases
   → แก้ Device name เป็น "ESP32_ThaiTechZone_V2"
   → Save
   ```

4. **ลบ/เพิ่ม Widgets**
   ```
   ลบ widgets ที่ไม่ต้องการ
   เพิ่ม widgets ที่ต้องการ
   ```

---

### วิธีที่ 3: ใช้ Widget Library

สร้างทีละ widget จาก library:

#### Widget 1: ข้อมูลระบบ (เริ่มที่นี่)

```
+ Add widget
→ Current bundle: Cards
→ Widget type: Entities table

Datasource:
- Entity alias: MyESP32
- Keys to display:
  * uptime (label: "Uptime", units: "sec")
  * mode (label: "Mode")
  * wifi_rssi (label: "WiFi", units: "dBm")
  * free_heap (label: "Memory", units: "bytes")

Appearance:
- Title: "System Info"
- Size: 12x5

→ Add
```

**ทดสอบ:** ต้องเห็นข้อมูลทั้ง 4 ค่า

---

#### Widget 2: กราฟอุณหภูมิ

```
+ Add widget
→ Current bundle: Charts
→ Widget type: Timeseries - Line chart

Datasources:
- Entity alias: MyESP32
- Keys:
  * temperature (color: Red, units: "°C")
  * humidity (color: Blue, units: "%")

Time window:
- Realtime - Last 1 hour
- Interval: 1 second
- Aggregation: None

Appearance:
- Title: "Temperature & Humidity"
- Show legend: Yes
- Size: 12x8

→ Add
```

**ทดสอบ:** ต้องเห็นเส้นกราฟ 2 เส้น (แดง/น้ำเงิน)

---

#### Widget 3: ควบคุมโหมด

```
+ Add widget
→ Current bundle: Control widgets
→ Widget type: Switch control

Target device:
- Entity alias: MyESP32

Initial state:
- Retrieve value method: getValue
- Value attribute: mode
- Parse value function:
  return value === 'manual';

Switch state:
- RPC set value method: setMode
- On switch state function (ON):
  return { method: 'setMode', params: { mode: 'manual' } };
- On switch state function (OFF):
  return { method: 'setMode', params: { mode: 'auto' } };

Appearance:
- Title: "Mode"
- Label ON: "MANUAL"
- Label OFF: "AUTO"
- Color ON: Orange
- Color OFF: Blue
- Size: 6x4

→ Add
```

**ทดสอบ:** กดสวิตช์ → ดู Serial Monitor ว่าได้รับ RPC หรือไม่

---

#### Widget 4: ควบคุม Relay 1

```
+ Add widget
→ Current bundle: Control widgets
→ Widget type: Switch control

Target device:
- Entity alias: MyESP32

Switch state:
- RPC set value method: setRelay
- On switch state function (ON):
  return { method: 'setRelay', params: { relay: 1, state: true } };
- On switch state function (OFF):
  return { method: 'setRelay', params: { relay: 1, state: false } };

Appearance:
- Title: "Relay 1"
- Label ON: "ON"
- Label OFF: "OFF"
- Color ON: Green
- Color OFF: Grey
- Size: 6x4

→ Add
```

**ทดสอบ:** 
1. เปลี่ยนเป็น Manual mode ก่อน
2. กดสวิตช์ Relay 1
3. ดู Serial Monitor → "Relay 1 set to ON"

---

#### Widget 5: ปุ่ม Emergency Stop

```
+ Add widget
→ Current bundle: Control widgets
→ Widget type: Basic button

Target device:
- Entity alias: MyESP32

Button action:
- RPC request method: turnOffAll
- RPC request params: {}
- Request confirmation: Yes
- Confirmation message: "Turn off all outputs?"

Appearance:
- Title: "Emergency"
- Button label: "🚨 STOP ALL"
- Button color: Red
- Icon: power_settings_new
- Size: 6x4

→ Add
```

**ทดสอบ:** กดปุ่ม → ยืนยัน → Relay ทุกตัวต้อง OFF

---

## 🎨 Layout แนะนำ (24 columns)

```
Row 0:
┌─────────────────────┬──────────────┐
│   System Info       │  Temp Chart  │
│   (12x5)            │  (12x8)      │
└─────────────────────┴──────────────┘

Row 1:
┌──────┬──────┬──────┬──────┬────────┐
│ Mode │Relay1│Relay2│Relay3│ Stop   │
│ (6x4)│(6x4) │(6x4) │(6x4) │ (6x4)  │
└──────┴──────┴──────┴──────┴────────┘
```

---

## 🐛 Troubleshooting

### ปัญหา: Widget ไม่แสดงข้อมูล

**เช็ค 1: Entity Alias**
```
Edit Dashboard → Entity aliases
→ กด "Filter preview" 
→ ต้องเห็น Device "ESP32_ThaiTechZone_V2"
```

**เช็ค 2: Latest Telemetry**
```
Devices → ESP32_ThaiTechZone_V2 → Latest telemetry
→ ต้องมีข้อมูล temperature, humidity, mode, etc.
```

**เช็ค 3: Time Window**
```
Widget settings → Advanced → Time window
→ เปลี่ยนเป็น "Realtime - Last 1 minute"
```

---

### ปัญหา: RPC ไม่ทำงาน

**เช็ค 1: MQTT Connection**
```
Serial Monitor → ดู "MQTT: ✅ CONNECTED"
```

**เช็ค 2: Manual Mode**
```
Relay control ต้องอยู่ใน Manual mode
กดสวิตช์ Mode → MANUAL ก่อน
```

**เช็ค 3: RPC Debug**
```
Device → RPC → Send test RPC
Method: setMode
Params: {"mode":"manual"}
→ กด Send
→ ดู Serial Monitor
```

---

### ปัญหา: Chart ไม่แสดงกราฟ

**สาเหตุ:** ข้อมูลยังไม่พอ

**วิธีแก้:**
1. รอ 2-3 นาที (ให้ส่ง telemetry 12-18 ครั้ง)
2. ตรวจสอบ Time window = "Last 1 hour"
3. ดู Latest telemetry ว่ามี temperature/humidity

---

## 📋 Checklist การสร้าง Dashboard

### Phase 1: Setup (5 นาที)
- [ ] สร้าง Dashboard ใหม่
- [ ] สร้าง Entity Alias "MyESP32"
- [ ] ทดสอบด้วย Simple card (1 widget)

### Phase 2: Monitoring (10 นาที)
- [ ] System Info card (uptime, mode, wifi, memory)
- [ ] Temperature & Humidity chart
- [ ] PM2.5 gauge (optional)
- [ ] Input status table

### Phase 3: Control (10 นาที)
- [ ] Mode control switch
- [ ] Relay 1-3 switches
- [ ] AUX buttons (optional)
- [ ] Emergency stop button

### Phase 4: Testing (5 นาที)
- [ ] ทดสอบ monitoring widgets
- [ ] ทดสอบ control widgets
- [ ] ทดสอบ Emergency stop
- [ ] Save dashboard

**เวลารวม:** ประมาณ 30 นาที

---

## 💡 Tips

### Tip 1: Clone Widgets
สร้าง Relay 1 แล้ว → Clone เป็น Relay 2, 3
```
Widget menu (⋮) → Make copy
→ แก้ Title และ params: { relay: 2, ... }
```

### Tip 2: Widget Templates
บันทึก widget ที่ชอบเป็น template:
```
Widget menu (⋮) → Export widget
→ บันทึกไฟล์ .json
→ Import ได้ใน dashboard อื่น
```

### Tip 3: Dashboard States
สร้าง states หลายหน้า:
```
Edit → Dashboard states → Add state
→ เช่น: "Monitoring" และ "Control"
→ สลับด้วยปุ่ม
```

### Tip 4: Keyboard Shortcuts
```
Ctrl + S = Save dashboard
Ctrl + Z = Undo
Ctrl + Y = Redo
Delete = ลบ widget ที่เลือก
Arrow keys = เลื่อน widget
```

---

## 🎯 Quick Dashboard (10 นาที)

ถ้าต้องการเร็ว สร้างแค่ 3 widgets:

1. **System Table** (uptime, mode, wifi, memory)
2. **Temperature Chart** (temp + humidity)
3. **Mode Switch** (auto/manual control)

→ ใช้งานได้แล้ว! เพิ่ม widgets อื่นทีหลัง

---

## 📚 เอกสารเพิ่มเติม

### ThingsBoard Official Docs:
- Dashboard: https://thingsboard.io/docs/user-guide/ui/dashboards/
- Widgets: https://thingsboard.io/docs/user-guide/ui/widget-library/
- RPC: https://thingsboard.io/docs/user-guide/rpc/

### Video Tutorials:
- "ThingsBoard Dashboard Tutorial" (YouTube)
- "Creating Custom Widgets" (ThingsBoard Academy)

---

## ✅ สรุป

**JSON Import ไม่ทำงาน** เพราะ:
- JSON structure ซับซ้อน
- Version ของ ThingsBoard ต่างกัน
- Widget configuration ไม่ครบ

**วิธีแก้ที่ดีที่สุด:**
- ✅ สร้างด้วยมือ (30 นาที)
- ✅ เริ่มจาก Simple card
- ✅ เพิ่ม widgets ทีละตัว
- ✅ ทดสอบทุก widget
- ✅ Save บ่อย ๆ

**ผลลัพธ์:**
- Dashboard ที่ใช้งานได้จริง 100%
- เข้าใจวิธีการทำงาน
- แก้ไข/ปรับแต่งได้เอง

---

**เริ่มสร้างได้เลยครับ! มีปัญหาตรงไหนถามได้เสมอ** 🚀
