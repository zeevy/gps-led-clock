# EEPROM Wear-Leveling System

This directory contains the EEPROM wear-leveling implementation for the GPS LED Clock project.

## Overview

The wear-leveling system extends EEPROM lifetime by distributing writes across multiple slots instead of using fixed addresses. This prevents wear-out of individual EEPROM cells that would occur from repeated writes to the same location.

## Features

- **Slot-based rotation**: Each data type uses 16 slots that are rotated through
- **Sequence tracking**: Each write includes a sequence number to identify the latest data
- **Automatic recovery**: At startup, the system scans all slots to find the most recent valid data
- **Data validation**: Input validation and corruption detection
- **Minimal changes**: Drop-in replacement for direct EEPROM calls

## Memory Layout

The wear-leveling system uses the following EEPROM address ranges:

- **Addresses 0-9**: Reserved for legacy/temporary use
- **Addresses 10-57**: Time format slots (16 slots × 3 bytes = 48 bytes)
  - Each slot: 2 bytes sequence number + 1 byte data
- **Addresses 60-155**: Power cycle slots (16 slots × 6 bytes = 96 bytes)  
  - Each slot: 2 bytes sequence number + 4 bytes data
- **Addresses 156+**: Available for future use

## Usage

### Basic Usage

```cpp
#include "EEPROMWearLeveling.h"

EEPROMWearLeveling eepromWL;

void setup() {
  // Initialize the wear-leveling system
  eepromWL.begin();
  
  // Read time format (0=12H, 1=24H)
  uint8_t format = eepromWL.readTimeFormat();
  
  // Write time format
  eepromWL.writeTimeFormat(1);
  
  // Read power cycle count
  unsigned long count = eepromWL.readPowerCycleCount();
  
  // Write power cycle count
  eepromWL.writePowerCycleCount(count + 1);
}
```

### Migration from Direct EEPROM

Replace direct EEPROM calls:

```cpp
// Before (direct EEPROM):
uint8_t format = EEPROM.read(EEPROM_TIME_FORMAT_ADDR);
EEPROM.write(EEPROM_TIME_FORMAT_ADDR, newFormat);

// After (wear-leveling):
uint8_t format = eepromWL.readTimeFormat();
eepromWL.writeTimeFormat(newFormat);
```

## Configuration

Key configuration constants in `EEPROMWearLeveling.h`:

- `WL_NUM_SLOTS`: Number of slots per data type (default: 16)
- `WL_TIME_FORMAT_START_ADDR`: Starting address for time format slots
- `WL_POWER_CYCLE_START_ADDR`: Starting address for power cycle slots

## Benefits

1. **Extended lifespan**: Distributes ~100k write cycles across 16 slots = ~1.6M total writes per data type
2. **Automatic**: No code changes needed beyond initialization
3. **Robust**: Handles power failures and data corruption gracefully
4. **Efficient**: Quick startup scanning and minimal overhead
5. **Compatible**: Maintains the same API as direct EEPROM access

## Technical Details

### Slot Structure

Each slot contains:
- **Sequence number** (2 bytes): Identifies the order of writes
- **Data payload** (variable): The actual data being stored

### Write Process

1. Move to the next slot (round-robin)
2. Write sequence number + data to the new slot
3. Increment sequence number for next write

### Read Process

1. Scan all slots for the data type
2. Find the slot with the highest sequence number
3. Validate and return the data from that slot

### Sequence Number Handling

- Sequence numbers start at 1 (0 is reserved for invalid/empty slots)
- Handles wraparound when sequence reaches maximum value
- Detects wraparound vs. newer writes using threshold logic

## Files

- `EEPROMWearLeveling.h`: Header file with class definition and configuration
- `EEPROMWearLeveling.cpp`: Implementation of the wear-leveling system
- `EEPROMWearLevelingDemo.ino`: Example sketch demonstrating usage

## Testing

The system includes comprehensive validation:
- Slot rotation verification
- Data persistence across restarts  
- Sequence number handling
- Data validation and corruption detection

See the demo sketch for a working example of the wear-leveling system in action.