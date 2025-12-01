#include "gsm.h"

GSMModule::GSMModule()
    : isInitialized(false), isConnected(false), lastConnectionAttempt(0) {
  gsmSerial = new HardwareSerial(2); // Use UART2
}

GSMModule::~GSMModule() {
  delete client;
  delete modem;
  delete gsmSerial;
}

bool GSMModule::begin() {
  DEBUG_PRINTLN("Initializing GSM module...");

  // Initialize serial communication with SIM800L
  gsmSerial->begin(SIM800L_BAUD, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  delay(3000);

  // Create modem instance
  modem = new TinyGsm(*gsmSerial);
  client = new TinyGsmClient(*modem);

  // Restart modem
  DEBUG_PRINTLN("Restarting modem...");
  modem->restart();
  delay(5000);

  // Check if modem is responding
  String modemInfo = modem->getModemInfo();
  DEBUG_PRINT("Modem Info: ");
  DEBUG_PRINTLN(modemInfo);

  if (modemInfo.length() == 0) {
    DEBUG_PRINTLN("Modem not responding!");
    return false;
  }

  isInitialized = true;
  DEBUG_PRINTLN("GSM module initialized");

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

  DEBUG_PRINTLN("Waiting for network...");
  if (!modem->waitForNetwork(GSM_TIMEOUT)) {
    DEBUG_PRINTLN("Network registration failed!");
    return false;
  }

  DEBUG_PRINTLN("Network registered");

  // Get signal quality
  int signal = modem->getSignalQuality();
  DEBUG_PRINT("Signal quality: ");
  DEBUG_PRINTLN(signal);

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

void GSMModule::restart() {
  if (isInitialized) {
    DEBUG_PRINTLN("Restarting modem...");
    modem->restart();
    delay(5000);
    isConnected = false;
  }
}
