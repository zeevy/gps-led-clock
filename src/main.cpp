/**
 * @file main.cpp
 * @brief GPS Clock with 4xMAX7219 LED Matrix Display
 * 
 * This Arduino project creates a GPS-synchronized clock that displays time on a 32x8 LED matrix.
 * The project is specifically designed for 4x MAX7219 modules arranged horizontally.
 * Features include:
 * - GPS time synchronization with timezone adjustment
 * - Animated time display with vertical slide transitions
 * - Date display with ordinal suffixes
 * - Rain effect animation when GPS signal is lost
 * - Automatic brightness adjustment for day/night
 * - Scrolling text messages
 * - Fixed 32x8 matrix display (4x MAX7219 modules)
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-06
 * @license MIT
 */

#include <config.h>
#include "RainEffect.h"
#include "GpsStabilityFilter.h"

// ============================================================================
// CONSTANTS AND CONFIGURATION
// ============================================================================

// Space between characters in pixels for text scrolling
const int CHAR_SPACING = 1;

// Total character width including spacing for text scrolling
const int CHAR_WIDTH = 5 + CHAR_SPACING;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// ----------------------------------------------------------------------------
// GPS AND SIGNAL TRACKING
// ----------------------------------------------------------------------------
TinyGPSPlus gpsModule;                    // GPS module interface
bool wasShowingRainEffect = false;        // Track previous rain effect state for efficient screen clearing
GpsStabilityFilter gpsFilter(gpsModule);  // GPS coordinates stability filter

// GPS Baud Rate Auto-Detection Variables
bool gpsBaudDetectionComplete = false;    // Flag to indicate if baud rate detection is complete
unsigned long currentGpsBaudRate = 115200; // Current GPS baud rate (starts with default)
int currentBaudRateIndex = 0;             // Index in GPS_BAUD_RATES array for current test
unsigned long baudTestStartTime = 0;      // Start time for current baud rate test
int validGpsSentencesCount = 0;           // Count of valid GPS sentences received during test

// ----------------------------------------------------------------------------
// TIME AND DATE MANAGEMENT
// ----------------------------------------------------------------------------
DateTime currentDateTime;                 // Current date and time from GPS
TimeDigits currentTimeDigits;             // Current time digits for display
TimeDigits previousTimeDigits;            // Previous time digits for animation comparison
bool is24Hour = false;                    // Time format cache (read once in setup for performance)
bool toggleBlinker = false;               // Controls colon blinking state in time display

// ----------------------------------------------------------------------------
// DISPLAY AND ANIMATION
// ----------------------------------------------------------------------------
Max72xxPanel ledMatrix = Max72xxPanel(MATRIX_CS_PIN, MATRIX_TOTAL_MODULES_X, MATRIX_TOTAL_MODULES_Y);
RainEffect rainEffect(ledMatrix);         // Rain effect animation object
char textScrollBuffer[TEXT_BUFFER_SIZE];  // Buffer for scrolling text display

// ----------------------------------------------------------------------------
// TIMERS
// ----------------------------------------------------------------------------
TickTwo gpsTimeUpdateTicker(updateGpsTime, TIME_UPDATE_INTERVAL_MS);  // Timer for GPS time updates
TickTwo dateDisplayTicker(displayDate, DATE_DISPLAY_INTERVAL_MS);     // Timer for date display

/**
 * @brief Arduino setup function - initializes the GPS clock system
 * 
 * This function performs the following initialization steps:
 * 1. Initializes serial communication for debugging
 * 2. Configures the LED matrix display
 * 3. Runs a startup animation
 * 4. Displays welcome message
 * 5. Starts the GPS time update and date display timers
 * 
 * @note This function runs once when the Arduino starts up
 */
