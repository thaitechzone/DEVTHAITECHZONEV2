"""
MQTT Control Script for ESP32 ThaiTechZone V2.0
This script demonstrates how to control the ESP32 board via MQTT

Requirements:
    pip install paho-mqtt

Usage:
    python mqtt_control.py
"""

import paho.mqtt.client as mqtt
import json
import time
import sys

# MQTT Configuration
MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883
MQTT_CLIENT_ID = "Python_Controller"

# MQTT Topics
TOPIC_STATUS = "thaitechzone/v2/status"
TOPIC_INPUTS = "thaitechzone/v2/inputs"
TOPIC_WEATHER = "thaitechzone/v2/weather"
TOPIC_AIR_QUALITY = "thaitechzone/v2/airquality"
TOPIC_MODE = "thaitechzone/v2/mode"

# Command Topics
TOPIC_MODE_CMD = "thaitechzone/v2/mode/cmd"
TOPIC_RELAY_CMD = "thaitechzone/v2/relay/cmd"
TOPIC_AUX_CMD = "thaitechzone/v2/aux/cmd"

# Global variables
current_mode = "unknown"
relay_states = {"rl1": False, "rl2": False, "rl3": False}

# Callback when connected to MQTT broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Connected to MQTT Broker!")
        print(f"   Server: {MQTT_BROKER}")
        print()
        
        # Subscribe to all status topics
        client.subscribe(TOPIC_STATUS)
        client.subscribe(TOPIC_INPUTS)
        client.subscribe(TOPIC_WEATHER)
        client.subscribe(TOPIC_AIR_QUALITY)
        client.subscribe(TOPIC_MODE)
        
        print("📡 Subscribed to status topics")
        print()
    else:
        print(f"❌ Connection failed with code {rc}")

# Callback when a message is received
def on_message(client, userdata, msg):
    global current_mode, relay_states
    
    try:
        payload = json.loads(msg.payload.decode())
        
        if msg.topic == TOPIC_STATUS:
            print("📊 System Status:")
            print(f"   Mode: {payload.get('mode', 'N/A')}")
            print(f"   Uptime: {payload.get('uptime', 0)} seconds")
            print(f"   WiFi RSSI: {payload.get('wifi_rssi', 0)} dBm")
            
            if 'relays' in payload:
                relay_states = payload['relays']
                print(f"   Relays: RL1={relay_states['rl1']}, RL2={relay_states['rl2']}, RL3={relay_states['rl3']}")
            
            if 'aux' in payload:
                aux = payload['aux']
                print(f"   AUX: AUX1={aux['aux1']}, AUX2={aux['aux2']}, AUX3={aux['aux3']}, AUX4={aux['aux4']}")
            print()
            
        elif msg.topic == TOPIC_INPUTS:
            print("🔘 Input States:")
            print(f"   SW1={payload.get('sw1', False)}, SW2={payload.get('sw2', False)}, SW3={payload.get('sw3', False)}")
            print(f"   ISO1={payload.get('iso1', False)}, ISO2={payload.get('iso2', False)}")
            print()
            
        elif msg.topic == TOPIC_WEATHER:
            print("🌤️  Weather Data:")
            print(f"   City: {payload.get('city', 'N/A')}")
            print(f"   Temperature: {payload.get('temperature', 0)}°C")
            print(f"   Humidity: {payload.get('humidity', 0)}%")
            print(f"   Description: {payload.get('description', 'N/A')}")
            print()
            
        elif msg.topic == TOPIC_AIR_QUALITY:
            print("💨 Air Quality:")
            print(f"   PM2.5: {payload.get('pm2_5', 0)} µg/m³")
            print(f"   PM10: {payload.get('pm10', 0)} µg/m³")
            print(f"   AQI: {payload.get('aqi', 0)} ({payload.get('quality', 'N/A')})")
            print()
            
        elif msg.topic == TOPIC_MODE:
            current_mode = payload.get('mode', 'unknown')
            print(f"🔄 Mode Changed: {current_mode.upper()}")
            print()
            
    except json.JSONDecodeError:
        print(f"⚠️  Invalid JSON from topic {msg.topic}")
    except Exception as e:
        print(f"⚠️  Error processing message: {e}")

# Function to switch mode
def switch_mode(client, mode):
    """Switch between auto and manual mode"""
    print(f"\n🔄 Switching to {mode.upper()} mode...")
    payload = json.dumps({"mode": mode})
    client.publish(TOPIC_MODE_CMD, payload)
    time.sleep(1)

# Function to control relay
def control_relay(client, relay_num, state):
    """Control a specific relay (1-3)"""
    print(f"\n🔌 Setting Relay {relay_num} to {'ON' if state else 'OFF'}...")
    payload = json.dumps({"relay": relay_num, "state": state})
    client.publish(TOPIC_RELAY_CMD, payload)
    time.sleep(0.5)

# Function to control AUX
def control_aux(client, aux_num, state):
    """Control a specific AUX output (1-4)"""
    print(f"\n⚡ Setting AUX {aux_num} to {'ON' if state else 'OFF'}...")
    payload = json.dumps({"aux": aux_num, "state": state})
    client.publish(TOPIC_AUX_CMD, payload)
    time.sleep(0.5)

# Interactive menu
def print_menu():
    print("\n" + "="*50)
    print("   ESP32 ThaiTechZone V2.0 - MQTT Controller")
    print("="*50)
    print("1. Switch to AUTO mode")
    print("2. Switch to MANUAL mode")
    print("3. Control Relay 1")
    print("4. Control Relay 2")
    print("5. Control Relay 3")
    print("6. Control AUX 1")
    print("7. Control AUX 2")
    print("8. Control AUX 3")
    print("9. Control AUX 4")
    print("0. Exit")
    print("="*50)

# Main function
def main():
    print("🚀 Starting ESP32 MQTT Controller...")
    print()
    
    # Create MQTT client
    client = mqtt.Client(MQTT_CLIENT_ID)
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Connect to broker
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
    except Exception as e:
        print(f"❌ Failed to connect to broker: {e}")
        return
    
    # Start background thread
    client.loop_start()
    
    # Wait for connection
    time.sleep(2)
    
    # Interactive control loop
    try:
        while True:
            print_menu()
            choice = input("\nEnter your choice: ").strip()
            
            if choice == "1":
                switch_mode(client, "auto")
                
            elif choice == "2":
                switch_mode(client, "manual")
                
            elif choice == "3":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_relay(client, 1, state)
                    
            elif choice == "4":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_relay(client, 2, state)
                    
            elif choice == "5":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_relay(client, 3, state)
                    
            elif choice == "6":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_aux(client, 1, state)
                    
            elif choice == "7":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_aux(client, 2, state)
                    
            elif choice == "8":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_aux(client, 3, state)
                    
            elif choice == "9":
                if current_mode != "manual":
                    print("⚠️  Please switch to MANUAL mode first!")
                else:
                    state = input("State (ON/OFF): ").strip().upper() == "ON"
                    control_aux(client, 4, state)
                    
            elif choice == "0":
                print("\n👋 Exiting...")
                break
                
            else:
                print("❌ Invalid choice!")
                
    except KeyboardInterrupt:
        print("\n\n👋 Interrupted by user")
    finally:
        client.loop_stop()
        client.disconnect()
        print("✅ Disconnected from MQTT Broker")

if __name__ == "__main__":
    main()
