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
  const unsigned long GPS_TIMEOUT = 10000; // 10 seconds timeout

public:
  GPSModule();

  // Initialize GPS module
  bool begin();

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

  // Get GPS date/time
  String getDateTime();

  // Get location as JSON string
  String getLocationJSON();

  // Check if GPS module is responding
  bool isReady();
};

#endif // GPS_H