void setup() {
  // Load saved GPS baud rate or use default (115200)
  currentGpsBaudRate = loadGpsBaudRate();
  
  // Check if we have a valid saved baud rate
  if (currentGpsBaudRate != 115200) {
    // We have a previously detected baud rate, mark detection as complete
    gpsBaudDetectionComplete = true;
  }
  
  Serial.begin(currentGpsBaudRate);
  while (!Serial) delay(100);

  #if ENABLE_SERIAL_DEBUG
  Serial.print("GPS Clock starting with baud rate: ");
  Serial.println(currentGpsBaudRate);
  #endif

  // Initialize LED matrix with low brightness
  ledMatrix.setIntensity(LED_BRIGHTNESS_LOW);
  ledMatrix.fillScreen(LOW);
  ledMatrix.write();
  configureLedMatrix();

  // Read time format from EEPROM once
  uint8_t format = EEPROM.read(EEPROM_TIME_FORMAT_ADDR);
  is24Hour = (format == 1);

  // Detect power cycles for time format switching
  checkPowerCycles();

  // Fun startup animation: randomly light up LEDs (increased delay for power cycle detection)
  randomSeed(analogRead(A0));
  for (int i = 0; i < ledMatrix.width() * ledMatrix.height(); i++) {
    ledMatrix.drawPixel(random(ledMatrix.width()), random(ledMatrix.height()), HIGH);
    ledMatrix.write();
    delay(5);
  }

  // Display welcome message and start timers
  scrollTextHorizontally(WELCOME_MESSAGE);

  // Reset power cycle counter after welcome message
  EEPROM.put(EEPROM_POWER_CYCLE_ADDR, (unsigned long)0);

  gpsTimeUpdateTicker.start();
  dateDisplayTicker.start();
}

void loop() {
  // Only read GPS data normally if baud rate detection is complete
  if (gpsBaudDetectionComplete) {
    while (Serial.available()){
      char receivedChar = Serial.read();
      gpsModule.encode(receivedChar);
    }
  }

  if (validGpsDateTime()) {
    // GPS signal good - clear screen if transitioning from rain effect
    // This prevents rain drops from being visible with time display
    if (wasShowingRainEffect) {
      ledMatrix.fillScreen(LOW);
      previousTimeDigits = TimeDigits();
      wasShowingRainEffect = false;
    }

    gpsTimeUpdateTicker.update();
    dateDisplayTicker.update();
  }else{
    // GPS signal lost - show rain effect and detect GPS baud rate
    if (!rainEffect.isInitialized()) rainEffect.initialize();
    rainEffect.update();
    rainEffect.render();
    wasShowingRainEffect = true;
    
    // Perform GPS baud rate detection during rain effect
    detectGpsBaudRate();
  }
}

/**
 * @brief Checks if the GPS date and time are valid and recent.
 * 
 * This function validates that the GPS location, date, and time are all valid,
 * and that the GPS data is not stale (i.e., received within the configured timeout).
 * 
 * @return true if GPS location, date, and time are valid and recent; false otherwise.
 */
bool validGpsDateTime() {
  return gpsModule.location.isValid() &&
         gpsModule.location.age() < GPS_SIGNAL_TIMEOUT_MS &&
         gpsModule.date.isValid() &&
         gpsModule.time.isValid();
}

/**
 * @brief Updates GPS time and displays it on the LED matrix
 * 
 * This function is called periodically by the GPS time update timer. It:
 * 1. Validates GPS data availability
 * 2. Converts GPS time to local timezone (configurable)
 * 3. Extracts individual time digits
 * 4. Animates digit changes with vertical slide effect
 * 5. Displays PM indicator and blinking colon
 * 
 * @note This function is called every TIME_UPDATE_INTERVAL_MS milliseconds
 * @note Timezone offset is configurable in config.h via TIMEZONE_OFFSET_* defines
 */
