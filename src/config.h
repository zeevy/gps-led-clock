/**
 * @file config.h
 * @brief Configuration header for GPS Clock with 32x8 LED Matrix Display
 * 
 * This file contains all configuration constants, pin definitions, and hardware
 * setup information for the GPS clock project. It includes LED matrix display
 * settings (fixed to 32x8), timing intervals, timezone configuration, and message definitions.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-06
 * @license MIT
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// LIBRARY INCLUDES
// ============================================================================

#include <Arduino.h>
#include <TinyGPS++.h>
#include <TickTwo.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <RTClib.h>
#include <TimerOne.h>
#include <EEPROM.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

/**
 * @brief Hardware pin connections for the GPS Clock
 * 
 * GPS Module Connections:
 * - GPS RX: Not connected (GPS module only transmits)
 * - GPS TX: Connected to Arduino RX0 (Digital Pin 0)
 * 
 * LED Matrix Connections (4xMAX7219):
 * - DIN (Data In): Connected to Arduino Pin 11 (MOSI)
 * - CLK (Clock): Connected to Arduino Pin 13 (SCK)
 * - CS (Chip Select): Connected to Arduino Pin 10 (SS)
 * - VCC: Connected to 5V
 * - GND: Connected to Ground
 */

// Pin Definitions
#define MATRIX_CS_PIN               10   // LED matrix chip select (CS) pin for SPI

// LED Matrix Display Configuration
// This project is designed specifically for a 32x8 LED matrix using 4x MAX7219 modules
// Matrix size is fixed and cannot be changed without code modifications
#define MATRIX_TOTAL_MODULES_X      4    // Number of LED matrix modules horizontally
#define MATRIX_TOTAL_MODULES_Y      1    // Number of LED matrix modules vertically

// LED Matrix Brightness Levels
const int LED_BRIGHTNESS_HIGH       = 10;  // High brightness level (0-15), used during day time
const int LED_BRIGHTNESS_LOW        = 5;  // Low brightness level (0-15), used during night time

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================

// Update Intervals
#define TIME_UPDATE_INTERVAL_MS     500  // Interval to check for GPS time updates (also affects seconds blinker)
#define DATE_DISPLAY_INTERVAL_MS    (2 * 60 * 1000UL + 30 * 1000UL)  // Interval to display date (2 minutes 30 seconds)

// GPS Signal Management
const unsigned long GPS_SIGNAL_TIMEOUT_MS = 60000UL;  // GPS signal timeout (60 seconds) - rain effect shown if exceeded

// Colon Blink Positions for Time Separator (x, y coordinates)
byte COLON_BLINK_POSITIONS[][2] = {
  {14, 3}, {15, 3},  // Top row of colon
  {14, 4}, {15, 4}   // Bottom row of colon
};

// ============================================================================
// TIMEZONE CONFIGURATION
// ============================================================================

/**
 * @brief Timezone offset configuration
 * 
 * Adjust these values to set your local timezone offset from UTC.
 * The TimeSpan constructor takes: (days, hours, minutes, seconds)
 * 
 * Common timezone examples:
 * - IST (UTC+5:30): DAYS=0, HOURS=5,  MINUTES=30, SECONDS=0
 * - EST (UTC-5:00): DAYS=0, HOURS=-5, MINUTES=0,  SECONDS=0
 * - PST (UTC-8:00): DAYS=0, HOURS=-8, MINUTES=0,  SECONDS=0
 * - GMT (UTC+0:00): DAYS=0, HOURS=0,  MINUTES=0,  SECONDS=0
 * - JST (UTC+9:00): DAYS=0, HOURS=9,  MINUTES=0,  SECONDS=0
 * - CET (UTC+1:00): DAYS=0, HOURS=1,  MINUTES=0,  SECONDS=0
 * - AEST(UTC+10:00):DAYS=0, HOURS=10, MINUTES=0,  SECONDS=0
 * 
 * Note: Use negative values for timezones west of UTC
 */
#define TIMEZONE_OFFSET_DAYS    0    // Days offset from UTC
#define TIMEZONE_OFFSET_HOURS   5    // Hours offset from UTC (positive for east, negative for west)
#define TIMEZONE_OFFSET_MINUTES 30   // Minutes offset from UTC
#define TIMEZONE_OFFSET_SECONDS 0    // Seconds offset from UTC

// ============================================================================
// GPS DISPLAY CONFIGURATION
// ============================================================================

// GPS Location Display Messages
const char* GPS_LAT_PREFIX          = "LAT:";  // Latitude prefix
const char* GPS_LON_PREFIX          = "LON:";  // Longitude prefix
const char* GPS_ALT_PREFIX          = "ALT:";  // Altitude prefix
const char* GPS_ALT_SUFFIX          = "ft";    // Altitude suffix (feet)

