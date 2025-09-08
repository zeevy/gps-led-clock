/**
 * @file EEPROMWearLeveling.cpp
 * @brief Implementation of EEPROM Wear-Leveling System
 * 
 * This file implements the slot-based wear-leveling mechanism that distributes
 * EEPROM writes across multiple slots to extend the lifetime of EEPROM cells.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-01-XX
 * @license MIT
 */

#include "EEPROMWearLeveling.h"

// ============================================================================
// STATIC CONFIGURATION DATA
// ============================================================================

/**
 * @brief Configuration table for each data type's slot layout
 */
const WLSlotInfo EEPROMWearLeveling::slotConfig[2] = {
  // WL_TIME_FORMAT
  {
    .startAddr = WL_TIME_FORMAT_START_ADDR,
    .slotSize = WL_TIME_FORMAT_SLOT_SIZE,
    .dataSize = WL_TIME_FORMAT_DATA_SIZE
  },
  // WL_POWER_CYCLE  
  {
    .startAddr = WL_POWER_CYCLE_START_ADDR,
    .slotSize = WL_POWER_CYCLE_SLOT_SIZE,
    .dataSize = WL_POWER_CYCLE_DATA_SIZE
  }
};

// ============================================================================
// CONSTRUCTOR AND INITIALIZATION
// ============================================================================

/**
 * @brief Constructor - initializes internal state
 */
EEPROMWearLeveling::EEPROMWearLeveling() {
  // Initialize sequence numbers and current slots
  for (int i = 0; i < 2; i++) {
    nextSequence[i] = 1;  // Start with sequence 1 (0 is reserved for invalid)
    currentSlot[i] = 0;   // Start with slot 0
  }
}

/**
 * @brief Initializes wear-leveling by scanning existing EEPROM data
 * 
 * This function scans all slots for each data type to find the latest
 * valid data and initializes the internal state accordingly.
 */
