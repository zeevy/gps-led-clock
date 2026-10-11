/**
 * @file GpsStabilityFilter.cpp
 * @brief Implementation of the GpsStabilityFilter class for GPS coordinate stability
 * 
 * This file contains the implementation of the GpsStabilityFilter class, which
 * reduces GPS coordinate variations using a hybrid median+average filtering
 * algorithm with circular buffer management.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-23
 * @license MIT
 */

#include "GpsStabilityFilter.h"
#include <ClockLogic.h>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor for GpsStabilityFilter
 * @param gps Reference to the TinyGPSPlus GPS module object
 * 
 * Initializes the GPS stability filter by setting up the member variables
 * and resetting all buffer indices and counters.
 */
GpsStabilityFilter::GpsStabilityFilter(TinyGPSPlus& gps) 
  : gpsModule(gps), currentIndex(0), totalReadings(0) {
  // Arrays are automatically initialized to zero by default
  // No need to explicitly initialize the reading arrays
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

/**
 * @brief Update GPS stability filter with new coordinate readings
 * 
 * This method adds new GPS readings to the stability filter buffers using
 * a FIFO (First In, First Out) approach. It should be called whenever new
 * valid GPS location data is available.
 * 
 * @note Automatically manages buffer overflow using circular buffer approach
 * @note Only updates filter if GPS location and altitude data are valid
 */
void GpsStabilityFilter::update() {
  // Only update filter if GPS location data is valid
  if (!gpsModule.location.isValid() || !gpsModule.altitude.isValid()) {
    return;
  }

  // Add new readings to the FIFO buffers
  latReadings[currentIndex] = gpsModule.location.lat();
  lonReadings[currentIndex] = gpsModule.location.lng();
  altReadings[currentIndex] = gpsModule.altitude.feet();

  // Update indices and counters (circular buffer)
  currentIndex = (currentIndex + 1) % GPS_FILTER_WINDOW_SIZE;
  
  // Track total readings for adaptive window sizing
  if (totalReadings < GPS_FILTER_WINDOW_SIZE) {
    totalReadings++;
  }
}

/**
 * @brief Get filtered latitude value using hybrid median+average filter
 *
 * The mean of the middle readings (see trimmedMean() in ClockLogic), so a
 * few jumps in the GPS position do not show on the display.
 *
 * @return Filtered latitude value, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredLatitude() {
  return filtered(latReadings, gpsModule.location.lat(), gpsModule.location.isValid());
}

/**
 * @brief Get filtered longitude value using hybrid median+average filter
 * @return Filtered longitude value, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredLongitude() {
  return filtered(lonReadings, gpsModule.location.lng(), gpsModule.location.isValid());
}

/**
 * @brief Get filtered altitude value (in feet) using hybrid median+average filter
 * @return Filtered altitude value in feet, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredAltitude() {
  return filtered(altReadings, gpsModule.altitude.feet(), gpsModule.altitude.isValid());
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

/**
 * @brief Trimmed mean of a reading buffer
 *
 * Works on a copy, because trimmedMean() sorts in place and the ring
 * buffer must keep its order.
 *
 * @param readings Ring buffer of one coordinate
 * @param raw Current raw GPS value
 * @param valid Whether the raw GPS value is valid
 * @return Filtered value, or raw when there are too few readings or no fix
 */
float GpsStabilityFilter::filtered(const float readings[], float raw, bool valid) const {
  if (totalReadings < GPS_FILTER_MIN_READINGS || !valid) {
    return raw;
  }

  float workingBuffer[GPS_FILTER_WINDOW_SIZE];
  memcpy(workingBuffer, readings, totalReadings * sizeof(float));
  return trimmedMean(workingBuffer, totalReadings);
}
