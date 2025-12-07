#ifndef GSM_H
#define GSM_H

#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include "config.h"
#include <TinyGsmClient.h>

class GSMModule {
private:
  TinyGsm *modem;
  TinyGsmClient *client;
  HardwareSerial *gsmSerial;
  bool isInitialized;
  bool isConnected;

  unsigned long lastConnectionAttempt;

public:
  GSMModule();
  ~GSMModule();

  // Initialize GSM module with dedicated UART
  bool begin(HardwareSerial *serial);

  // Hardware reset using reset pin
  void hardwareReset();

  // Connect to GPRS network
  bool connectGPRS();

  // Disconnect from GPRS
  void disconnectGPRS();

  // Check if connected to GPRS
  bool isGPRSConnected();

  // Get signal quality (0-31, 99=unknown)
  int getSignalQuality();

  // Get TinyGsmClient for MQTT/HTTP
  TinyGsmClient *getClient();

  // Send HTTP POST request (for API)
  bool sendHTTPPost(const char *url, const char *data);

  // Check if modem is responding
  bool isModemReady();

  // Check antenna connection and signal quality
  bool checkAntennaConnection();

  // Restart modem
  void restart();
};

#endif // GSM_H
