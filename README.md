# ESP32 GPS Tracker with SIM800L

A real-time GPS tracking system using ESP32, NEO-6M GPS module, and SIM800L GSM/GPRS module. The system sends live location data to the cloud via MQTT and can be extended with HTTP API support.

## 📋 Hardware Requirements

- **ESP32 Board**: 4D Systems ESP32-S3 Gen4 R8N16 (or any ESP32 variant)
- **GPS Module**: NEO-6M
- **GSM Module**: SIM800L
- **SIM Card**: With data plan and configured APN
- **Power Supply**: 5V for ESP32, ensure SIM800L has adequate power (3.7-4.2V, peak 2A)

## 🔌 Wiring Connections

### NEO-6M GPS Module
```
NEO-6M    →    ESP32
--------------------------
VCC       →    3.3V or 5V
GND       →    GND
TX        →    GPIO 27 (RX)
RX        →    GPIO 26 (TX)
```

### SIM800L GSM Module
```
SIM800L   →    ESP32
--------------------------
VCC       →    3.7-4.2V (use separate power supply!)
GND       →    GND (common ground with ESP32)
TX        →    GPIO 16 (RX)
RX        →    GPIO 17 (TX)
```

⚠️ **Important**: SIM800L requires a stable power supply capable of providing 2A peak current. Do not power it directly from ESP32's 3.3V pin!

## ⚙️ Configuration

### 1. Update Pin Configuration (if needed)

Edit `include/config.h` to match your wiring:

```cpp
// SIM800L Pins
#define SIM800L_TX_PIN 17
#define SIM800L_RX_PIN 16

// NEO-6M GPS Pins
#define GPS_TX_PIN 26
#define GPS_RX_PIN 27
```

### 2. Configure Your SIM Card APN

Edit `include/config.h` with your carrier's APN settings:

```cpp
#define APN "internet"        // Your carrier APN
#define GPRS_USER ""          // Usually empty
#define GPRS_PASS ""          // Usually empty
```

**Common APN Settings**:
- **Tunisie Telecom**: `internet.tn` (configured)
- Orange Morocco: `internet.ma`
- Maroc Telecom: `www.mtds.ma`
- Inwi: `www.inwi.ma`
- AT&T (US): `broadband`
- Vodafone: `internet`

### 3. Configure MQTT Broker

Edit `include/config.h`:

```cpp
#define MQTT_BROKER "broker.hivemq.com"  // Your MQTT broker
#define MQTT_PORT 1883
#define MQTT_USER ""                      // If authentication required
#define MQTT_PASS ""                      // If authentication required
```

**Free MQTT Brokers for Testing**:
- `broker.hivemq.com` (port 1883)
- `test.mosquitto.org` (port 1883)
- `broker.emqx.io` (port 1883)

## 🚀 Building and Uploading

### Using PlatformIO CLI

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Using PlatformIO in VS Code

1. Open the project in VS Code
2. Click the PlatformIO icon in the sidebar
3. Click "Build" (checkmark icon)
4. Click "Upload" (arrow icon)
5. Click "Serial Monitor" to view output

## 📊 Data Format

### GPS Location Data (MQTT Topic: `gps/location`)

```json
{
  "latitude": 33.573110,
  "longitude": -7.589843,
  "altitude": 50.5,
  "speed": 15.3,
  "satellites": 8,
  "datetime": "2025-12-01 14:30:45",
  "valid": true
}
```

### Status Data (MQTT Topic: `gps/status`)

```json
{
  "gps_fix": true,
  "satellites": 8,
  "signal": 25,
  "uptime": 3600
}
```

## 📱 Testing with MQTT Client

### Using mosquitto_sub (Linux/Mac)

```bash
# Subscribe to location updates
mosquitto_sub -h broker.hivemq.com -t "gps/location"

# Subscribe to status updates
mosquitto_sub -h broker.hivemq.com -t "gps/status"
```

### Using MQTT Explorer (GUI)

1. Download MQTT Explorer
2. Connect to your MQTT broker
3. Subscribe to `gps/location` and `gps/status` topics
4. View real-time GPS data

## 🔧 Troubleshooting

### GPS Not Getting Fix
- Ensure GPS module has clear view of sky
- Wait 1-5 minutes for initial GPS fix (cold start)
- Check wiring connections
- Verify GPS module is receiving power

### SIM800L Not Connecting
- Ensure SIM card is properly inserted
- Verify SIM card has active data plan
- Check APN settings for your carrier
- Ensure adequate power supply (2A peak)
- Check for correct TX/RX pin connections

### MQTT Connection Failed
- Verify GPRS is connected first
- Check MQTT broker address and port
- Test broker connectivity from another device

## 📁 Project Structure

```
projetsecurite/
├── include/
│   ├── config.h          # Configuration settings
│   ├── gps.h             # GPS module interface
│   ├── gsm.h             # GSM module interface
│   └── mqtt_client.h     # MQTT client interface
├── src/
│   ├── main.cpp          # Main application
│   ├── gps.cpp           # GPS implementation
│   ├── gsm.cpp           # GSM implementation
│   └── mqtt_client.cpp   # MQTT implementation
└── platformio.ini        # PlatformIO configuration
```

## 🔄 Update Intervals

Configured in `include/config.h`:

```cpp
#define GPS_UPDATE_INTERVAL 5000       // Send GPS every 5 seconds
#define MQTT_RECONNECT_INTERVAL 5000   // Reconnect attempt interval
```

## 🌐 Features

✅ Modular firmware architecture  
✅ SIM800L GSM/GPRS connectivity  
✅ NEO-6M GPS support  
✅ MQTT publishing of live GPS data  
✅ JSON formatted output  
✅ Auto-reconnect functionality  
✅ Status monitoring  
✅ Detailed debug output  

## 🔮 Future Enhancements

- [ ] HTTP API integration
- [ ] SD card logging
- [ ] Geofencing alerts
- [ ] Low power mode
- [ ] OTA firmware updates
- [ ] SMS command interface

## 📝 Notes

- First GPS fix can take 1-5 minutes (cold start)
- SIM800L draws high current - use appropriate power supply
- Monitor data usage on your SIM card
- Serial monitor baud rate: 115200

## 📄 License

This project is provided as-is for educational and commercial use.