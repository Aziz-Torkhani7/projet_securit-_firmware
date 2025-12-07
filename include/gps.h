#ifndef GPS_H
#define GPS_H

#include "config.h"
#include <Arduino.h>
#include <TinyGPSPlus.h>

class GPSModule {
private:
  TinyGPSPlus gps;
  HardwareSerial *gpsSerial;
  bool isInitialized;

  unsigned long lastValidDataTime;

public:
  GPSModule();

  // Initialize GPS module with dedicated UART
  bool begin(HardwareSerial *serial);

  // Update GPS data (call frequently in loop)
  void update();

  // Check if GPS has valid location fix
  bool hasValidLocation();

  // Get latitude
  double getLatitude();

  // Get longitude
  double getLongitude();

  // Get altitude in meters
  double getAltitude();

  // Get speed in km/h
  double getSpeed();

  // Get number of satellites
  int getSatellites();

  // Get number of characters processed by GPS
  unsigned long getCharsProcessed();

  // Get GPS date/time
  String getDateTime();

  // Get location as JSON string
  String getLocationJSON();

  // Check if GPS module is responding
  bool isReady();
};

#endif // GPS_H
