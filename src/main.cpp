#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ╔═══════════════════════════════════════════════════════════╗
// ║  ThaiTechZone V2.0 - Hardware IO Tester                  ║
// ║  Simple Test Loop - No WiFi, No Sensors, No Web          ║
// ║  Purpose: Test all IOs to verify hardware functionality  ║
// ╚═══════════════════════════════════════════════════════════╝

// ===== OLED Display Configuration =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== Pin Definitions =====
// Input Toggle Switches 
#define SW1_PIN 34
#define SW2_PIN 35
#define SW3_PIN 32

// Isolated Inputs
#define ISOIN1_PIN 33
#define ISOIN2_PIN 27

// Output Pins
#define LED_PIN 2
#define RL1_PIN 17  // RELAY1
#define RL2_PIN 16  // RELAY2
#define RL3_PIN 4   // RELAY3

// ===== Test Cycle Configuration =====
const unsigned long RELAY_ON_TIME = 4000;   // 4 seconds ON (for startup only)
const unsigned long RELAY_OFF_TIME = 2000;  // 2 seconds OFF (for startup only)

// ===== Global Variables =====
// Startup sequence control
bool startup_completed = false;
unsigned long startup_timer = 0;
bool startup_all_on = true;  // true=ALL ON phase, false=ALL OFF phase

// Switch debounce variables
unsigned long lastSwitchMillis[3] = {0, 0, 0};
bool lastSwitchState[3] = {HIGH, HIGH, HIGH};  // INPUT_PULLUP: HIGH = released
bool switchPressed[3] = {false, false, false};
const unsigned long DEBOUNCE_MS = 50;

// Manual relay states (controlled by switches)
bool manual_relay_states[3] = {true, true, true};  // Start with all ON

// OLED Display availability
bool oled_available = false;

// ===== Function Declarations =====
void testOLED();
void updateOLED();
void setupPins();
void handleIsolatedInputs();
void handleSwitches();
void runStartupSequence();
void printStatus();

// ===== Setup Function =====
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔═══════════════════════════════════════════════╗");
  Serial.println("║   ThaiTechZone V2.0 - Hardware Test Mode     ║");
  Serial.println("║   IO Tester - Simple Loop Test                ║");
  Serial.println("╚═══════════════════════════════════════════════╝\n");
  
  // Setup all pins
  setupPins();
  
  Serial.println("✓ Pin initialization complete");
  
  // Test OLED Display
  testOLED();
  
  Serial.println("✓ Starting startup sequence...\n");
  Serial.println("Startup Sequence:");
  Serial.println("  - Phase 1: ALL RELAYS ON for 4 seconds");
  Serial.println("  - Phase 2: ALL RELAYS OFF for 2 seconds");
  Serial.println("  - Then keep all relays ON (SW1-3 to toggle)\n");
  
  // Start startup sequence
  startup_timer = millis();
  
  // Turn ALL relays ON for startup
  digitalWrite(RL1_PIN, LOW);  // Active Low - ON
  digitalWrite(RL2_PIN, LOW);  // Active Low - ON
  digitalWrite(RL3_PIN, LOW);  // Active Low - ON
  
  Serial.println("╔═══════════════════════════════════════════════╗");
  Serial.println("║  STARTUP: ALL RELAYS ON (4 seconds)          ║");
  Serial.println("╚═══════════════════════════════════════════════╝\n");
}

// ===== Main Loop =====
void loop() {
  static unsigned long last_status_print = 0;
  static unsigned long last_oled_update = 0;
  
  // 1. Handle Isolated Inputs (ISOIN1, ISOIN2) → LED
  handleIsolatedInputs();
  
  // 2. Handle Switch Inputs (SW1-3) → Manual Relay Control
  handleSwitches();
  
  // 3. Run startup sequence only
  if (!startup_completed) {
    runStartupSequence();
  }
  
  // 4. Print status every 2 seconds
  if (millis() - last_status_print >= 2000) {
    last_status_print = millis();
    printStatus();
  }
  
  // 5. Update OLED every 1 second (if available)
  if (oled_available && millis() - last_oled_update >= 1000) {
    last_oled_update = millis();
    updateOLED();
  }
  
  delay(10);  // Small delay for stability
}

