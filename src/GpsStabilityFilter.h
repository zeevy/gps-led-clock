/**
 * @file GpsStabilityFilter.h
 * @brief GPS coordinates stability filter for LED matrix GPS clock
 * 
 * This file contains the GpsStabilityFilter class that implements a hybrid
 * median+average filtering system to reduce GPS coordinate variations that
 * occur naturally even when the device is stationary.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-23
 * @license MIT
 */

#ifndef GPS_STABILITY_FILTER_H
#define GPS_STABILITY_FILTER_H

#include <Arduino.h>
#include <TinyGPS++.h>

// ============================================================================
// CONSTANTS
// ============================================================================

/** Number of GPS readings to store for filtering (12 readings ≈ 12 seconds) */
#define GPS_FILTER_WINDOW_SIZE          12

/** Minimum readings needed before filtering (adaptive window) */
#define GPS_FILTER_MIN_READINGS         3

// ============================================================================
// GPS STABILITY FILTER CLASS
// ============================================================================

/**
 * @class GpsStabilityFilter
 * @brief Manages GPS coordinate stability filtering using hybrid median+average approach
 * 
 * This class reduces GPS coordinate variations by collecting recent readings and
 * applying a hybrid filtering algorithm that combines median filtering (to remove
 * outliers) with averaging (for stability). The filter uses a circular buffer
 * to maintain the most recent GPS readings.
 * 
 * Features:
 * - Hybrid median + average filtering algorithm
 * - Circular buffer for efficient memory management  
 * - Adaptive window sizing for quick startup
 * - Support for negative coordinates (Southern/Western hemispheres)
 * - Memory efficient (146 bytes total)
 * 
 * Algorithm:
 * 1. Collect last N GPS readings in circular buffers
 * 2. Sort readings to find median values
 * 3. Remove top/bottom 20% outliers (if enough readings)
 * 4. Average remaining middle values for final result
 * 
 * @note Memory usage: 12 readings × 3 coordinates × 4 bytes + 2 counters = 146 bytes
 * @note This represents only 7% of Arduino Nano's 2KB SRAM
 */
class GpsStabilityFilter {
public:
  // ========================================================================
  // CONSTRUCTOR
  // ========================================================================
  
  /**
   * @brief Constructor for GpsStabilityFilter
   * @param gps Reference to the TinyGPSPlus GPS module object
   */
  GpsStabilityFilter(TinyGPSPlus& gps);
  
  // ========================================================================
  // PUBLIC METHODS
  // ========================================================================
  
  /**
   * @brief Update GPS stability filter with new coordinate readings
   * 
   * This method adds new GPS readings to the stability filter buffers using
   * a FIFO (First In, First Out) approach. Should be called whenever new
   * valid GPS location data is available.
   * 
   * @note Automatically manages buffer overflow using circular buffer approach
   * @note Only updates filter if GPS location and altitude data are valid
   */
  void update();
  
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
  float getFilteredLatitude();
  
  /**
   * @brief Get filtered longitude value using hybrid median+average filter
   * 
   * This method applies the same hybrid filtering approach as getFilteredLatitude()
   * but operates on longitude readings from the GPS filter buffer.
   * 
   * @return Filtered longitude value, or raw GPS value if insufficient data
   */
  float getFilteredLongitude();
  
  /**
   * @brief Get filtered altitude value using hybrid median+average filter
   * 
   * This method applies the same hybrid filtering approach as other coordinate
   * filtering functions but operates on altitude readings in feet.
   * 
   * @return Filtered altitude value, or raw GPS value if insufficient data
   */
  float getFilteredAltitude();
  
  /**
   * @brief Get the total number of readings collected so far
   * @return Number of readings in the filter buffer (0-12)
   */
  uint8_t getTotalReadings() const { return totalReadings; }
  
private:
  // ========================================================================
  // MEMBER VARIABLES
  // ========================================================================
  
  /** Reference to the GPS module object */
  TinyGPSPlus& gpsModule;
  
  /** Latitude readings buffer (circular buffer) */
  float latReadings[GPS_FILTER_WINDOW_SIZE];
  
  /** Longitude readings buffer (circular buffer) */
  float lonReadings[GPS_FILTER_WINDOW_SIZE];
  
  /** Altitude readings buffer (circular buffer) */
  float altReadings[GPS_FILTER_WINDOW_SIZE];
  
  /** Current insertion index for FIFO circular buffer */
  uint8_t currentIndex;
  
  /** Total readings collected so far (0 to GPS_FILTER_WINDOW_SIZE) */
  uint8_t totalReadings;
  
  // ========================================================================
  // PRIVATE METHODS
  // ========================================================================
  
  /**
   * @brief Helper function to sort an array of float values (for median calculation)
   * 
   * This function performs a simple bubble sort on a small array of float values.
   * Used internally by the GPS filtering functions to calculate median values.
   * 
   * @param arr Array of float values to sort
   * @param n Number of elements in the array
   */
  void sortFloatArray(float arr[], uint8_t n);
};

#endif // GPS_STABILITY_FILTER_H