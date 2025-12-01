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

// ============================================
// TIMING VARIABLES
// ============================================

unsigned long lastGPSUpdate = 0;
unsigned long lastStatusUpdate = 0;
const unsigned long STATUS_UPDATE_INTERVAL =
    60000; // Send status every 60 seconds

// ============================================
// STATE VARIABLES
// ============================================

bool gpsInitialized = false;
bool gsmInitialized = false;
bool mqttInitialized = false;

// ============================================
// FUNCTION DECLARATIONS
// ============================================

void initializeModules();
void handleGPSData();
void handleConnectivity();
void printSystemStatus();

// ============================================
// SETUP FUNCTION
// ============================================

void setup() {
  // Initialize debug serial
  DEBUG_SERIAL.begin(DEBUG_BAUD);
  delay(2000);

  DEBUG_PRINTLN("\n\n========================================");
  DEBUG_PRINTLN("ESP32 GPS Tracker with SIM800L");
  DEBUG_PRINTLN("========================================\n");

  // Initialize all modules
  initializeModules();
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  // Update GPS data continuously
  if (gpsInitialized) {
    gps.update();
  }

  // Handle MQTT connection and messages
  if (mqttInitialized && mqttClient) {
    mqttClient->loop();

    // Auto-reconnect if disconnected
    if (!mqttClient->isConnectedToBroker()) {
      handleConnectivity();
    }
  }

  // Send GPS data at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastGPSUpdate >= GPS_UPDATE_INTERVAL) {
    lastGPSUpdate = currentMillis;
    handleGPSData();
  }

  // Send status update periodically
  if (currentMillis - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
    lastStatusUpdate = currentMillis;
    printSystemStatus();
  }

  // Small delay to prevent watchdog issues
  delay(10);
}

// ============================================
// FUNCTION DEFINITIONS
// ============================================

void initializeModules() {
  DEBUG_PRINTLN("Initializing modules...\n");

  // Initialize GPS
  DEBUG_PRINTLN("1. Initializing GPS module...");
  gpsInitialized = gps.begin();
  if (gpsInitialized) {
    DEBUG_PRINTLN("   ✓ GPS initialized successfully");
  } else {
    DEBUG_PRINTLN("   ✗ GPS initialization failed");
  }
  delay(1000);

  // Initialize GSM
  DEBUG_PRINTLN("\n2. Initializing GSM module...");
  gsmInitialized = gsm.begin();
  if (gsmInitialized) {
    DEBUG_PRINTLN("   ✓ GSM initialized successfully");
  } else {
    DEBUG_PRINTLN("   ✗ GSM initialization failed");
  }
  delay(1000);

  // Connect to GPRS
  if (gsmInitialized) {
    DEBUG_PRINTLN("\n3. Connecting to GPRS...");
    if (gsm.connectGPRS()) {
      DEBUG_PRINTLN("   ✓ GPRS connected successfully");

      // Initialize MQTT
      DEBUG_PRINTLN("\n4. Initializing MQTT client...");
      mqttClient = new MQTTClientModule(&gsm);
      mqttInitialized = mqttClient->begin();

      if (mqttInitialized) {
        DEBUG_PRINTLN("   ✓ MQTT initialized successfully");

        // Connect to MQTT broker
        DEBUG_PRINTLN("\n5. Connecting to MQTT broker...");
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
  DEBUG_PRINTLN("Initialization complete!");
  DEBUG_PRINTLN("========================================\n");
}

void handleGPSData() {
  if (!gpsInitialized) {
    return;
  }

  // Check if we have valid GPS fix
  if (gps.hasValidLocation()) {
    // Get location data as JSON
    String locationJSON = gps.getLocationJSON();

    DEBUG_PRINTLN("\n--- GPS Data ---");
    DEBUG_PRINTLN(locationJSON);
    DEBUG_PRINTLN("----------------\n");

    // Publish to MQTT if connected
    if (mqttInitialized && mqttClient && mqttClient->isConnectedToBroker()) {
      if (mqttClient->publishLocation(locationJSON)) {
        DEBUG_PRINTLN("✓ Location sent to MQTT broker");
      } else {
        DEBUG_PRINTLN("✗ Failed to send location to MQTT");
      }
    }

    // TODO: Send to HTTP API (to be implemented)
    // if (gsmInitialized && gsm.isGPRSConnected()) {
    //     gsm.sendHTTPPost(API_ENDPOINT, locationJSON.c_str());
    // }

  } else {
    DEBUG_PRINTLN("⚠ Waiting for GPS fix...");
    DEBUG_PRINT("   Satellites: ");
    DEBUG_PRINTLN(gps.getSatellites());
  }
}

void handleConnectivity() {
  DEBUG_PRINTLN("\n⚠ Connection lost, attempting to reconnect...");

  // Check GPRS connection
  if (!gsm.isGPRSConnected()) {
    DEBUG_PRINTLN("GPRS disconnected, reconnecting...");
    gsm.connectGPRS();
  }

  // Reconnect MQTT
  if (mqttInitialized && mqttClient) {
    mqttClient->reconnect();
  }
}

void printSystemStatus() {
  DEBUG_PRINTLN("\n========================================");
  DEBUG_PRINTLN("SYSTEM STATUS");
  DEBUG_PRINTLN("========================================");

  // GPS Status
  DEBUG_PRINT("GPS: ");
  DEBUG_PRINTLN(gpsInitialized ? "Initialized" : "Failed");
  if (gpsInitialized) {
    DEBUG_PRINT("  Fix: ");
    DEBUG_PRINTLN(gps.hasValidLocation() ? "Valid" : "No fix");
    DEBUG_PRINT("  Satellites: ");
    DEBUG_PRINTLN(gps.getSatellites());
  }

  // GSM Status
  DEBUG_PRINT("\nGSM: ");
  DEBUG_PRINTLN(gsmInitialized ? "Initialized" : "Failed");
  if (gsmInitialized) {
    DEBUG_PRINT("  Signal: ");
    DEBUG_PRINTLN(gsm.getSignalQuality());
    DEBUG_PRINT("  GPRS: ");
    DEBUG_PRINTLN(gsm.isGPRSConnected() ? "Connected" : "Disconnected");
  }

  // MQTT Status
  DEBUG_PRINT("\nMQTT: ");
  DEBUG_PRINTLN(mqttInitialized ? "Initialized" : "Failed");
  if (mqttInitialized && mqttClient) {
    DEBUG_PRINT("  Broker: ");
    DEBUG_PRINTLN(mqttClient->isConnectedToBroker() ? "Connected"
                                                    : "Disconnected");
  }

  DEBUG_PRINTLN("========================================\n");

  // Send status via MQTT if connected
  if (mqttInitialized && mqttClient && mqttClient->isConnectedToBroker()) {
    String status = "{";
    status +=
        "\"gps_fix\":" + String(gps.hasValidLocation() ? "true" : "false") +
        ",";
    status += "\"satellites\":" + String(gps.getSatellites()) + ",";
    status += "\"signal\":" + String(gsm.getSignalQuality()) + ",";
    status += "\"uptime\":" + String(millis() / 1000);
    status += "}";
    mqttClient->publishStatus(status);
  }
}