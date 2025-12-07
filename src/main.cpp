#include "config.h"
#include "gps.h"
#include "gsm.h"
#include "mqtt_client.h"
#include <Arduino.h>

// ============================================
// GLOBAL OBJECTS
// ============================================

GPSModule gps;
GSMModule gsm;
MQTTClientModule *mqttClient = nullptr;

// Separate UARTs for GPS and GSM (Dual UART Architecture)
HardwareSerial gpsSerial(GPS_UART_NUM); // UART0 for GPS
HardwareSerial gsmSerial(GSM_UART_NUM); // UART1 for GSM

// ============================================
// STATE VARIABLES
// ============================================

bool gpsInitialized = false;
bool gsmInitialized = false;
bool mqttInitialized = false;

unsigned long lastGPSRead = 0;
unsigned long lastMQTTPublish = 0;
unsigned long lastConnectivityCheck = 0;

// ============================================
// FUNCTION DECLARATIONS
// ============================================

void initializeModules();

// ============================================
// SETUP FUNCTION
// ============================================

void setup() {
  // Initialize debug serial
  DEBUG_SERIAL.begin(DEBUG_BAUD);
  delay(2000);

  DEBUG_PRINTLN("\n\n========================================");
  DEBUG_PRINTLN("ESP32-S3 GPS Tracker");
  DEBUG_PRINTLN("Dual UART: GPS on UART0, GSM on UART1");
  DEBUG_PRINTLN("========================================\n");

  // Initialize all modules
  initializeModules();

  DEBUG_PRINTLN("\n========================================");
  DEBUG_PRINTLN("System Ready!");
  DEBUG_PRINTLN("========================================\n");
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  unsigned long currentTime = millis();

  // Read GPS data every 100ms
  if (currentTime - lastGPSRead >= GPS_TASK_DELAY_MS) {
    lastGPSRead = currentTime;

    if (gpsInitialized) {
      // Check if data is available on GPS serial port
      static unsigned long lastSerialCheck = 0;
      if (currentTime - lastSerialCheck >= 10000) { // Every 10 seconds
        lastSerialCheck = currentTime;
        DEBUG_PRINT("GPS Serial available bytes: ");
        DEBUG_PRINTLN(gpsSerial.available());
        if (gpsSerial.available() == 0) {
          DEBUG_PRINTLN("WARNING: No data from GPS module!");
          DEBUG_PRINTLN("Check: 1) GPS TX connected to ESP32 RX pin 44");
          DEBUG_PRINTLN("       2) GPS power (3.3V or 5V depending on module)");
          DEBUG_PRINTLN("       3) GPS antenna connected");
          DEBUG_PRINTLN("       4) GPS has clear view of sky");
        }
      }

      gps.update();

      if (gps.hasValidLocation()) {
        DEBUG_PRINT("GPS - Lat: ");
        DEBUG_PRINT(gps.getLatitude(), 6);
        DEBUG_PRINT(", Lon: ");
        DEBUG_PRINT(gps.getLongitude(), 6);
        DEBUG_PRINT(", Sats: ");
        DEBUG_PRINTLN(gps.getSatellites());
      }
    }
  }

  // Handle MQTT and publish every 5 seconds
  if (currentTime - lastMQTTPublish >= GPS_UPDATE_INTERVAL) {
    lastMQTTPublish = currentTime;

    // Handle MQTT loop
    if (mqttInitialized && mqttClient) {
      mqttClient->loop();
    }

    // Publish GPS data if available
    if (gpsInitialized && gps.hasValidLocation()) {
      if (mqttInitialized && mqttClient && mqttClient->isConnectedToBroker()) {
        // Create JSON
        char locationJSON[MQTT_BUFFER_SIZE];
        snprintf(locationJSON, sizeof(locationJSON),
                 "{"
                 "\"latitude\":%.6f,"
                 "\"longitude\":%.6f,"
                 "\"altitude\":%.2f,"
                 "\"speed\":%.2f,"
                 "\"satellites\":%d,"
                 "\"valid\":true,"
                 "\"timestamp\":%lu"
                 "}",
                 gps.getLatitude(), gps.getLongitude(), gps.getAltitude(),
                 gps.getSpeed(), gps.getSatellites(), currentTime);

        if (mqttClient->publishLocation(String(locationJSON))) {
          DEBUG_PRINTLN("✓ Location published");
        } else {
          DEBUG_PRINTLN("✗ Failed to publish");
        }
      } else {
        DEBUG_PRINTLN("MQTT not connected");
      }
    } else {
      // GPS fix not available - publish status
      static unsigned long lastGPSStatusLog = 0;
      if (currentTime - lastGPSStatusLog >= 5000) {
        lastGPSStatusLog = currentTime;
        DEBUG_PRINT("Waiting for GPS fix... Satellites: ");
        DEBUG_PRINT(gps.getSatellites());
        DEBUG_PRINT(", Chars processed: ");
        DEBUG_PRINTLN(gps.getCharsProcessed());

        // Publish GPS status to MQTT
        if (mqttInitialized && mqttClient &&
            mqttClient->isConnectedToBroker()) {
          char statusJSON[256];
          snprintf(statusJSON, sizeof(statusJSON),
                   "{"
                   "\"status\":\"waiting_for_fix\","
                   "\"satellites\":%d,"
                   "\"chars_processed\":%lu,"
                   "\"valid\":false,"
                   "\"timestamp\":%lu"
                   "}",
                   gps.getSatellites(), gps.getCharsProcessed(), currentTime);
          mqttClient->publishLocation(String(statusJSON));
        }
      }
    }
  }

  // Check connectivity every 10 seconds
  if (currentTime - lastConnectivityCheck >= CONNECTIVITY_CHECK_MS) {
    lastConnectivityCheck = currentTime;

    // Check GPRS connection
    if (gsmInitialized && !gsm.isGPRSConnected()) {
      DEBUG_PRINTLN("GPRS disconnected, reconnecting...");
      gsm.connectGPRS();
    }

    // Check MQTT connection
    if (mqttInitialized && mqttClient && !mqttClient->isConnectedToBroker()) {
      DEBUG_PRINTLN("MQTT disconnected, reconnecting...");
      mqttClient->reconnect();
    }

    // Print system status
    DEBUG_PRINTLN("\nSystem Status:");
    DEBUG_PRINT("  GPS: ");
    DEBUG_PRINT(gpsInitialized ? "OK" : "FAIL");
    if (gpsInitialized) {
      DEBUG_PRINT(" | Fix: ");
      DEBUG_PRINT(gps.hasValidLocation() ? "Valid" : "No fix");
      DEBUG_PRINT(" | Satellites: ");
      DEBUG_PRINTLN(gps.getSatellites());
    } else {
      DEBUG_PRINTLN();
    }

    DEBUG_PRINT("  GSM: ");
    DEBUG_PRINT(gsmInitialized ? "OK" : "FAIL");
    if (gsmInitialized) {
      DEBUG_PRINT(" | Signal: ");
      DEBUG_PRINT(gsm.getSignalQuality());
      DEBUG_PRINT(" | GPRS: ");
      DEBUG_PRINTLN(gsm.isGPRSConnected() ? "Connected" : "Disconnected");
    } else {
      DEBUG_PRINTLN();
    }

    DEBUG_PRINT("  MQTT: ");
    if (mqttInitialized && mqttClient) {
      DEBUG_PRINTLN(mqttClient->isConnectedToBroker() ? "Connected"
                                                      : "Disconnected");
    } else {
      DEBUG_PRINTLN("Not initialized");
    }

    DEBUG_PRINT("  Free Heap: ");
    DEBUG_PRINT(ESP.getFreeHeap());
    DEBUG_PRINTLN(" bytes\n");
  }

  // Small delay to prevent tight looping
  delay(10);
}