void updateGpsTime() {
  if (gpsModule.date.isValid() && gpsModule.time.isValid()) {
    // Create DateTime object from GPS data with timezone adjustment
    // Timezone offset is configured in config.h
    currentDateTime = DateTime(
      gpsModule.date.year(), gpsModule.date.month(), gpsModule.date.day(),
      gpsModule.time.hour(), gpsModule.time.minute(), gpsModule.time.second()
    ) + TimeSpan(TIMEZONE_OFFSET_DAYS, TIMEZONE_OFFSET_HOURS, TIMEZONE_OFFSET_MINUTES, TIMEZONE_OFFSET_SECONDS);

    // Extract individual digits for display
    extractTimeDigits(currentTimeDigits);

    // Animate digit changes with vertical slide effect
    if (currentTimeDigits.minOnes != previousTimeDigits.minOnes) {
      animateVerticalSlide(previousTimeDigits.minOnes, currentTimeDigits.minOnes, 25);
    }

    if (currentTimeDigits.minTens != previousTimeDigits.minTens) {
      animateVerticalSlide(previousTimeDigits.minTens, currentTimeDigits.minTens, 18);
    }

    if (currentTimeDigits.hourOnes != previousTimeDigits.hourOnes) {
      animateVerticalSlide(previousTimeDigits.hourOnes, currentTimeDigits.hourOnes, 7);
    }

    if (currentTimeDigits.hourTens != previousTimeDigits.hourTens) {
      animateVerticalSlide(previousTimeDigits.hourTens, currentTimeDigits.hourTens, 1);
    }

    // Display PM indicator in bottom-right corner (only in 12-hour format)
    if (!is24Hour) {
      ledMatrix.drawPixel(ledMatrix.width() - 1, ledMatrix.height() - 1, currentDateTime.isPM());
    }

    // Toggle the colon blinker for time separator
    for (byte i = 0; i < sizeof(COLON_BLINK_POSITIONS) / sizeof(COLON_BLINK_POSITIONS[0]); i++) {
      ledMatrix.drawPixel(COLON_BLINK_POSITIONS[i][0], COLON_BLINK_POSITIONS[i][1], toggleBlinker);
    }

    ledMatrix.write();

    // Print timestamp to serial when seconds change (debug only)
    #if ENABLE_SERIAL_DEBUG
    if (currentTimeDigits.secOnes != previousTimeDigits.secOnes) {
      Serial.println(currentDateTime.timestamp(DateTime::TIMESTAMP_FULL));
    }
    #endif

    // Update previous digits for next comparison
    extractTimeDigits(previousTimeDigits);
    toggleBlinker = !toggleBlinker;
  }
}

/**
 * @brief Configures LED matrix module positions and rotations
 * 
 * This function sets up the 4x MAX7219 LED matrix modules for a 32x8 display.
 * The matrix is configured in a standard horizontal arrangement:
 * - 4 modules arranged horizontally (8x8 each)
 * - Total display size: 32x8 pixels
 * - Standard left-to-right positioning
 * - No rotation applied (0 degrees)
 * 
 * @note This function is designed specifically for 4x MAX7219 modules
 * @note Matrix size is fixed to 32x8 pixels
 */
void configureLedMatrix() {
  int totalModules = MATRIX_TOTAL_MODULES_X * MATRIX_TOTAL_MODULES_Y;

  for (int moduleIndex = 0; moduleIndex < totalModules; moduleIndex++) {
    // Calculate base position for this module
    byte xPosition = moduleIndex % MATRIX_TOTAL_MODULES_X;
    byte yPosition = moduleIndex / MATRIX_TOTAL_MODULES_X;
    byte rotationAngle = 1; // Default rotation (0 degrees)

    ledMatrix.setPosition(moduleIndex, xPosition, yPosition);
    ledMatrix.setRotation(moduleIndex, rotationAngle);
  }
}

/**
 * @brief Scrolls text horizontally across the LED matrix display
 * 
 * This function creates a smooth horizontal scrolling effect for text messages.
 * The text scrolls from right to left across the display. If the text is shorter
 * than the display width, it will still scroll to ensure visibility.
 * 
 * @param message The text message to scroll (null-terminated string)
 * 
 * @note The function adds a space at the end of the message for better readability
 * @note Scroll speed is controlled by the delay(30) - adjust for different speeds
 * @note This function resets the previousTimeDigits to prevent animation conflicts
 * 
 * @example
 * scrollTextHorizontally("Hello World"); // Scrolls "Hello World " across display
 */
void scrollTextHorizontally(const char* message) {  
  // Print message to serial for debugging
  #if ENABLE_SERIAL_DEBUG
  Serial.println(message);
  #endif

  // Prepare text buffer with trailing space for better readability
  snprintf(textScrollBuffer, sizeof(textScrollBuffer), "%s ", message);

  // Calculate total text width in pixels
  int totalTextWidth = strlen(textScrollBuffer) * CHAR_WIDTH;
  
  // Ensure minimum scroll distance is at least the display width
  if (totalTextWidth < ledMatrix.width()) {
    totalTextWidth = ledMatrix.width();
  }

  // Scroll text from right edge to left edge
  for (int xPosition = ledMatrix.width(); xPosition >= -totalTextWidth; xPosition--) {
    // Clear display and set cursor position
    ledMatrix.fillScreen(LOW);
    ledMatrix.setCursor(xPosition, 0);
    ledMatrix.print(textScrollBuffer);
    ledMatrix.write();
    delay(35); // Control scroll speed (35ms per frame)
  }

  // Reset previous time digits to prevent animation conflicts
  previousTimeDigits = TimeDigits();
}