void EEPROMWearLeveling::begin() {
  for (int dataType = 0; dataType < 2; dataType++) {
    // Find the slot with the latest sequence number
    uint8_t latestSlot = findLatestSlot((WLDataType)dataType);
    uint16_t latestSequence = readSlotSequence((WLDataType)dataType, latestSlot);
    
    // Initialize state based on found data
    currentSlot[dataType] = latestSlot;
    if (latestSequence == WL_INVALID_SEQUENCE) {
      // No valid data found, start fresh
      nextSequence[dataType] = 1;
    } else {
      // Set next sequence number, handling wraparound
      nextSequence[dataType] = (latestSequence == 0xFFFF) ? 1 : latestSequence + 1;
    }
  }
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================

/**
 * @brief Calculates the EEPROM address for a specific slot
 */
uint16_t EEPROMWearLeveling::getSlotAddress(WLDataType dataType, uint8_t slotIndex) {
  const WLSlotInfo& config = slotConfig[dataType];
  return config.startAddr + (slotIndex * config.slotSize);
}

/**
 * @brief Reads the sequence number from a slot
 */
uint16_t EEPROMWearLeveling::readSlotSequence(WLDataType dataType, uint8_t slotIndex) {
  if (slotIndex >= WL_NUM_SLOTS) return WL_INVALID_SEQUENCE;
  
  uint16_t addr = getSlotAddress(dataType, slotIndex);
  uint16_t sequence;
  EEPROM.get(addr, sequence);
  
  // Return 0 for uninitialized EEPROM (0xFF values)
  if (sequence == 0xFFFF) return WL_INVALID_SEQUENCE;
  
  return sequence;
}

/**
 * @brief Writes data to a specific slot with sequence number
 */
void EEPROMWearLeveling::writeSlot(WLDataType dataType, uint8_t slotIndex, uint16_t sequence, const void* data) {
  if (slotIndex >= WL_NUM_SLOTS || data == nullptr) return;
  
  uint16_t addr = getSlotAddress(dataType, slotIndex);
  const WLSlotInfo& config = slotConfig[dataType];
  
  // Write sequence number first
  EEPROM.put(addr, sequence);
  
  // Write data after sequence number
  for (uint8_t i = 0; i < config.dataSize; i++) {
    EEPROM.write(addr + WL_SEQUENCE_SIZE + i, *((uint8_t*)data + i));
  }
}

/**
 * @brief Reads data from a specific slot
 */
bool EEPROMWearLeveling::readSlot(WLDataType dataType, uint8_t slotIndex, void* data) {
  if (slotIndex >= WL_NUM_SLOTS || data == nullptr) return false;
  
  uint16_t sequence = readSlotSequence(dataType, slotIndex);
  if (sequence == WL_INVALID_SEQUENCE) return false;
  
  uint16_t addr = getSlotAddress(dataType, slotIndex);
  const WLSlotInfo& config = slotConfig[dataType];
  
  // Read data from after the sequence number
  for (uint8_t i = 0; i < config.dataSize; i++) {
    *((uint8_t*)data + i) = EEPROM.read(addr + WL_SEQUENCE_SIZE + i);
  }
  
  return true;
}

/**
 * @brief Finds the slot with the highest sequence number (most recent data)
 */
uint8_t EEPROMWearLeveling::findLatestSlot(WLDataType dataType) {
  uint16_t highestSequence = WL_INVALID_SEQUENCE;
  uint8_t latestSlot = 0;
  
  for (uint8_t slot = 0; slot < WL_NUM_SLOTS; slot++) {
    uint16_t sequence = readSlotSequence(dataType, slot);
    
    // Handle sequence number wraparound - assume newer sequence if significantly different
    if (sequence != WL_INVALID_SEQUENCE) {
      if (highestSequence == WL_INVALID_SEQUENCE || 
          sequence > highestSequence || 
          (highestSequence > 0xF000 && sequence < 0x1000)) {
        highestSequence = sequence;
        latestSlot = slot;
      }
    }
  }
  
  return latestSlot;
}

// ============================================================================
// PUBLIC API METHODS
// ============================================================================

/**
 * @brief Reads time format with wear-leveling
 */
uint8_t EEPROMWearLeveling::readTimeFormat() {
  uint8_t latestSlot = findLatestSlot(WL_TIME_FORMAT);
  uint8_t format;
  
  if (readSlot(WL_TIME_FORMAT, latestSlot, &format)) {
    // Validate the format value
    if (format == 0 || format == 1) {
      return format;
    }
  }
  
  // Return default if no valid data found
  return 0; // Default to 12-hour format
}

/**
 * @brief Writes time format with wear-leveling
 */
void EEPROMWearLeveling::writeTimeFormat(uint8_t format) {
  // Validate input
  if (format != 0 && format != 1) return;
  
  // Move to next slot for wear-leveling
  currentSlot[WL_TIME_FORMAT] = (currentSlot[WL_TIME_FORMAT] + 1) % WL_NUM_SLOTS;
  
  // Write to the new slot
  writeSlot(WL_TIME_FORMAT, currentSlot[WL_TIME_FORMAT], nextSequence[WL_TIME_FORMAT], &format);
  
  // Increment sequence number for next write
  nextSequence[WL_TIME_FORMAT]++;
  if (nextSequence[WL_TIME_FORMAT] == WL_INVALID_SEQUENCE || nextSequence[WL_TIME_FORMAT] == 0xFFFF) {
    nextSequence[WL_TIME_FORMAT] = 1; // Wrap around, avoid reserved values
  }
}

/**
 * @brief Reads power cycle count with wear-leveling
 */
unsigned long EEPROMWearLeveling::readPowerCycleCount() {
  uint8_t latestSlot = findLatestSlot(WL_POWER_CYCLE);
  unsigned long count;
  
  if (readSlot(WL_POWER_CYCLE, latestSlot, &count)) {
    // Basic validation - should be reasonable value
    if (count < 0xFFFFFF00UL) { // Allow up to ~4 billion cycles
      return count;
    }
  }
  
  // Return default if no valid data found
  return 0;
}

/**
 * @brief Writes power cycle count with wear-leveling
 */
void EEPROMWearLeveling::writePowerCycleCount(unsigned long count) {
  // Move to next slot for wear-leveling
  currentSlot[WL_POWER_CYCLE] = (currentSlot[WL_POWER_CYCLE] + 1) % WL_NUM_SLOTS;
  
  // Write to the new slot
  writeSlot(WL_POWER_CYCLE, currentSlot[WL_POWER_CYCLE], nextSequence[WL_POWER_CYCLE], &count);
  
  // Increment sequence number for next write
  nextSequence[WL_POWER_CYCLE]++;
  if (nextSequence[WL_POWER_CYCLE] == WL_INVALID_SEQUENCE || nextSequence[WL_POWER_CYCLE] == 0xFFFF) {
    nextSequence[WL_POWER_CYCLE] = 1; // Wrap around, avoid reserved values
  }
}

/**
 * @brief Gets debug information about current wear-leveling state
 */
void EEPROMWearLeveling::getDebugInfo(WLDataType dataType, uint8_t* currentSlotOut, uint16_t* sequenceOut) {
  if (dataType < 2 && currentSlotOut && sequenceOut) {
    *currentSlotOut = currentSlot[dataType];
    *sequenceOut = nextSequence[dataType] - 1; // Return current sequence, not next
  }
}