// ============================================
// FUNCTION DEFINITIONS
// ============================================

void initializeModules() {
  DEBUG_PRINTLN("Initializing system components...\n");

  // Initialize UART0 for GPS (NEO-6M)
  DEBUG_PRINTLN("1. Initializing UART0 for GPS...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(100);
  DEBUG_PRINTLN("   ✓ UART0 initialized");

  // Initialize GPS module
  DEBUG_PRINTLN("\n2. Initializing GPS module...");
  gpsInitialized = gps.begin(&gpsSerial);
  if (gpsInitialized) {
    DEBUG_PRINTLN("   ✓ GPS initialized successfully");
  } else {
    DEBUG_PRINTLN("   ✗ GPS initialization failed");
  }

  // Initialize UART1 for GSM (SIM800L)
  DEBUG_PRINTLN("\n3. Initializing UART1 for GSM...");
  DEBUG_PRINT("   TX1 Pin: ");
  DEBUG_PRINTLN(GSM_TX_PIN);
  DEBUG_PRINT("   RX1 Pin: ");
  DEBUG_PRINTLN(GSM_RX_PIN);
  DEBUG_PRINT("   Baud: ");
  DEBUG_PRINTLN(GSM_BAUD);
  gsmSerial.begin(GSM_BAUD, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(100);
  DEBUG_PRINTLN("   ✓ UART1 initialized");

  // Initialize GSM module
  DEBUG_PRINTLN("\n4. Initializing GSM module...");
  DEBUG_PRINTLN("   This may take up to 15 seconds...");
  gsmInitialized = gsm.begin(&gsmSerial);
  if (gsmInitialized) {
    DEBUG_PRINTLN("   ✓ GSM initialized successfully");

    // Test antenna connection
    gsm.checkAntennaConnection();
  } else {
    DEBUG_PRINTLN("   ✗ GSM initialization failed");
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("   TROUBLESHOOTING TIPS:");
    DEBUG_PRINTLN("   1. Check SIM800L power: 3.7-4.2V with 2A capability");
    DEBUG_PRINTLN("   2. Verify antenna is connected");
    DEBUG_PRINTLN("   3. Check UART connections (TX1<->RX, RX1<->TX)");
    DEBUG_PRINTLN("   4. Ensure reset pin (GPIO 5) is connected");
    DEBUG_PRINTLN("   5. Try external power supply (not USB power)");
  }

  // Connect to GPRS
  if (gsmInitialized) {
    DEBUG_PRINTLN("\n5. Connecting to GPRS...");
    if (gsm.connectGPRS()) {
      DEBUG_PRINTLN("   ✓ GPRS connected successfully");

      // Initialize MQTT
      DEBUG_PRINTLN("\n6. Initializing MQTT client...");
      mqttClient = new MQTTClientModule(&gsm);
      mqttInitialized = mqttClient->begin();

      if (mqttInitialized) {
        DEBUG_PRINTLN("   ✓ MQTT initialized successfully");

        // Connect to MQTT broker
        DEBUG_PRINTLN("\n7. Connecting to MQTT broker...");
        if (mqttClient->connect()) {
          DEBUG_PRINTLN("   ✓ MQTT connected successfully");
        } else {
          DEBUG_PRINTLN("   ✗ MQTT connection failed");
        }
      } else {
        DEBUG_PRINTLN("   ✗ MQTT initialization failed");
      }
    } else {
      DEBUG_PRINTLN("   ✗ GPRS connection failed");
    }
  }

  DEBUG_PRINTLN("\n========================================");
  DEBUG_PRINTLN("Module initialization complete!");
  DEBUG_PRINTLN("========================================\n");
}
