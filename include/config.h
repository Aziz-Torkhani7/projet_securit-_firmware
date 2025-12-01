#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// PIN CONFIGURATION
// ============================================

// SIM800L Pins
#define SIM800L_TX_PIN 17 // ESP32 TX -> SIM800L RX
#define SIM800L_RX_PIN 16 // ESP32 RX -> SIM800L TX
#define SIM800L_BAUD 9600

// NEO-6M GPS Pins
#define GPS_TX_PIN 26 // ESP32 TX -> GPS RX
#define GPS_RX_PIN 27 // ESP32 RX -> GPS TX
#define GPS_BAUD 9600

// ============================================
// GSM/GPRS CONFIGURATION
// ============================================

// SIM Card APN Settings - Tunisie Telecom
#define APN "internet.tn" // Tunisie Telecom APN
#define GPRS_USER ""      // Usually empty for most carriers
#define GPRS_PASS ""      // Usually empty for most carriers

// ============================================
// MQTT CONFIGURATION
// ============================================

#define MQTT_BROKER "broker.hivemq.com" // Change to your broker
#define MQTT_PORT 1883
#define MQTT_USER "" // Update if authentication required
#define MQTT_PASS "" // Update if authentication required
#define MQTT_CLIENT_ID "ESP32_GPS_Tracker"

// MQTT Topics
#define MQTT_TOPIC_GPS "gps/location"
#define MQTT_TOPIC_STATUS "gps/status"

// ============================================
// API CONFIGURATION (For future HTTP API)
// ============================================

#define API_ENDPOINT "http://your-api.com/location" // Update with your API
#define API_KEY ""                                  // Your API key if needed

// ============================================
// TIMING CONFIGURATION
// ============================================

#define GPS_UPDATE_INTERVAL 5000     // Send GPS data every 5 seconds
#define MQTT_RECONNECT_INTERVAL 5000 // Try to reconnect to MQTT every 5 seconds
#define GSM_TIMEOUT 30000            // GSM connection timeout

// ============================================
// DEBUG CONFIGURATION
// ============================================

#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200
#define ENABLE_DEBUG true

#if ENABLE_DEBUG
#define DEBUG_PRINT(x) DEBUG_SERIAL.print(x)
#define DEBUG_PRINTLN(x) DEBUG_SERIAL.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