/**
 * @brief Displays the current date with ordinal suffixes and adjusts brightness
 * 
 * This function is called periodically by the date display timer. It:
 * 1. Checks if GPS time is acquired, shows waiting message if not
 * 2. Formats the date with weekday, day with ordinal suffix, month, and year
 * 3. Scrolls the formatted date across the display
 * 4. Adjusts LED brightness based on time of day (night mode)
 * 
 * @note Called every DATE_DISPLAY_INTERVAL_MS milliseconds
 * @note Night mode: low brightness from 9PM to 6AM, high brightness otherwise
 * @note Date format: "Mon 1st Jan 2024" (example)
 * 
 * @example
 * // If current date is Monday, January 1st, 2024
 * // Display will show: "Mon 1st Jan 2024"
 */
void displayDate() {
  // Show waiting message if GPS time is not valid
  if (!gpsModule.time.isValid()) {
    scrollTextHorizontally(WAITING_FOR_GPS);
    return;
  }

  // Bounds checking for array access using Arduino constrain function
  int dayOfWeek = constrain(currentDateTime.dayOfTheWeek(), 0, 6);
  int month = constrain(currentDateTime.month(), 1, 12);

  // Format date string with ordinal suffix
  // Format: "Day OrdinalSuffix Month Year" (e.g., "Mon 1st Jan 2024")
  int day = currentDateTime.day();
  const char* ordinalSuffix = getOrdinalSuffix(day);

  snprintf(
    textScrollBuffer, sizeof(textScrollBuffer),
    "%s %d%s %s %d",
    WEEKDAY_NAMES[dayOfWeek],
    day,
    ordinalSuffix,
    MONTH_NAMES[month - 1],
    currentDateTime.year()
  );

  // Display the formatted date
  scrollTextHorizontally(textScrollBuffer);
  displayGpsLocation();

  // Adjust brightness based on time of day (night mode)
  if (currentDateTime.hour() >= 21 || currentDateTime.hour() <= 6) {
    // Night hours: 9PM to 6AM - use low brightness
    ledMatrix.setIntensity(LED_BRIGHTNESS_LOW);
  } else {
    // Day hours: 6AM to 9PM - use high brightness
    ledMatrix.setIntensity(LED_BRIGHTNESS_HIGH);
  }
}

/**
 * @brief Returns the appropriate ordinal suffix for a given number
 * 
 * This function determines the correct ordinal suffix (st, nd, rd, th) for a number.
 * It handles special cases for numbers ending in 11, 12, 13 (which all use "th")
 * and regular cases for other numbers.
 * 
 * @param number The number to get the ordinal suffix for
 * @return Pointer to the ordinal suffix string ("st", "nd", "rd", or "th")
 * 
 * @note This is a helper function for date formatting
 * 
 * @example
 * getOrdinalSuffix(1)  // Returns "st" (1st)
 * getOrdinalSuffix(2)  // Returns "nd" (2nd)
 * getOrdinalSuffix(3)  // Returns "rd" (3rd)
 * getOrdinalSuffix(11) // Returns "th" (11th)
 * getOrdinalSuffix(21) // Returns "st" (21st)
 */
const char* getOrdinalSuffix(int number) {
  int lastTwoDigits = number % 100;
  int lastDigit = number % 10;

  // Special case: 11th, 12th, 13th all use "th"
  if (lastTwoDigits >= 11 && lastTwoDigits <= 13) {
    return "th";
  } else {
    // Regular cases based on last digit
    switch (lastDigit) {
      case 1: return "st";
      case 2: return "nd";
      case 3: return "rd";
      default: return "th";
    }
  }
}

