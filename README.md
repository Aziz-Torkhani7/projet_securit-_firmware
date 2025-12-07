# ESP32-S3 GPS Tracker with GSM/GPRS and MQTT

A comprehensive IoT GPS tracking system built on ESP32-S3 that collects location data from a NEO-6M GPS module, transmits it via SIM800L GSM/GPRS module, and publishes to an MQTT broker (HiveMQ Cloud) for real-time monitoring.

## 📋 Table of Contents

- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Architecture](#software-architecture)
- [Features](#features)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [MQTT Topics](#mqtt-topics)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)

## 🎯 Overview

This project implements a GPS tracking device that:
1. Reads GPS coordinates from NEO-6M module via UART0
2. Connects to cellular network using SIM800L via UART1
3. Establishes GPRS data connection
4. Publishes location data to MQTT broker over TLS
5. Provides real-time status monitoring via serial debug

**Key Design:** Dual UART architecture - GPS and GSM modules operate on separate hardware serial ports for optimal performance.

## 🔧 Hardware Requirements

| Component | Model | Specifications |
|-----------|-------|----------------|
| Microcontroller | ESP32-S3 | Dual-core, WiFi/BLE capable |
| GPS Module | NEO-6M | UART, 9600 baud, -165dBm sensitivity |
| GSM Module | SIM800L | Quad-band, GPRS Class 10 |
| SIM Card | Any GSM carrier | Active data plan required |
| Power Supply | 5V/2A | Minimum 2A for SIM800L |

### Power Considerations

⚠️ **Important:** SIM800L requires stable 3.7-4.2V with 2A peak current capability. USB power may be insufficient. Use external power supply or LiPo battery.

## 📌 Pin Configuration

### GPS Module (NEO-6M) - UART0

```
ESP32-S3          NEO-6M
GPIO 17 (TX) ---> RX
GPIO 18 (RX) ---> TX
GND          ---> GND
5V           ---> VCC
```

### GSM Module (SIM800L) - UART1

```
ESP32-S3          SIM800L
GPIO 43 (TX) ---> RX
GPIO 44 (RX) ---> TX
GPIO 4       ---> RST (Reset)
GND          ---> GND
3.7-4.2V     ---> VCC (Use dedicated power)
```

### Debug Serial

```
ESP32-S3 USB: 115200 baud
```

## 🏗️ Software Architecture

### Component Modules

#### 1. **GPS Module** (`gps.cpp/h`)
- **Purpose:** Interface with NEO-6M GPS receiver
- **Library:** TinyGPSPlus v1.1.0
- **Functionality:**
  - Continuous NMEA sentence parsing
  - Location validity checking (fix status)
  - Satellite count monitoring
  - Lat/Lon/Alt/Speed extraction
  - Data age verification (<2 seconds)

**Key Methods:**
```cpp
bool begin(HardwareSerial *serial);      // Initialize GPS on UART
void update();                            // Parse incoming NMEA data
bool hasValidLocation();                  // Check for valid GPS fix
double getLatitude();                     // Get current latitude
double getLongitude();                    // Get current longitude
int getSatellites();                      // Get satellite count
```

#### 2. **GSM Module** (`gsm.cpp/h`)
- **Purpose:** Manage SIM800L cellular connectivity
- **Library:** TinyGSM v0.12.0
- **Functionality:**
  - Modem initialization with hardware reset
  - Network registration
  - Signal quality monitoring
  - GPRS/data connection management
  - SIM card status verification
  - Antenna diagnostics

**Key Methods:**
```cpp
bool begin(HardwareSerial *serial);      // Initialize GSM modem
bool connectGPRS();                       // Establish GPRS connection
bool isGPRSConnected();                   // Check GPRS status
int getSignalQuality();                   // Get signal strength (0-31)
void checkAntennaConnection();            // Diagnose antenna
```

**Network Connection Flow:**
1. Hardware reset via GPIO 4
2. Modem initialization (5 retries, 5s timeout)
3. SIM card verification
4. Network registration
5. Signal quality check
6. GPRS activation with APN

#### 3. **MQTT Client** (`mqtt_client.cpp/h`)
- **Purpose:** Handle MQTT communication over GPRS
- **Library:** PubSubClient v2.8
- **Functionality:**
  - TLS/SSL connection to HiveMQ Cloud
  - Automatic reconnection with exponential backoff
  - JSON message publishing
  - Connection state management

**Key Methods:**
```cpp
bool begin();                             // Initialize MQTT client
bool connect();                           // Connect to broker
bool publishLocation(String json);        // Publish GPS data
void reconnect();                         // Reconnect with backoff
bool isConnectedToBroker();              // Check connection status
```

**Backoff Algorithm:**
- Initial delay: 5 seconds
- Maximum delay: 60 seconds
- Multiplier: 2x per failed attempt

#### 4. **Main Application** (`main.cpp`)
- **Purpose:** Orchestrate all modules
- **Architecture:** Loop-based with millis() timing
- **Timing:**
  - GPS read: Every 100ms
  - MQTT publish: Every 5 seconds
  - Connectivity check: Every 10 seconds

**Execution Flow:**
```
Setup:
  1. Initialize debug serial (115200)
  2. Initialize GPS on UART0
  3. Initialize GSM on UART1
  4. Connect to GPRS network
  5. Connect to MQTT broker

Loop:
  1. Read & parse GPS data (100ms interval)
  2. Publish location if valid fix (5s interval)
  3. Monitor & restore connections (10s interval)
  4. Display system status
```

## ✨ Features

### Core Features
- ✅ Dual UART architecture (no port sharing conflicts)
- ✅ Real-time GPS tracking with validity checks
- ✅ Cellular GPRS connectivity
- ✅ Secure MQTT over TLS (port 8883)
- ✅ JSON-formatted location data
- ✅ Automatic reconnection handling
- ✅ Comprehensive error diagnostics

### GPS Features
- Satellite count monitoring
- Location validity verification
- Speed and altitude reporting
- Data freshness checking (2s max age)
- Character processing statistics

### GSM Features
- Hardware reset capability
- Network registration monitoring
- Signal strength reporting (dBm)
- SIM card status verification
- Antenna connection diagnostics
- Multi-retry initialization (5 attempts)

### MQTT Features
- TLS encryption
- Authenticated connections
- Automatic reconnection
- Exponential backoff
- Dual topics (location + status)
- Connection status messages

## 📦 Installation

### Prerequisites

1. **PlatformIO IDE** (VSCode extension)
2. **Git** (for cloning repository)

### Setup Steps

```bash
# Clone the repository
git clone https://github.com/Aziz-Torkhani7/projet_securit-_firmware.git
cd projet_securit-_firmware

# Install dependencies (automatic via platformio.ini)
pio pkg install

# Build the project
pio run

# Upload to ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor
```

### Dependencies (Auto-installed)

```ini
bblanchon/ArduinoJson @ ^7.2.1
mikalhart/TinyGPSPlus @ ^1.1.0
vshymanskyy/TinyGSM @ ^0.12.0
knolleary/PubSubClient @ ^2.8
```

## ⚙️ Configuration

All settings are in `include/config.h`:

### Network Settings

```cpp
// APN Configuration (Update for your carrier)
#define APN "internet.tn"          // Tunisie Telecom
#define GPRS_USER ""               // Usually empty
#define GPRS_PASS ""               // Usually empty
```

**Common APNs:**
- Tunisie Telecom: `internet.tn`
- Orange TN: `internet.orange.tn`
- Ooredoo TN: `internet.ooredoo.tn`

### MQTT Broker Settings

```cpp
#define MQTT_BROKER "e5d4da86ab7e42cd9edf1c3e52785053.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883                    // TLS port
#define MQTT_USER "Projet_FST_IOT"
#define MQTT_PASS "jE%NYGq_J]UD8?P"
#define MQTT_CLIENT_ID "ESP32_GPS_Tracker"
```

### Pin Assignments

```cpp
// GPS on UART0
#define GPS_TX_PIN 17
#define GPS_RX_PIN 18
#define GPS_BAUD 9600

// GSM on UART1
#define GSM_TX_PIN 43
#define GSM_RX_PIN 44
#define GSM_BAUD 9600
#define SIM800L_RESET_PIN 4
```

### Timing Parameters

```cpp
#define GPS_TASK_DELAY_MS 100           // GPS read interval
#define GPS_UPDATE_INTERVAL 5000        // MQTT publish interval
#define CONNECTIVITY_CHECK_MS 10000     // Connection check interval
#define GPS_DATA_MAX_AGE_MS 2000        // Max GPS data age
```

## 🚀 Usage

### First Run

1. **Insert SIM card** into SIM800L (data plan active)
2. **Connect hardware** per pin configuration
3. **Power ESP32-S3** with adequate supply
4. **Open serial monitor** at 115200 baud
5. **Wait for GPS fix** (may take 30-60s outdoors)

### Serial Output Example

```
========================================
ESP32-S3 GPS Tracker
Dual UART: GPS on UART0, GSM on UART1
========================================

Initializing system components...

1. Initializing UART0 for GPS...
   ✓ UART0 initialized

2. Initializing GPS module...
   ✓ GPS initialized successfully

3. Initializing UART1 for GSM...
   TX1 Pin: 43
   RX1 Pin: 44
   Baud: 9600
   ✓ UART1 initialized

4. Initializing GSM module...
   This may take up to 15 seconds...
   ✓ GSM initialized successfully
   Signal Quality: 18 (Fair - Acceptable)

5. Connecting to GPRS...
   SIM Status: Ready
   Signal Quality: 18
   Operator: Tunisie Telecom
   ✓ GPRS connected successfully

6. Initializing MQTT client...
   ✓ MQTT initialized successfully

7. Connecting to MQTT broker...
   ✓ MQTT connected successfully

System Ready!

GPS - Lat: 36.806389, Lon: 10.181667, Sats: 8
✓ Location published

System Status:
  GPS: OK | Fix: Valid | Satellites: 8
  GSM: OK | Signal: 18 | GPRS: Connected
  MQTT: Connected
  Free Heap: 252084 bytes
```

## 📡 MQTT Topics

### Location Data Topic
**Topic:** `gps/location`

**Valid Fix Message:**
```json
{
  "latitude": 36.806389,
  "longitude": 10.181667,
  "altitude": 12.50,
  "speed": 0.00,
  "satellites": 8,
  "valid": true,
  "timestamp": 123456
}
```

**Waiting for Fix Message:**
```json
{
  "status": "waiting_for_fix",
  "satellites": 3,
  "chars_processed": 1524,
  "valid": false,
  "timestamp": 123456
}
```

### Status Topic
**Topic:** `gps/status`

**Connection Message:**
```json
{
  "status": "connected",
  "client": "ESP32_GPS_Tracker",
  "timestamp": 5893
}
```

### Subscribing to Topics (HiveMQ Dashboard)

1. Login to HiveMQ Cloud Console
2. Navigate to "Web Client"
3. Subscribe to:
   - `gps/location`
   - `gps/status`
4. Monitor incoming messages in real-time

## 🔍 Troubleshooting

### GPS Issues

| Problem | Solution |
|---------|----------|
| No GPS fix | Move outdoors, wait 30-60s for cold start |
| Satellites: 0 | Check wiring (TX/RX not swapped), verify 5V power |
| Chars processed: 0 | GPS not sending data - check baud rate (9600) |

### GSM Issues

| Problem | Solution |
|---------|----------|
| GSM initialization failed | Check power (2A capable), verify SIM card inserted |
| Network registration failed | Check SIM has signal, verify APN settings |
| Signal: 99 | Antenna disconnected or no coverage |
| SIM not detected | Reinsert SIM, ensure gold contacts clean |

### MQTT Issues

| Error Code | Meaning | Solution |
|------------|---------|----------|
| -4 | Connection timeout | Check GPRS connection, verify broker URL |
| -2 | Connection failed | GPRS not active, check network |
| 4 | Bad credentials | Verify MQTT_USER and MQTT_PASS |
| 5 | Unauthorized | Check HiveMQ permissions |

### Power Issues

**Symptoms:**
- Frequent resets
- GSM initialization failures
- "Brownout detector" errors

**Solutions:**
- Use 5V/2A minimum power supply
- Add 1000µF capacitor near SIM800L VCC
- Use separate power for SIM800L
- Avoid USB power for production

### Memory Issues

**Monitor heap:**
```cpp
DEBUG_PRINT("Free Heap: ");
DEBUG_PRINTLN(ESP.getFreeHeap());
```

**If heap < 100KB:**
- Memory leak detected
- Check for String concatenation
- Verify MQTT client not recreated

## 📁 Project Structure

```
projetsecurite/
├── include/
│   ├── config.h              # Configuration constants
│   ├── gps.h                 # GPS module interface
│   ├── gsm.h                 # GSM module interface
│   └── mqtt_client.h         # MQTT client interface
├── src/
│   ├── main.cpp              # Main application
│   ├── gps.cpp               # GPS implementation
│   ├── gsm.cpp               # GSM implementation
│   └── mqtt_client.cpp       # MQTT implementation
├── lib/                      # Custom libraries (empty)
├── test/                     # Unit tests (empty)
├── platformio.ini            # PlatformIO configuration
├── README.md                 # This file
├── QUICK_REFERENCE.md        # Quick setup guide
└── RTOS_GUIDE.md            # Historical RTOS notes (deprecated)
```

### File Descriptions

| File | Purpose | Key Contents |
|------|---------|--------------|
| `config.h` | Centralized configuration | Pin definitions, network settings, MQTT credentials |
| `main.cpp` | Application orchestration | Setup, main loop, module coordination |
| `gps.cpp/h` | GPS functionality | TinyGPSPlus wrapper, location parsing |
| `gsm.cpp/h` | GSM functionality | TinyGSM wrapper, network management |
| `mqtt_client.cpp/h` | MQTT functionality | PubSubClient wrapper, publish/reconnect |
| `platformio.ini` | Build configuration | Board settings, dependencies, upload config |

## 🔒 Security Considerations

1. **MQTT Credentials:** Store in `config.h` (gitignored in production)
2. **TLS Encryption:** Port 8883 with certificate validation disabled (insecure mode for testing)
3. **Production:** Implement certificate pinning for full security

## 📊 Performance Metrics

- **GPS Update Rate:** 10 Hz (100ms intervals)
- **MQTT Publish Rate:** 0.2 Hz (every 5 seconds)
- **Memory Usage:** ~240KB heap available
- **Power Consumption:** ~2W average (SIM800L dominant)
- **Network Latency:** 500-2000ms typical for GPRS

## 🛠️ Development Notes

### Architecture Decisions

1. **Dual UART:** Eliminates GPS/GSM conflicts, no software serial overhead
2. **Loop-based:** Replaced FreeRTOS due to watchdog timeout issues during long network waits
3. **millis() timing:** Non-blocking intervals for concurrent operations
4. **Exponential backoff:** Prevents MQTT broker flooding during outages

### Known Limitations

- No GPS data buffering during network outages
- MQTT QoS 0 only (no guaranteed delivery)
- Single client connection (no multi-broker support)
- Insecure TLS (certificate validation disabled)

## 📝 License

This project is for educational purposes. Ensure compliance with local regulations regarding GPS tracking devices.

## 👥 Contributors

- **Author:** Aziz Torkhani
- **Organization:** FST (Faculté des Sciences de Tunis)
- **Project:** IoT Security Project

## 📞 Support

For issues or questions:
1. Check [Troubleshooting](#troubleshooting) section
2. Review serial output for error codes
3. Open issue on GitHub repository

---

**Last Updated:** December 7, 2025  
**Version:** 1.0.0  
**Board:** ESP32-S3  
**Platform:** PlatformIO
