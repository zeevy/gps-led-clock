/**
 * @file EEPROMWearLeveling.h
 * @brief EEPROM Wear-Leveling System for GPS Clock
 * 
 * This class implements a slot-based wear-leveling mechanism to extend EEPROM lifetime
 * by distributing writes across multiple slots instead of using fixed addresses.
 * Each data type gets a configurable number of slots with sequence number tracking.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-01-XX
 * @license MIT
 */

#ifndef EEPROM_WEAR_LEVELING_H
#define EEPROM_WEAR_LEVELING_H

#include <Arduino.h>
#include <EEPROM.h>

// ============================================================================
// WEAR-LEVELING CONFIGURATION
// ============================================================================

#define WL_NUM_SLOTS              16    // Number of slots per data type
#define WL_SEQUENCE_SIZE          2     // Size of sequence number (uint16_t)
#define WL_INVALID_SEQUENCE       0     // Reserved sequence number for invalid/empty slots

// EEPROM Address Allocation for Wear-Leveling
#define WL_TIME_FORMAT_START_ADDR 10    // Starting address for time format slots
#define WL_POWER_CYCLE_START_ADDR 60    // Starting address for power cycle slots

// Data type sizes
#define WL_TIME_FORMAT_DATA_SIZE  1     // Size of time format data (byte)
#define WL_POWER_CYCLE_DATA_SIZE  4     // Size of power cycle data (unsigned long)

// Calculated slot sizes (sequence + data)
#define WL_TIME_FORMAT_SLOT_SIZE  (WL_SEQUENCE_SIZE + WL_TIME_FORMAT_DATA_SIZE)   // 3 bytes
#define WL_POWER_CYCLE_SLOT_SIZE  (WL_SEQUENCE_SIZE + WL_POWER_CYCLE_DATA_SIZE)   // 6 bytes

/**
 * @enum WLDataType
 * @brief Enumeration of supported data types for wear-leveling
 */
enum WLDataType {
  WL_TIME_FORMAT = 0,   /**< Time format data (12H/24H) */
  WL_POWER_CYCLE = 1    /**< Power cycle counter data */
};

/**
 * @struct WLSlotInfo
 * @brief Information about a wear-leveling slot configuration
 */
struct WLSlotInfo {
  uint16_t startAddr;   /**< Starting EEPROM address for this data type */
  uint8_t slotSize;     /**< Size of each slot in bytes */
  uint8_t dataSize;     /**< Size of the actual data in bytes */
};

/**
 * @class EEPROMWearLeveling
 * @brief EEPROM wear-leveling implementation using slot-based rotation
 * 
 * This class provides wear-leveling functionality for EEPROM storage by:
 * - Distributing writes across multiple slots per data type
 * - Using sequence numbers to track the latest write
 * - Scanning slots at startup to find the most recent valid data
 * - Providing simple read/write API compatible with existing code
 */
class EEPROMWearLeveling {
private:
  static const WLSlotInfo slotConfig[2];  /**< Configuration for each data type */
  uint16_t nextSequence[2];               /**< Next sequence number for each data type */
  uint8_t currentSlot[2];                 /**< Current slot index for each data type */

  /**
   * @brief Gets the EEPROM address for a specific slot
   * @param dataType The data type identifier
   * @param slotIndex The slot index (0 to WL_NUM_SLOTS-1)
   * @return EEPROM address of the slot
   */
  uint16_t getSlotAddress(WLDataType dataType, uint8_t slotIndex);

  /**
   * @brief Reads sequence number from a slot
   * @param dataType The data type identifier
   * @param slotIndex The slot index
   * @return Sequence number (0 if invalid/empty)
   */
  uint16_t readSlotSequence(WLDataType dataType, uint8_t slotIndex);

  /**
   * @brief Writes data to a specific slot with sequence number
   * @param dataType The data type identifier
   * @param slotIndex The slot index
   * @param sequence The sequence number to write
   * @param data Pointer to data to write
   */
  void writeSlot(WLDataType dataType, uint8_t slotIndex, uint16_t sequence, const void* data);

  /**
   * @brief Reads data from a specific slot
   * @param dataType The data type identifier
   * @param slotIndex The slot index
   * @param data Pointer to buffer for reading data
   * @return true if slot contains valid data, false otherwise
   */
  bool readSlot(WLDataType dataType, uint8_t slotIndex, void* data);

  /**
   * @brief Finds the slot with the latest (highest) sequence number
   * @param dataType The data type identifier
   * @return Slot index with latest data, or 0 if no valid data found
   */
  uint8_t findLatestSlot(WLDataType dataType);

public:
  /**
   * @brief Constructor - initializes the wear-leveling system
   */
  EEPROMWearLeveling();

  /**
   * @brief Initializes wear-leveling system by scanning existing data
   * Must be called once after construction, typically in setup()
   */
  void begin();

  /**
   * @brief Reads time format data with wear-leveling
   * @return Time format value (0=12H, 1=24H), or 0 if no valid data
   */
  uint8_t readTimeFormat();

  /**
   * @brief Writes time format data with wear-leveling
   * @param format Time format value (0=12H, 1=24H)
   */
  void writeTimeFormat(uint8_t format);

  /**
   * @brief Reads power cycle counter with wear-leveling
   * @return Power cycle count value, or 0 if no valid data
   */
  unsigned long readPowerCycleCount();

  /**
   * @brief Writes power cycle counter with wear-leveling
   * @param count Power cycle count value
   */
  void writePowerCycleCount(unsigned long count);

  /**
   * @brief Gets debug information about current slot usage
   * @param dataType The data type identifier
   * @param currentSlotOut Current slot index being used
   * @param sequenceOut Current sequence number
   */
  void getDebugInfo(WLDataType dataType, uint8_t* currentSlotOut, uint16_t* sequenceOut);
};

#endif // EEPROM_WEAR_LEVELING_H