/**
 * @brief Extracts individual digits from the current time for display
 * 
 * This function takes the current date/time and extracts individual digits
 * for hours, minutes, and seconds. It converts the numeric values to ASCII
 * characters for display on the LED matrix.
 * 
 * @param digits Reference to TimeDigits struct to store the extracted digits
 * 
 * @note Uses configurable time format (12-hour or 24-hour) based on TIME_FORMAT_* defines
 * @note All digits are converted to ASCII characters ('0' to '9')
 * 
 * @example
 * // 12-hour format: If current time is 2:35:47 PM
 * // digits.hourTens = '0', digits.hourOnes = '2'
 * // 24-hour format: If current time is 14:35:47
 * // digits.hourTens = '1', digits.hourOnes = '4'
 */
void extractTimeDigits(TimeDigits& digits) {
  // Get time components based on cached format
  int hour;
  if (is24Hour) {
    hour = currentDateTime.hour();
  } else {
    hour = currentDateTime.twelveHour();
  }

  int minute = currentDateTime.minute();
  int second = currentDateTime.second();

  // Extract and convert digits to ASCII characters
  digits.hourOnes = (hour % 10) + '0';
  digits.hourTens = (hour / 10) + '0';
  digits.minOnes = (minute % 10) + '0';
  digits.minTens = (minute / 10) + '0';
  digits.secOnes = (second % 10) + '0';
  digits.secTens = (second / 10) + '0';
}

/**
 * @brief Creates a vertical sliding animation when time digits change
 * 
 * This function creates a smooth vertical sliding effect when a time digit changes.
 * The old character slides down while the new character slides up from the top,
 * creating a visually appealing transition effect.
 * 
 * @param previousChar The character that was previously displayed
 * @param newChar The new character to display
 * @param xPosition The X coordinate where the animation occurs
 * 
 * @note Animation speed is controlled by delay(25) - adjust for different speeds
 * @note This function blocks execution during animation
 * 
 * @example
 * animateVerticalSlide('5', '6', 25); // Animate change from '5' to '6' at x=25
 */
void animateVerticalSlide(char previousChar, char newChar, int xPosition) {
  // Create sliding effect by moving both characters vertically
  for (uint8_t yPosition = 0; yPosition <= ledMatrix.height(); yPosition++) {
    // Draw new character sliding up from top
    ledMatrix.drawChar(xPosition, yPosition - ledMatrix.height(), newChar, HIGH, LOW, 1);
    // Draw previous character sliding down
    ledMatrix.drawChar(xPosition, yPosition, previousChar, HIGH, LOW, 1);
    ledMatrix.write();
    delay(25); // Control animation speed (25ms per frame)
  }
}

/**
 * @brief Displays GPS location information (latitude, longitude, altitude)
 * 
 * This function displays the current GPS coordinates
 * immediately after the date display. It shows:
 * - Latitude with "LAT:" prefix (4 decimal places, ~11m accuracy)
 * - Longitude with "LON:" prefix (4 decimal places, ~11m accuracy)
 * - Altitude with "ALT:" prefix and "ft" suffix (1 decimal place,
 * 
 * @note Only displays if GPS location data is valid
 * @note Each coordinate is displayed on a separate line with scrolling
 * @note GPS precision is fixed at 4 decimal places for optimal readability
 * @note All values are validated before display to prevent invalid data
 * @note Uses integer arithmetic to avoid sprintf floating-point issues on Arduino
 */
