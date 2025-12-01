#include "gps.h"

GPSModule::GPSModule() : isInitialized(false), lastValidDataTime(0) {
  gpsSerial = new HardwareSerial(1); // Use UART1
}

bool GPSModule::begin() {
  DEBUG_PRINTLN("Initializing GPS module...");

  gpsSerial->begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(100);

  isInitialized = true;
  DEBUG_PRINTLN("GPS module initialized");

  return true;
}

void GPSModule::update() {
  if (!isInitialized)
    return;

  while (gpsSerial->available() > 0) {
    char c = gpsSerial->read();
    gps.encode(c);

// Debug: Print raw NMEA data
#if ENABLE_DEBUG
    DEBUG_SERIAL.write(c);
#endif
  }

  // Update last valid data time if location is valid
  if (gps.location.isValid()) {
    lastValidDataTime = millis();
  }
}

bool GPSModule::hasValidLocation() {
  if (!isInitialized)
    return false;

  return gps.location.isValid() &&
         gps.location.age() < 2000 && // Data less than 2 seconds old
         (millis() - lastValidDataTime) < GPS_TIMEOUT;
}

double GPSModule::getLatitude() {
  if (hasValidLocation()) {
    return gps.location.lat();
  }
  return 0.0;
}

double GPSModule::getLongitude() {
  if (hasValidLocation()) {
    return gps.location.lng();
  }
  return 0.0;
}

double GPSModule::getAltitude() {
  if (gps.altitude.isValid()) {
    return gps.altitude.meters();
  }
  return 0.0;
}

double GPSModule::getSpeed() {
  if (gps.speed.isValid()) {
    return gps.speed.kmph();
  }
  return 0.0;
}

int GPSModule::getSatellites() {
  if (gps.satellites.isValid()) {
    return gps.satellites.value();
  }
  return 0;
}

String GPSModule::getDateTime() {
  if (gps.date.isValid() && gps.time.isValid()) {
    char datetime[32];
    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", gps.date.year(),
            gps.date.month(), gps.date.day(), gps.time.hour(),
            gps.time.minute(), gps.time.second());
    return String(datetime);
  }
  return "Invalid";
}

String GPSModule::getLocationJSON() {
  String json = "{";
  json += "\"latitude\":" + String(getLatitude(), 6) + ",";
  json += "\"longitude\":" + String(getLongitude(), 6) + ",";
  json += "\"altitude\":" + String(getAltitude(), 2) + ",";
  json += "\"speed\":" + String(getSpeed(), 2) + ",";
  json += "\"satellites\":" + String(getSatellites()) + ",";
  json += "\"datetime\":\"" + getDateTime() + "\",";
  json += "\"valid\":" + String(hasValidLocation() ? "true" : "false");
  json += "}";
  return json;
}

bool GPSModule::isReady() {
  return isInitialized && gps.charsProcessed() > 100;
}
