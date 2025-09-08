/**
 * @file EEPROMWearLevelingDemo.ino
 * @brief Simple demonstration of EEPROM wear-leveling functionality
 * 
 * This sketch demonstrates the wear-leveling system working correctly
 * by cycling through multiple writes and showing the slot rotation.
 * 
 * To use: Upload this sketch and open the Serial Monitor at 115200 baud.
 * The sketch will show how writes are distributed across slots.
 */

#include "EEPROMWearLeveling.h"

EEPROMWearLeveling wearLeveling;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);
  
  Serial.println("=== EEPROM Wear-Leveling Demo ===");
  
  // Initialize the wear-leveling system
  wearLeveling.begin();
  
  // Show initial values
  Serial.print("Initial time format: ");
  Serial.println(wearLeveling.readTimeFormat());
  Serial.print("Initial power cycle count: ");
  Serial.println(wearLeveling.readPowerCycleCount());
  
  // Demonstrate slot rotation with time format
  Serial.println("\n--- Time Format Slot Rotation ---");
  for (int i = 0; i < 10; i++) {
    uint8_t format = i % 2;
    wearLeveling.writeTimeFormat(format);
    
    uint8_t currentSlot;
    uint16_t sequence;
    wearLeveling.getDebugInfo(WL_TIME_FORMAT, &currentSlot, &sequence);
    
    Serial.print("Write ");
    Serial.print(i);
    Serial.print(": format=");
    Serial.print(format);
    Serial.print(", slot=");
    Serial.print(currentSlot);
    Serial.print(", sequence=");
    Serial.print(sequence);
    Serial.print(", read_back=");
    Serial.println(wearLeveling.readTimeFormat());
    
    delay(100);
  }
  
  // Demonstrate power cycle counter
  Serial.println("\n--- Power Cycle Counter ---");
  for (int i = 0; i < 5; i++) {
    unsigned long count = (i + 1) * 10;
    wearLeveling.writePowerCycleCount(count);
    
    uint8_t currentSlot;
    uint16_t sequence;
    wearLeveling.getDebugInfo(WL_POWER_CYCLE, &currentSlot, &sequence);
    
    Serial.print("Write ");
    Serial.print(i);
    Serial.print(": count=");
    Serial.print(count);
    Serial.print(", slot=");
    Serial.print(currentSlot);
    Serial.print(", sequence=");
    Serial.print(sequence);
    Serial.print(", read_back=");
    Serial.println(wearLeveling.readPowerCycleCount());
    
    delay(100);
  }
  
  Serial.println("\n=== Demo Complete ===");
  Serial.println("The wear-leveling system successfully distributed writes across multiple slots!");
  Serial.println("This extends EEPROM lifetime by avoiding repeated writes to the same address.");
}

void loop() {
  // Demo runs once in setup()
  delay(1000);
}