void displayGpsLocation() {
  if (!gpsModule.location.isValid()) return;

  // Update GPS stability filter with new readings
  gpsFilter.update();

  // Display latitude using filtered value for stability (4 decimal places = ~11m accuracy)
  float lat = gpsFilter.getFilteredLatitude();
  int latInt = (int)lat;
  float latDecimal = lat - latInt;
  int latFrac = (int)(latDecimal * GPS_COORD_PRECISION_MULTIPLIER);
  
  #if ENABLE_SERIAL_DEBUG
  // Debug output: show raw vs filtered values
  Serial.print("LAT - Raw: ");
  Serial.print(gpsModule.location.lat(), 6);
  Serial.print(", Filtered: ");
  Serial.print(lat, 6);
  Serial.print(", Readings: ");
  Serial.println(gpsFilter.getTotalReadings());
  #endif
  
  snprintf(textScrollBuffer, sizeof(textScrollBuffer), "%s%d.%04d", GPS_LAT_PREFIX, latInt, latFrac);
  scrollTextHorizontally(textScrollBuffer);

  // Display longitude using filtered value for stability (4 decimal places = ~11m accuracy)
  float lng = gpsFilter.getFilteredLongitude();
  int lngInt = (int)lng;
  float lngDecimal = lng - lngInt;
  int lngFrac = (int)(lngDecimal * GPS_COORD_PRECISION_MULTIPLIER);
  
  #if ENABLE_SERIAL_DEBUG
  // Debug output: show raw vs filtered values  
  Serial.print("LON - Raw: ");
  Serial.print(gpsModule.location.lng(), 6);
  Serial.print(", Filtered: ");
  Serial.println(lng, 6);
  #endif
  
  snprintf(textScrollBuffer, sizeof(textScrollBuffer), "%s%d.%04d", GPS_LON_PREFIX, lngInt, lngFrac);
  scrollTextHorizontally(textScrollBuffer);

  // Display altitude using filtered value for stability if available
  if (gpsModule.altitude.isValid()) {
    // Use filtered altitude value and integer arithmetic since Arduino sprintf doesn't support floating-point
    double altFeet = gpsFilter.getFilteredAltitude();
    int altInt = (int)altFeet;
    int altFrac = (int)((altFeet - altInt) * GPS_ALT_PRECISION_MULTIPLIER);
    
    #if ENABLE_SERIAL_DEBUG
    // Debug output: show raw vs filtered altitude
    Serial.print("ALT - Raw: ");
    Serial.print(gpsModule.altitude.feet(), 2);
    Serial.print("ft, Filtered: ");
    Serial.print(altFeet, 2);
    Serial.println("ft");
    #endif
    
    snprintf(textScrollBuffer, sizeof(textScrollBuffer), "%s%d.%d%s", GPS_ALT_PREFIX, altInt, altFrac, GPS_ALT_SUFFIX);
    scrollTextHorizontally(textScrollBuffer);
  }
}

/**
 * @brief Detects power cycles and toggles time format if threshold is reached
 * 
 * This function detects power cycles during the startup sequence to toggle between 
 * 12-hour and 24-hour time formats. The detection uses EEPROM to store cycle count.
 * 
 * The system works by incrementing a counter on each power cycle during startup.
 * If the counter reaches the threshold (5), it toggles the time format and resets 
 * the counter. The counter is reset after the welcome message to prevent accidental 
 * toggles during normal operation.
 * 
 * @note Called in setup() to detect power cycles for format switching
 * @note Uses EEPROM to persist cycle data across reboots
 * @note Counter resets after welcome message
 */
void checkPowerCycles() {
  // Read cycle count from EEPROM
  unsigned long cycleCount = 0;
  EEPROM.get(EEPROM_POWER_CYCLE_ADDR, cycleCount);
  
  // Increment cycle count
  cycleCount++;
  EEPROM.put(EEPROM_POWER_CYCLE_ADDR, cycleCount);

  // If threshold reached, toggle time format
  if (cycleCount > POWER_CYCLE_THRESHOLD) toggleTimeFormat();
}

/**
 * @brief Toggles between 12-hour and 24-hour time format
 * 
 * This function toggles the time format and stores the new setting in EEPROM.
 * It also displays a confirmation message on the LED matrix to inform the user
 * of the format change.
 * 
 * @note Stores the new format in EEPROM for persistence
 * @note Shows confirmation message on display
 */
void toggleTimeFormat() {
  bool currentFormat = EEPROM.read(EEPROM_TIME_FORMAT_ADDR);
  bool newFormat = !currentFormat;

  EEPROM.write(EEPROM_TIME_FORMAT_ADDR, newFormat);

  // Update cached format for immediate effect
  is24Hour = newFormat;

  // Show format change confirmation on display
  if (newFormat) {
    scrollTextHorizontally(FORMAT_24H_MESSAGE);
  } else {
    scrollTextHorizontally(FORMAT_12H_MESSAGE);
  }
}

/**
 * @brief Loads the GPS baud rate from EEPROM or uses default
 * @return The GPS baud rate to use for serial communication
 */