// ===== Setup Pins Function =====
void setupPins() {
  // Configure output pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RL1_PIN, OUTPUT);
  pinMode(RL2_PIN, OUTPUT);
  pinMode(RL3_PIN, OUTPUT);
  
  // Configure input pins with pull-up
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(ISOIN1_PIN, INPUT_PULLUP);
  pinMode(ISOIN2_PIN, INPUT_PULLUP);
  
  // Initialize all outputs to OFF
  // Note: Relays are Active Low (HIGH = OFF, LOW = ON)
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RL1_PIN, HIGH);   // Active Low - OFF
  digitalWrite(RL2_PIN, HIGH);   // Active Low - OFF
  digitalWrite(RL3_PIN, HIGH);   // Active Low - OFF
}

// ===== Handle Isolated Inputs =====
void handleIsolatedInputs() {
  // Read isolated inputs (Active Low)
  bool iso1_active = (digitalRead(ISOIN1_PIN) == LOW);
  bool iso2_active = (digitalRead(ISOIN2_PIN) == LOW);
  
  // Turn LED ON if either input is LOW (active)
  if (iso1_active || iso2_active) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// ===== Handle Switch Inputs =====
void handleSwitches() {
  const int swPins[3] = {SW1_PIN, SW2_PIN, SW3_PIN};
  const int rlPins[3] = {RL1_PIN, RL2_PIN, RL3_PIN};
  unsigned long now = millis();
  
  for (int i = 0; i < 3; i++) {
    bool reading = digitalRead(swPins[i]);  // HIGH = released, LOW = pressed
    
    // Detect state change
    if (reading != lastSwitchState[i]) {
      lastSwitchMillis[i] = now;
      lastSwitchState[i] = reading;
    }
    
    // Check if debounced
    if (now - lastSwitchMillis[i] >= DEBOUNCE_MS) {
      // Detect press (falling edge)
      if (reading == LOW && !switchPressed[i]) {
        switchPressed[i] = true;
        
        // Toggle relay state
        manual_relay_states[i] = !manual_relay_states[i];
        
        // Set relay (Active Low: LOW = ON, HIGH = OFF)
        digitalWrite(rlPins[i], manual_relay_states[i] ? LOW : HIGH);
        
        // Print action
        Serial.print("╔═══ SW");
        Serial.print(i + 1);
        Serial.println(" PRESSED ═══════════════════════════════╗");
        Serial.print("║ RELAY");
        Serial.print(i + 1);
        Serial.print(" → ");
        Serial.println(manual_relay_states[i] ? "ON                                  ║" : "OFF                                 ║");
        Serial.println("╚═══════════════════════════════════════════════╝\n");
      }
      
      // Detect release (rising edge)
      if (reading == HIGH && switchPressed[i]) {
        switchPressed[i] = false;
      }
    }
  }
}

// ===== Run Startup Sequence =====
void runStartupSequence() {
  unsigned long elapsed = millis() - startup_timer;
  
  if (startup_all_on) {
    // Phase 1: ALL RELAYS ON for 4 seconds
    if (elapsed >= RELAY_ON_TIME) {
      // Turn ALL relays OFF
      digitalWrite(RL1_PIN, HIGH);  // Active Low - OFF
      digitalWrite(RL2_PIN, HIGH);  // Active Low - OFF
      digitalWrite(RL3_PIN, HIGH);  // Active Low - OFF
      
      Serial.println("╔═══════════════════════════════════════════════╗");
      Serial.println("║  STARTUP: ALL RELAYS OFF (2 seconds)         ║");
      Serial.println("╚═══════════════════════════════════════════════╝\n");
      
      startup_all_on = false;
      startup_timer = millis();
    }
  } else {
    // Phase 2: ALL RELAYS OFF for 2 seconds
    if (elapsed >= RELAY_OFF_TIME) {
      Serial.println("╔═══════════════════════════════════════════════╗");
      Serial.println("║  STARTUP COMPLETE - Relays Ready             ║");
      Serial.println("╚═══════════════════════════════════════════════╝\n");
      Serial.println("Operating Mode:");
      Serial.println("  - RELAY1-3: All ON (constant)");
      Serial.println("  - SW1-3: Toggle RELAY1-3 ON/OFF");
      Serial.println("  - ISOIN1/2: Turn LED ON when LOW");
      Serial.println("  - Status updates every 2 seconds\n");
      
      startup_completed = true;
      
      // Turn ALL relays back ON after startup
      digitalWrite(RL1_PIN, LOW);   // Active Low - ON
      digitalWrite(RL2_PIN, LOW);   // Active Low - ON
      digitalWrite(RL3_PIN, LOW);   // Active Low - ON
      
      Serial.println("All relays turned ON. Use SW1-3 to toggle.\n");
    }
  }
}

// ===== Print Status =====
void printStatus() {
  Serial.println("\n╔═══════════════════════════════════════════════╗");
  Serial.println("║              SYSTEM STATUS                    ║");
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // Relay states
  Serial.print("║ RELAY1: ");
  Serial.print(!digitalRead(RL1_PIN) ? "ON " : "OFF");  // Active Low - invert
  Serial.print(" | RELAY2: ");
  Serial.print(!digitalRead(RL2_PIN) ? "ON " : "OFF");
  Serial.print(" | RELAY3: ");
  Serial.print(!digitalRead(RL3_PIN) ? "ON " : "OFF");
  Serial.println("  ║");
  
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // Switch states
  Serial.print("║ SW1: ");
  Serial.print(digitalRead(SW1_PIN) == LOW ? "PRESSED " : "RELEASED");
  Serial.print(" | SW2: ");
  Serial.print(digitalRead(SW2_PIN) == LOW ? "PRESSED " : "RELEASED");
  Serial.println("  ║");
  Serial.print("║ SW3: ");
  Serial.print(digitalRead(SW3_PIN) == LOW ? "PRESSED " : "RELEASED");
  Serial.println("                               ║");
  
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // Isolated inputs
  Serial.print("║ ISOIN1: ");
  Serial.print(digitalRead(ISOIN1_PIN) == LOW ? "ACTIVE  " : "INACTIVE");
  Serial.print(" | ISOIN2: ");
  Serial.print(digitalRead(ISOIN2_PIN) == LOW ? "ACTIVE  " : "INACTIVE");
  Serial.println("  ║");
  
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // LED state
  Serial.print("║ LED (GPIO2): ");
  Serial.print(digitalRead(LED_PIN) ? "ON " : "OFF");
  Serial.println("                            ║");
  
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // Operating mode
  if (startup_completed) {
    Serial.println("║ Mode: Manual Control (SW1-3 to toggle)       ║");
  } else {
    Serial.println("║ Mode: STARTUP SEQUENCE                       ║");
  }
  
  Serial.println("╠═══════════════════════════════════════════════╣");
  
  // Uptime
  Serial.print("║ Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds                             ║");
  
  Serial.println("╚═══════════════════════════════════════════════╝\n");
}

// ===== Test OLED Display =====
void testOLED() {
  Serial.print("Testing OLED Display (I2C 0x3C)... ");
  
  // Initialize I2C
  Wire.begin();
  
  // Try to initialize OLED display
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    oled_available = true;
    Serial.println("✓ OLED Found!");
    
    // Clear display
    display.clearDisplay();
    
    // Display welcome message
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ThaiTechZone V2.0");
    display.println("Hardware Tester");
    display.println("");
    display.println("OLED: OK");
    display.println("I2C: 0x3C");
    display.display();
    
    delay(2000);  // Show welcome for 2 seconds
    
  } else {
    oled_available = false;
    Serial.println("✗ OLED Not Found");
    Serial.println("  Continuing without OLED display...");
  }
}

