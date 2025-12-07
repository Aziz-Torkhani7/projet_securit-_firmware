#include "gsm.h"

GSMModule::GSMModule()
    : isInitialized(false), isConnected(false), lastConnectionAttempt(0),
      gsmSerial(nullptr), modem(nullptr), client(nullptr) {}

GSMModule::~GSMModule() {
  if (client)
    delete client;
  if (modem)
    delete modem;
  // Don't delete gsmSerial - it's a reference
}

void GSMModule::hardwareReset() {
  DEBUG_PRINTLN("Performing hardware reset of SIM800L...");
  DEBUG_PRINT("Reset pin: ");
  DEBUG_PRINTLN(SIM800L_RESET_PIN);

  pinMode(SIM800L_RESET_PIN, OUTPUT);
  digitalWrite(SIM800L_RESET_PIN, HIGH);
  delay(100);
  digitalWrite(SIM800L_RESET_PIN, LOW);
  delay(200);
  digitalWrite(SIM800L_RESET_PIN, HIGH);

  DEBUG_PRINTLN("Waiting for SIM800L to boot (5 seconds)...");
  delay(5000); // Wait longer for module to boot

  DEBUG_PRINTLN("SIM800L hardware reset complete");
}

bool GSMModule::begin(HardwareSerial *serial) {
  DEBUG_PRINTLN("Setting up GSM module...");

  gsmSerial = serial;
  // UART is already initialized in main.cpp

  // Perform hardware reset
  hardwareReset();

  // Create modem instance
  modem = new TinyGsm(*gsmSerial);
  client = new TinyGsmClient(*modem);

  // Check if modem is responding (try multiple times)
  DEBUG_PRINTLN("Testing modem communication...");

  String modemInfo = "";
  for (int retry = 0; retry < 5; retry++) {
    DEBUG_PRINT("Attempt ");
    DEBUG_PRINT(retry + 1);
    DEBUG_PRINTLN("/5...");

    // Try AT command first
    modem->testAT();
    delay(500);

    modemInfo = modem->getModemInfo();
    if (modemInfo.length() > 0) {
      break;
    }
    delay(1000);
  }

  DEBUG_PRINT("Modem Info: ");
  DEBUG_PRINTLN(modemInfo);

  if (modemInfo.length() == 0) {
    DEBUG_PRINTLN("✗ Modem not responding!");
    DEBUG_PRINTLN("Check: Power supply (3.7-4.2V, 2A), UART pins, antenna");
    return false;
  }

  isInitialized = true;
  DEBUG_PRINTLN("✓ GSM module initialized on UART1");

  return true;
}

bool GSMModule::connectGPRS() {
  if (!isInitialized) {
    DEBUG_PRINTLN("GSM not initialized!");
    return false;
  }

  // Don't attempt to reconnect too frequently
  if (millis() - lastConnectionAttempt < MQTT_RECONNECT_INTERVAL) {
    return isConnected;
  }

  lastConnectionAttempt = millis();

  // Check SIM card status first
  DEBUG_PRINTLN("Checking SIM card status...");
  int simStatus = modem->getSimStatus();
  DEBUG_PRINT("SIM Status: ");
  DEBUG_PRINTLN(simStatus);

  if (simStatus != 1 && simStatus != 5) {
    DEBUG_PRINTLN("✗ SIM card not ready!");
    DEBUG_PRINTLN("  Possible issues:");
    DEBUG_PRINTLN("  - SIM card not inserted");
    DEBUG_PRINTLN("  - SIM card not detected");
    DEBUG_PRINTLN("  - PIN code required");
    return false;
  }
  DEBUG_PRINTLN("✓ SIM card detected");

  // Check signal strength before registration
  int signalBefore = modem->getSignalQuality();
  DEBUG_PRINT("Signal strength: ");
  DEBUG_PRINT(signalBefore);
  DEBUG_PRINTLN(" (0-31, 99=unknown)");

  if (signalBefore == 0 || signalBefore == 99) {
    DEBUG_PRINTLN("✗ No signal! Check antenna connection");
    return false;
  }

  DEBUG_PRINTLN("Waiting for network registration...");
  DEBUG_PRINTLN("This may take 15-60 seconds...");

  // Wait for network
  if (!modem->waitForNetwork(GSM_TIMEOUT)) {
    DEBUG_PRINTLN("✗ Network registration failed!");
    DEBUG_PRINTLN("  Troubleshooting:");
    DEBUG_PRINTLN("  1. Check antenna is properly connected");
    DEBUG_PRINTLN("  2. Verify SIM card is activated");
    DEBUG_PRINTLN("  3. Move to area with better signal");
    DEBUG_PRINTLN("  4. Check SIM card has credit/active plan");
    DEBUG_PRINTLN("  5. Try increasing GSM_TIMEOUT in config.h");

    // Try to get operator name even if not registered
    String cop = modem->getOperator();
    DEBUG_PRINT("  Operator detected: ");
    DEBUG_PRINTLN(cop.length() > 0 ? cop : "None");

    return false;
  }

  DEBUG_PRINTLN("✓ Network registered");

  // Show network info
  String networkOperator = modem->getOperator();
  DEBUG_PRINT("Operator: ");
  DEBUG_PRINTLN(networkOperator);

  // Get signal quality
  int signal = modem->getSignalQuality();
  DEBUG_PRINT("Signal quality: ");
  DEBUG_PRINT(signal);
  DEBUG_PRINTLN("/31");

  DEBUG_PRINT("Connecting to APN: ");
  DEBUG_PRINTLN(APN);

  if (!modem->gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
    DEBUG_PRINTLN("GPRS connection failed!");
    isConnected = false;
    return false;
  }

  DEBUG_PRINTLN("GPRS connected!");
  isConnected = true;

  return true;
}