unsigned long loadGpsBaudRate() {
  unsigned long savedBaudRate = 0;
  EEPROM.get(EEPROM_GPS_BAUD_RATE_ADDR, savedBaudRate);
  
  // Validate the saved baud rate against supported rates
  for (int i = 0; i < GPS_BAUD_RATES_COUNT; i++) {
    if (savedBaudRate == GPS_BAUD_RATES[i]) {
      #if ENABLE_SERIAL_DEBUG
      Serial.print("Loaded GPS baud rate from EEPROM: ");
      Serial.println(savedBaudRate);
      #endif
      return savedBaudRate;
    }
  }
  
  // If no valid baud rate found in EEPROM, return default (115200)
  #if ENABLE_SERIAL_DEBUG
  Serial.println("No valid GPS baud rate in EEPROM, using default: 115200");
  #endif
  return 115200;
}

/**
 * @brief Saves the detected GPS baud rate to EEPROM
 * @param baudRate The baud rate to save
 */
void saveGpsBaudRate(unsigned long baudRate) {
  EEPROM.put(EEPROM_GPS_BAUD_RATE_ADDR, baudRate);
  #if ENABLE_SERIAL_DEBUG
  Serial.print("Saved GPS baud rate to EEPROM: ");
  Serial.println(baudRate);
  #endif
}

/**
 * @brief Detects the correct GPS baud rate by testing different rates
 * Called during rain effect when GPS signal is not valid
 */
void detectGpsBaudRate() {
  // Skip detection if already completed
  if (gpsBaudDetectionComplete) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Initialize baud rate detection on first call
  if (baudTestStartTime == 0) {
    baudTestStartTime = currentTime;
    currentBaudRateIndex = 0;
    validGpsSentencesCount = 0;
    
    // Start with the first baud rate in the list
    currentGpsBaudRate = GPS_BAUD_RATES[currentBaudRateIndex];
    Serial.end();
    delay(100);  // Allow serial to properly close
    Serial.begin(currentGpsBaudRate);
    
    #if ENABLE_SERIAL_DEBUG
    // Note: Debug output may not work during baud rate changes
    // This will only show after detection is complete
    #endif
    
    return;
  }
  
  // Read and process GPS data for current baud rate test
  while (Serial.available()) {
    char receivedChar = Serial.read();
    if (gpsModule.encode(receivedChar)) {
      // Valid GPS sentence received
      validGpsSentencesCount++;
    }
  }
  
  // Check if we have enough valid sentences to confirm this baud rate
  if (validGpsSentencesCount >= GPS_MIN_VALID_SENTENCES) {
    // Found the correct baud rate!
    gpsBaudDetectionComplete = true;
    saveGpsBaudRate(currentGpsBaudRate);
    
    #if ENABLE_SERIAL_DEBUG
    Serial.print("GPS baud rate detected: ");
    Serial.println(currentGpsBaudRate);
    Serial.print("Valid sentences received: ");
    Serial.println(validGpsSentencesCount);
    #endif
    
    return;
  }
  
  // Check if test duration has elapsed for current baud rate
  if (currentTime - baudTestStartTime >= GPS_BAUD_TEST_DURATION_MS) {
    // Move to next baud rate
    currentBaudRateIndex++;
    
    if (currentBaudRateIndex >= GPS_BAUD_RATES_COUNT) {
      // All baud rates tested, none worked - use default and mark complete
      gpsBaudDetectionComplete = true;
      currentGpsBaudRate = 115200;  // Default fallback
      
      #if ENABLE_SERIAL_DEBUG
      Serial.println("GPS baud rate detection failed - using default 115200");
      #endif
      
      // Ensure serial is configured with default baud rate
      Serial.end();
      delay(100);
      Serial.begin(currentGpsBaudRate);
      
      return;
    }
    
    // Test next baud rate
    currentGpsBaudRate = GPS_BAUD_RATES[currentBaudRateIndex];
    validGpsSentencesCount = 0;
    baudTestStartTime = currentTime;
    
    // Reconfigure serial with new baud rate
    Serial.end();
    delay(100);  // Allow serial to properly close
    Serial.begin(currentGpsBaudRate);
    
    #if ENABLE_SERIAL_DEBUG
    // Debug output won't work during baud rate transitions
    #endif
  }
}