// GPS Coordinate Precision Constants
#define GPS_COORD_PRECISION_MULTIPLIER  10000    // For 4 decimal places (~11m accuracy)
#define GPS_ALT_PRECISION_MULTIPLIER    10       // For 1 decimal place

// ============================================================================
// TEXT AND MESSAGES
// ============================================================================

// Display Messages
const char* WELCOME_MESSAGE         = "Arduino 32x8 GPS Clock";  // Welcome message displayed on startup
const char* WAITING_FOR_GPS         = "Waiting for GPS Signal...";  // Message while waiting for GPS signal

// Time Format Messages
const char* FORMAT_12H_MESSAGE      = "12H FORMAT";   // 12-hour format toggle confirmation
const char* FORMAT_24H_MESSAGE      = "24H FORMAT";   // 24-hour format toggle confirmation

// Text Display Configuration
const int TEXT_BUFFER_SIZE          = 75;  // Buffer size for scrolling text display

// Date Formatting Arrays
const char* WEEKDAY_NAMES[7]        = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };  // Abbreviated weekday names
const char* MONTH_NAMES[12]         = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };  // Abbreviated month names

// ============================================================================
// POWER CYCLE DETECTION CONFIGURATION
// ============================================================================

// Legacy EEPROM addresses (deprecated - use EEPROMWearLeveling class)
#define EEPROM_TIME_FORMAT_ADDR    0    // 1 byte: 0=12H, 1=24H (legacy)
#define EEPROM_POWER_CYCLE_ADDR    1    // 4 bytes: power cycle count (legacy)
#define POWER_CYCLE_THRESHOLD      5    // Number of cycles needed to toggle format

// ============================================================================
// EEPROM WEAR-LEVELING CONFIGURATION
// ============================================================================

/**
 * @brief EEPROM wear-leveling system configuration
 * 
 * The wear-leveling system distributes writes across multiple slots to extend
 * EEPROM lifetime. Each data type gets its own slot range with sequence tracking.
 * 
 * Memory Layout:
 * - Addresses 0-9: Reserved for legacy/temporary use
 * - Addresses 10-57: Time format slots (16 slots × 3 bytes = 48 bytes)
 * - Addresses 60-155: Power cycle slots (16 slots × 6 bytes = 96 bytes)
 * - Addresses 156+: Available for future use
 */

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

#define ENABLE_SERIAL_DEBUG         false     // Set to true for debug output, false for production

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct TimeDigits
 * @brief Holds individual digits for hours, minutes, and seconds for display
 * 
 * This structure stores the individual digits of the current time as ASCII characters.
 * It's used for comparing time changes and animating digit transitions.
 */
struct TimeDigits {
  uint8_t hourOnes = ' ';  /**< Hours ones digit (0-9) */
  uint8_t hourTens = ' ';  /**< Hours tens digit (0-1) */
  uint8_t minOnes = ' ';   /**< Minutes ones digit (0-9) */
  uint8_t minTens = ' ';   /**< Minutes tens digit (0-5) */
  uint8_t secOnes = ' ';   /**< Seconds ones digit (0-9) */
  uint8_t secTens = ' ';   /**< Seconds tens digit (0-5) */
};

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * @brief Updates GPS time and displays it on the LED matrix
 * Called periodically by the GPS time update timer
 */
void updateGpsTime();

/**
 * @brief Displays the current date with ordinal suffixes
 * Called periodically by the date display timer
 */
void displayDate();

/**
 * @brief Displays GPS location information (latitude, longitude, altitude)
 * Called after date display to show current GPS coordinates
 */
void displayGpsLocation();

/**
 * @brief Configures LED matrix module positions and rotations
 * Sets up the display based on the selected configuration pattern
 */
void configureLedMatrix();

/**
 * @brief Scrolls text horizontally across the LED matrix display
 * @param text The text message to scroll
 */
void scrollTextHorizontally(const char* text);

/**
 * @brief Creates a vertical sliding animation when time digits change
 * @param previousChar The character that was previously displayed
 * @param newChar The new character to display
 * @param xPosition The X coordinate where the animation occurs
 */
void animateVerticalSlide(char previousChar, char newChar, int xPosition);

/**
 * @brief Returns the appropriate ordinal suffix for a given number
 * @param number The number to get the ordinal suffix for
 * @return Pointer to the ordinal suffix string
 */
const char* getOrdinalSuffix(int number);

/**
 * @brief Extracts individual digits from the current time for display
 * @param digits Reference to TimeDigits struct to store the extracted digits
 */
void extractTimeDigits(TimeDigits& digits);

/**
 * @brief Detects power cycles and toggles time format if threshold is reached
 * Called in setup() to detect rapid power cycles for format switching
 */
void checkPowerCycles();

/**
 * @brief Toggles between 12-hour and 24-hour time format
 * Stores the new format in EEPROM and shows confirmation on display
 */
void toggleTimeFormat();

#endif // CONFIG_H