#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// PIN CONFIGURATION - DUAL UART ARCHITECTURE
// ============================================
// This implementation uses TWO separate UARTs:
// - UART0 for GPS (dedicated)
// - UART1 for GSM (dedicated)
// No UART sharing or multiplexing is used.

// GPS (NEO-6M) - UART0 (Dedicated)
#define GPS_UART_NUM 0 // UART0
#define GPS_TX_PIN 17  // TX - Connect to NEO-6M RX
#define GPS_RX_PIN 18  // RX - Connect to NEO-6M TX
#define GPS_BAUD 9600  // NEO-6M default baud rate

// GSM (SIM800L) - UART1 (Dedicated)
#define GSM_UART_NUM 1 // UART1
#define GSM_TX_PIN 43  // TX - Connect to SIM800L RX
#define GSM_RX_PIN 44  // RX - Connect to SIM800L TX
#define GSM_BAUD 9600  // SIM800L default baud rate

// SIM800L Control Pins
#define SIM800L_RESET_PIN 4  // SIM800L hardware reset pin (RST)
#define SIM800L_POWER_PIN -1 // Optional: Set to GPIO if using power control

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

#define MQTT_BROKER "yourbroker.com" // Replace with your MQTT broker addres
#define MQTT_PORT 8883 // TLS port (MQTT over TLS/SSL)
#define MQTT_USER "your_username" // Your MQTT username
#define MQTT_PASS "your_password" // Your MQTT password
#define MQTT_CLIENT_ID "ESP32_GPS_Tracker"
#define MQTT_BUFFER_SIZE 512

// MQTT Topics
#define MQTT_TOPIC_GPS "gps/location"
#define MQTT_TOPIC_STATUS "gps/status"

                           // Your API key if needed

// ============================================
// TIMING CONFIGURATION
// ============================================

#define GPS_TASK_DELAY_MS 100        // Read GPS data every 100ms
#define GPS_READ_DURATION_MS 20      // Time to read GPS data per update
#define GPS_DATA_MAX_AGE_MS 2000     // Max age for valid GPS data (2 seconds)
#define GPS_MIN_CHARS_PROCESSED 100  // Min characters to consider GPS ready
#define GPS_UPDATE_INTERVAL 5000     // Send GPS data every 5 seconds
#define CONNECTIVITY_CHECK_MS 10000  // Check connectivity every 10 seconds
#define MQTT_RECONNECT_INTERVAL 5000 // Try to reconnect to MQTT every 5 seconds
#define MQTT_RECONNECT_MAX_INTERVAL 60000UL // Max backoff interval
#define GSM_TIMEOUT 30000                   // GSM connection timeout

// ============================================
// DEBUG CONFIGURATION
// ============================================

#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200
#define ENABLE_DEBUG true

#if ENABLE_DEBUG
#define DEBUG_PRINT(...) DEBUG_SERIAL.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) DEBUG_SERIAL.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

#endif // CONFIG_H
