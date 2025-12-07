#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "config.h"
#include "gsm.h"
#include <Arduino.h>
#include <PubSubClient.h>

class MQTTClientModule {
private:
  PubSubClient *mqttClient;
  GSMModule *gsmModule;
  bool isConnected;
  unsigned long lastReconnectAttempt;
  unsigned long reconnectInterval; // Current backoff interval
  int reconnectAttempts;           // Track consecutive failures

  // MQTT callback for incoming messages
  static void messageCallback(char *topic, byte *payload, unsigned int length);

public:
  MQTTClientModule(GSMModule *gsm);
  ~MQTTClientModule();

  // Initialize MQTT client
  bool begin();

  // Connect to MQTT broker
  bool connect();

  // Disconnect from MQTT broker
  void disconnect();

  // Check if connected to MQTT broker
  bool isConnectedToBroker();

  // Publish GPS location data
  bool publishLocation(const String &locationData);

  // Publish status message
  bool publishStatus(const String &statusData);

  // Subscribe to a topic
  bool subscribe(const char *topic);

  // Process MQTT messages (call in loop)
  void loop();

  // Attempt to reconnect if disconnected
  bool reconnect();
};

#endif // MQTT_CLIENT_H