void GSMModule::disconnectGPRS() {
  if (isInitialized && isConnected) {
    modem->gprsDisconnect();
    isConnected = false;
    DEBUG_PRINTLN("GPRS disconnected");
  }
}

bool GSMModule::isGPRSConnected() {
  if (!isInitialized)
    return false;

  isConnected = modem->isGprsConnected();
  return isConnected;
}

int GSMModule::getSignalQuality() {
  if (!isInitialized)
    return 0;

  return modem->getSignalQuality();
}

TinyGsmClient *GSMModule::getClient() { return client; }

bool GSMModule::sendHTTPPost(const char *url, const char *data) {
  if (!isGPRSConnected()) {
    DEBUG_PRINTLN("No GPRS connection for HTTP!");
    return false;
  }

  DEBUG_PRINT("Sending HTTP POST to: ");
  DEBUG_PRINTLN(url);

  // This is a basic implementation
  // For production, consider using HTTP client library
  // For now, this is a placeholder for future implementation

  DEBUG_PRINTLN("HTTP POST feature - to be implemented with full HTTP client");

  return true;
}

bool GSMModule::isModemReady() {
  if (!isInitialized)
    return false;

  return modem->testAT();
}

bool GSMModule::checkAntennaConnection() {
  if (!isInitialized) {
    DEBUG_PRINTLN("GSM module not initialized");
    return false;
  }

  DEBUG_PRINTLN("\n=== Antenna Connection Test ===");

  // Test 1: Get signal quality
  int signal = modem->getSignalQuality();
  DEBUG_PRINT("Signal Quality: ");
  DEBUG_PRINT(signal);
  DEBUG_PRINT("/31 ");

  if (signal == 99) {
    DEBUG_PRINTLN("(NOT DETECTED - Antenna likely disconnected!)");
    DEBUG_PRINTLN("✗ FAIL: Antenna appears disconnected");
    DEBUG_PRINTLN("  Action: Check physical antenna connection");
    return false;
  } else if (signal == 0) {
    DEBUG_PRINTLN("(NO SIGNAL)");
    DEBUG_PRINTLN("⚠ WARNING: No signal detected");
    DEBUG_PRINTLN("  Possible causes:");
    DEBUG_PRINTLN("  1. Antenna disconnected");
    DEBUG_PRINTLN("  2. Outside cellular coverage area");
    DEBUG_PRINTLN("  3. Inside building blocking signal");
    return false;
  } else if (signal < 10) {
    DEBUG_PRINTLN("(WEAK SIGNAL)");
    DEBUG_PRINTLN("⚠ WARNING: Very weak signal");
    DEBUG_PRINTLN("  Try moving to window or outside");
    return false;
  } else if (signal < 15) {
    DEBUG_PRINTLN("(FAIR SIGNAL)");
    DEBUG_PRINTLN("✓ PASS: Antenna connected, signal could be better");
    return true;
  } else {
    DEBUG_PRINTLN("(GOOD/EXCELLENT SIGNAL)");
    DEBUG_PRINTLN("✓ PASS: Antenna working well");
    return true;
  }
}

void GSMModule::restart() {
  if (isInitialized) {
    DEBUG_PRINTLN("Restarting modem...");
    modem->restart();
    delay(5000);
    isConnected = false;
  }
}