// ===== Update OLED Display =====
void updateOLED() {
  if (!oled_available) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Title
  display.setCursor(0, 0);
  display.println("ThaiTech IOTester");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Relay status
  display.setCursor(0, 12);
  display.print("R1:");
  display.print(!digitalRead(RL1_PIN) ? "ON " : "OFF");
  display.print(" R2:");
  display.print(!digitalRead(RL2_PIN) ? "ON " : "OFF");
  display.print(" R3:");
  display.println(!digitalRead(RL3_PIN) ? "ON " : "OFF");
  
  // Switch status
  display.setCursor(0, 22);
  display.print("SW1:");
  display.print(digitalRead(SW1_PIN) == LOW ? "P" : "R");
  display.print(" SW2:");
  display.print(digitalRead(SW2_PIN) == LOW ? "P" : "R");
  display.print(" SW3:");
  display.println(digitalRead(SW3_PIN) == LOW ? "P" : "R");
  
  // ISO Input status
  display.setCursor(0, 32);
  display.print("ISO1:");
  display.print(digitalRead(ISOIN1_PIN) == LOW ? "ACT" : "---");
  display.print(" ISO2:");
  display.println(digitalRead(ISOIN2_PIN) == LOW ? "ACT" : "---");
  
  // LED status
  display.setCursor(0, 42);
  display.print("LED: ");
  display.println(digitalRead(LED_PIN) ? "ON" : "OFF");
  
  // Mode status
  display.setCursor(0, 52);
  if (startup_completed) {
    display.print("Mode:MANUAL");
  } else {
    display.print("STARTUP...");
  }
  
  // Uptime
  display.setCursor(80, 52);
  display.print(millis() / 1000);
  display.print("s");
  
  display.display();
}
