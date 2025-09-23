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
 * @date 2025-01-16
 * @license MIT
 */

#include "GpsStabilityFilter.h"

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
 * This method applies a hybrid filtering approach:
 * 1. Creates a working copy of recent latitude readings
 * 2. Sorts the values to find the median (removes outliers)
 * 3. Calculates average of the middle values for final result
 * 
 * @return Filtered latitude value, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredLatitude() {
  // If insufficient readings available, return raw GPS value
  if (totalReadings < GPS_FILTER_MIN_READINGS || !gpsModule.location.isValid()) {
    return gpsModule.location.lat();
  }

  // Create working copy of readings for sorting (preserve original buffer)  
  float workingBuffer[GPS_FILTER_WINDOW_SIZE];
  uint8_t readingsToProcess = totalReadings;
  
  for (uint8_t i = 0; i < readingsToProcess; i++) {
    workingBuffer[i] = latReadings[i];
  }

  // Sort values to find median and remove outliers
  sortFloatArray(workingBuffer, readingsToProcess);

  // Calculate average of middle values (hybrid median+average approach)
  // Use middle 60% of values to balance stability and responsiveness
  uint8_t startIndex = readingsToProcess > 5 ? readingsToProcess / 5 : 0;  // Skip bottom 20% if enough readings
  uint8_t endIndex = readingsToProcess > 5 ? readingsToProcess - startIndex : readingsToProcess;  // Skip top 20% if enough readings
  
  float sum = 0.0;
  uint8_t count = 0;
  
  for (uint8_t i = startIndex; i < endIndex; i++) {
    sum += workingBuffer[i];
    count++;
  }

  return (count > 0) ? (sum / count) : gpsModule.location.lat();
}

/**
 * @brief Get filtered longitude value using hybrid median+average filter
 * 
 * This method applies the same hybrid filtering approach as getFilteredLatitude()
 * but operates on longitude readings from the GPS filter buffer.
 * 
 * @return Filtered longitude value, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredLongitude() {
  // If insufficient readings available, return raw GPS value
  if (totalReadings < GPS_FILTER_MIN_READINGS || !gpsModule.location.isValid()) {
    return gpsModule.location.lng();
  }

  // Create working copy of readings for sorting (preserve original buffer)
  float workingBuffer[GPS_FILTER_WINDOW_SIZE];
  uint8_t readingsToProcess = totalReadings;
  
  for (uint8_t i = 0; i < readingsToProcess; i++) {
    workingBuffer[i] = lonReadings[i];
  }

  // Sort values to find median and remove outliers
  sortFloatArray(workingBuffer, readingsToProcess);

  // Calculate average of middle values (hybrid median+average approach)
  uint8_t startIndex = readingsToProcess > 5 ? readingsToProcess / 5 : 0;  // Skip bottom 20% if enough readings
  uint8_t endIndex = readingsToProcess > 5 ? readingsToProcess - startIndex : readingsToProcess;  // Skip top 20% if enough readings
  
  float sum = 0.0;
  uint8_t count = 0;
  
  for (uint8_t i = startIndex; i < endIndex; i++) {
    sum += workingBuffer[i];
    count++;
  }

  return (count > 0) ? (sum / count) : gpsModule.location.lng();
}

/**
 * @brief Get filtered altitude value using hybrid median+average filter
 * 
 * This method applies the same hybrid filtering approach as other coordinate
 * filtering functions but operates on altitude readings in feet.
 * 
 * @return Filtered altitude value, or raw GPS value if insufficient data
 */
float GpsStabilityFilter::getFilteredAltitude() {
  // If insufficient readings available, return raw GPS value
  if (totalReadings < GPS_FILTER_MIN_READINGS || !gpsModule.altitude.isValid()) {
    return gpsModule.altitude.feet();
  }

  // Create working copy of readings for sorting (preserve original buffer)
  float workingBuffer[GPS_FILTER_WINDOW_SIZE];
  uint8_t readingsToProcess = totalReadings;
  
  for (uint8_t i = 0; i < readingsToProcess; i++) {
    workingBuffer[i] = altReadings[i];
  }

  // Sort values to find median and remove outliers
  sortFloatArray(workingBuffer, readingsToProcess);

  // Calculate average of middle values (hybrid median+average approach)
  uint8_t startIndex = readingsToProcess > 5 ? readingsToProcess / 5 : 0;  // Skip bottom 20% if enough readings
  uint8_t endIndex = readingsToProcess > 5 ? readingsToProcess - startIndex : readingsToProcess;  // Skip top 20% if enough readings
  
  float sum = 0.0;
  uint8_t count = 0;
  
  for (uint8_t i = startIndex; i < endIndex; i++) {
    sum += workingBuffer[i];
    count++;
  }

  return (count > 0) ? (sum / count) : gpsModule.altitude.feet();
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

/**
 * @brief Helper function to sort an array of float values (for median calculation)
 * 
 * This function performs a simple bubble sort on a small array of float values.
 * Used internally by the GPS filtering functions to calculate median values.
 * 
 * @param arr Array of float values to sort
 * @param n Number of elements in the array
 */
void GpsStabilityFilter::sortFloatArray(float arr[], uint8_t n) {
  for (uint8_t i = 0; i < n - 1; i++) {
    for (uint8_t j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        // Swap elements
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}