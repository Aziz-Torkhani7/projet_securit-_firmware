#include "mqtt_client.h"

MQTTClientModule::MQTTClientModule(GSMModule *gsm)
    : gsmModule(gsm), isConnected(false), lastReconnectAttempt(0),
      reconnectInterval(MQTT_RECONNECT_INTERVAL), reconnectAttempts(0) {
  mqttClient = nullptr;
}

MQTTClientModule::~MQTTClientModule() {
  if (mqttClient) {
    delete mqttClient;
  }
}

bool MQTTClientModule::begin() {
  if (!gsmModule) {
    DEBUG_PRINTLN("GSM module not initialized!");
    return false;
  }

  DEBUG_PRINTLN("Initializing MQTT client...");

  // Prevent memory leak - check if already created
  if (mqttClient != nullptr) {
    DEBUG_PRINTLN("MQTT client already initialized, reusing...");
    return true;
  }

  // Create MQTT client with GSM client
  mqttClient = new PubSubClient(*gsmModule->getClient());

  // Set MQTT broker
  mqttClient->setServer(MQTT_BROKER, MQTT_PORT);

  // Set callback for incoming messages
  mqttClient->setCallback(messageCallback);

  // Increase buffer size for larger messages
  mqttClient->setBufferSize(MQTT_BUFFER_SIZE);

  DEBUG_PRINTLN("MQTT client initialized");

  return true;
}

bool MQTTClientModule::connect() {
  if (!mqttClient) {
    DEBUG_PRINTLN("MQTT client not initialized!");
    return false;
  }

  if (!gsmModule->isGPRSConnected()) {
    DEBUG_PRINTLN("No GPRS connection for MQTT!");
    return false;
  }

  DEBUG_PRINT("Connecting to MQTT broker: ");
  DEBUG_PRINTLN(MQTT_BROKER);

  // Attempt connection
  bool connected = false;

  if (strlen(MQTT_USER) > 0) {
    connected = mqttClient->connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
  } else {
    connected = mqttClient->connect(MQTT_CLIENT_ID);
  }

  if (connected) {
    DEBUG_PRINTLN("MQTT connected!");
    isConnected = true;

    // Reset backoff on successful connection
    reconnectAttempts = 0;
    reconnectInterval = MQTT_RECONNECT_INTERVAL;

    // Publish connection status
    publishStatus("{\"status\":\"connected\",\"device\":\"" MQTT_CLIENT_ID
                  "\"}");

    return true;
  } else {
    DEBUG_PRINT("MQTT connection failed, rc=");
    DEBUG_PRINTLN(mqttClient->state());
    isConnected = false;
    return false;
  }
}

void MQTTClientModule::disconnect() {
  if (mqttClient && isConnected) {
    publishStatus("{\"status\":\"disconnecting\"}");
    mqttClient->disconnect();
    isConnected = false;
    DEBUG_PRINTLN("MQTT disconnected");
  }
}

bool MQTTClientModule::isConnectedToBroker() {
  if (!mqttClient)
    return false;

  isConnected = mqttClient->connected();
  return isConnected;
}

bool MQTTClientModule::publishLocation(const String &locationData) {
  if (!isConnectedToBroker()) {
    DEBUG_PRINTLN("Not connected to MQTT broker!");
    return false;
  }

  DEBUG_PRINT("Publishing to topic: ");
  DEBUG_PRINTLN(MQTT_TOPIC_GPS);
  DEBUG_PRINTLN(locationData);

  bool result =
      mqttClient->publish(MQTT_TOPIC_GPS, locationData.c_str(), false);

  if (result) {
    DEBUG_PRINTLN("Location published successfully");
  } else {
    DEBUG_PRINTLN("Failed to publish location");
  }

  return result;
}

bool MQTTClientModule::publishStatus(const String &statusData) {
  if (!isConnectedToBroker()) {
    return false;
  }

  return mqttClient->publish(MQTT_TOPIC_STATUS, statusData.c_str(), false);
}

bool MQTTClientModule::subscribe(const char *topic) {
  if (!isConnectedToBroker()) {
    DEBUG_PRINTLN("Cannot subscribe - not connected!");
    return false;
  }

  DEBUG_PRINT("Subscribing to topic: ");
  DEBUG_PRINTLN(topic);

  return mqttClient->subscribe(topic);
}

void MQTTClientModule::loop() {
  if (mqttClient && isConnected) {
    mqttClient->loop();
  }
}

bool MQTTClientModule::reconnect() {
  // Don't attempt to reconnect too frequently (use exponential backoff)
  unsigned long now = millis();
  if (now - lastReconnectAttempt < reconnectInterval) {
    return false;
  }

  lastReconnectAttempt = now;
  reconnectAttempts++;

  DEBUG_PRINT("Attempting MQTT reconnection (attempt ");
  DEBUG_PRINT(reconnectAttempts);
  DEBUG_PRINTLN(")...");

  // First ensure GPRS is connected
  if (!gsmModule->isGPRSConnected()) {
    DEBUG_PRINTLN("GPRS not connected, attempting to connect...");
    if (!gsmModule->connectGPRS()) {
      DEBUG_PRINTLN("GPRS connection failed!");

      // Increase backoff interval (exponential backoff with cap)
      reconnectInterval = min(reconnectInterval * 2,
                              (unsigned long)MQTT_RECONNECT_MAX_INTERVAL);
      DEBUG_PRINT("Next retry in ");
      DEBUG_PRINT(reconnectInterval / 1000);
      DEBUG_PRINTLN(" seconds");

      return false;
    }
  }

  // Then connect to MQTT
  bool result = connect();

  if (!result) {
    // Increase backoff interval on failure
    reconnectInterval =
        min(reconnectInterval * 2, (unsigned long)MQTT_RECONNECT_MAX_INTERVAL);
    DEBUG_PRINT("Next retry in ");
    DEBUG_PRINT(reconnectInterval / 1000);
    DEBUG_PRINTLN(" seconds");
  }

  return result;
}

void MQTTClientModule::messageCallback(char *topic, byte *payload,
                                       unsigned int length) {
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("]: ");

  for (unsigned int i = 0; i < length; i++) {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINTLN();

  // Handle incoming messages here
  // You can add custom logic based on topic and payload
}
