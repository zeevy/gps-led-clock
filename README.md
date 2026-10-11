# GPS Clock with 32x8 LED Matrix Display

A sophisticated Arduino-based GPS clock that displays time on a 32x8 LED matrix with beautiful animations and effects. The clock automatically synchronizes with GPS time and includes a rain effect animation when GPS signal is lost.

<img width="2241" height="805" alt="image" src="https://github.com/user-attachments/assets/65b796b9-a7a7-471e-b6f8-0650e257d205" />

## Features

- **GPS Time Synchronization**: Automatically syncs with GPS time (UTC) with timezone adjustment and signal quality monitoring
- **Dynamic Time Format**: Toggle between 12-hour (with AM/PM) or 24-hour display via power cycles
- **Animated Time Display**: Smooth vertical sliding animations when time digits change
- **Date Display**: Shows date with ordinal suffixes (1st, 2nd, 3rd, etc.) every 2 minutes 30 seconds
- **GPS Location Display**: Shows latitude, longitude, and altitude in feet (4 decimal places, ~11m accuracy) immediately after date display
- **Rain Effect Animation**: Beautiful rain animation when GPS signal is lost
- **Smart Signal Detection**: Uses GPS time age for reliable signal quality monitoring
- **Automatic Brightness Control**: Adjusts brightness based on time of day (night mode)
- **Fixed 32x8 Display**: Optimized for 4x MAX7219 LED matrix modules
- **Scrolling Text Messages**: Welcome and status messages with smooth scrolling
- **PM Indicator**: Visual indicator for PM time display
- **Blinking Colon**: Animated colon separator for time display

## Hardware Requirements

### Components

- **Arduino Nano** (or compatible Arduino board)
- **GPS Module** (NEO-6M or similar with NMEA output)
- **32x8 LED Matrix Display** (exactly 4x MAX7219 modules, 8x8 each)
- **Breadboard and jumper wires**
- **5V Power supply** (or USB power)

**Important**: This project is specifically designed for a **32x8 LED matrix** using **4x MAX7219 modules**. Other matrix sizes or configurations are not supported without code modifications.

**Note**: The matrix configuration is fixed and optimized for this specific setup. Changing the matrix size would require significant code modifications to the display logic, positioning calculations, and text scrolling functions.

### Pin Connections

| Component           | Arduino Pin         | Description         |
|---------------------|--------------------|---------------------|
| **GPS TX**          | Pin 8              | GPS data output (software serial, 9600 baud) |
| **GPS RX**          | Pin 9 (optional)   | Software serial TX, driven high. Leave free or wire to GPS RX |
| **GPS VCC**         | 5V                 | Power supply        |
| **GPS GND**         | GND                | Ground              |
| **LED Matrix DIN**  | Pin 11 (MOSI)      | Data input          |
| **LED Matrix CLK**  | Pin 13 (SCK)       | Clock signal        |
| **LED Matrix CS**   | Pin 10 (SS)        | Chip select         |
| **LED Matrix VCC**  | 5V                 | Power supply        |
| **LED Matrix GND**  | GND                | Ground              |

### Other boards

The same firmware builds for these boards. Pick the env with `pio run -e <env> -t upload`.

| Board | Env | GPS TX to | Matrix DIN | Matrix CLK | Matrix CS |
|---|---|---|---|---|---|
| Arduino Nano | `nanoatmega328` | D8 | D11 | D13 | D10 |
| Wemos D1 mini | `d1_mini` | D2 | D7 | D5 | D8 |
| ESP32 DevKit V1 | `esp32dev` | GPIO16 | GPIO23 | GPIO18 | GPIO5 |
| Lolin S2 mini | `lolin_s2_mini` | GPIO4 | GPIO11 | GPIO7 | GPIO12 |
| ESP32-S3 DevKitC-1 | `esp32s3` | GPIO6 | GPIO11 | GPIO12 | GPIO10 |
| ESP32-C3 super mini | `esp32c3` | GPIO3 | GPIO6 | GPIO4 | GPIO7 |
| ESP32-C6 DevKitC-1 | `esp32c6` | GPIO2 | GPIO19 | GPIO21 | GPIO18 |

The D1 mini and ESP32 boards use 3.3 V logic, but the MAX7219 needs at least 3.5 V for a logic high at 5 V supply. Put a 74AHCT125 (or 74HCT125) powered from 5 V on DIN, CLK and CS, and power the matrix from 5 V. The GPS TX can go straight to the board.

<img width="2280" height="1386" alt="image" src="https://github.com/user-attachments/assets/2859d131-e09c-44f3-abb6-3fb95c12ad07" />

https://github.com/user-attachments/assets/0466696c-467c-4d51-b680-a392c1aeb819

## Software Requirements

- **PlatformIO** (recommended) or Arduino IDE
- **Arduino AVR Boards** package (ESP8266 and ESP32 boards: PlatformIO installs their platforms)
- Required libraries (automatically installed with PlatformIO):
  - TinyGPSPlus
  - Adafruit GFX Library
  - Max72xxPanel
  - RTClib
  - TickTwo
  - NeoSWSerial (Nano only)

## Installation & Setup

### Using PlatformIO (Recommended)

**Clone the repository**:

```bash
git clone https://github.com/zeevy/gps-led-clock.git
cd gps-led-clock
```

**Open in PlatformIO**:

```bash
pio project init
```

**Build and upload**:

```bash
pio run -t upload
```

> **Note:** The GPS module must send at 9600 baud, the NEO-6M factory default. If yours was set to another speed, set it back to 9600 in u-center (UBX > CFG > PRT) and save it (UBX > CFG > CFG). The GPS is on pin 8, so it can stay connected while you upload and use the serial monitor.

## Configuration

### Time Format Configuration

The time format can be toggled dynamically using power cycle detection:

**Power Cycle Toggle:**

- **5 power cycles** during **startup sequence** toggles between formats
- Power cycles must occur between the random pixel animation and welcome message
- **12-hour format** is the default
- Format is stored in EEPROM and persists across reboots
- Format confirmation is shown when toggled

**12-Hour Format:**

- Displays time as 1:00 PM to 12:59 AM
- Shows PM indicator (bottom-right pixel) for PM times
- Example: "2:35." with PM indicator

**24-Hour Format:**

- Displays time as 00:00 to 23:59
- No AM/PM indicator
- Example: "14:35" (no indicator)

**How to Toggle:**

1. Power on the device
2. During the startup animation (between random pixel lighting and welcome message), power off the device and power it back on, repeating this 5 times
3. Wait for the welcome message to complete
4. If toggled, display will show the new format confirmation ("24H FORMAT" or "12H FORMAT")
5. New format persists across reboots

### Timezone Adjustment

To change the timezone, modify the timezone offset in `src/config.h`:

```cpp
// Current: IST (UTC+5:30)
#define TIMEZONE_OFFSET_DAYS    0
#define TIMEZONE_OFFSET_HOURS   5
#define TIMEZONE_OFFSET_MINUTES 30
#define TIMEZONE_OFFSET_SECONDS 0
```

### LED Matrix Configuration

This project is designed specifically for a **32x8 LED matrix** using **4x MAX7219 modules**. The matrix size and configuration are fixed and cannot be changed without code modifications.

**Matrix Specifications:**

- **Display Size**: 32x8 pixels
- **Modules**: 4x MAX7219 (8x8 each)
- **Arrangement**: Horizontal (4 modules side by side)
- **Orientation**: Standard left-to-right

### Display Settings

Modify these constants in `src/config.h`:

```cpp
// Brightness Settings (Adjustable)
#define LED_BRIGHTNESS_HIGH         10    // High brightness (0-15)
#define LED_BRIGHTNESS_LOW          5     // Low brightness (0-15)

// Timing Settings (Adjustable)
#define TIME_UPDATE_INTERVAL_MS     500  // GPS update interval
#define DATE_DISPLAY_INTERVAL_MS    (2 * 60 * 1000UL + 30 * 1000UL) // Date display interval
```

## Usage

### First Time Setup

1. **Power on** the device
2. **Wait for GPS signal** - Rain effect shows until GPS signal is acquired, every `DATE_DISPLAY_INTERVAL_MS` "Waiting for GPS signal..." will be shown, if GPS fix is not yet acquired
3. **Time will appear** once GPS signal is acquired

### Normal Operation

- **Time Display**: Shows current time in configurable format (12-hour or 24-hour) with blinking colon
- **Date Display**: Automatically shows date every 2 minutes 30 seconds
- **GPS Location**: Shows latitude, longitude, and altitude in feet (4 decimal places) after date display
- **PM Indicator**: Bottom-right pixel indicates PM time (12-hour format only)
- **Night Mode**: Automatically dims display from 9 PM to 6 AM
- **Rain Effect**: Activates when GPS signal is lost

### Serial Monitor

Connect to the serial monitor (115200 baud) to see:

- GPS data during signal acquisition
- Timestamp updates
- Status messages

## Customization

### Adding Custom Messages

Modify the welcome message in `src/config.h`:

```cpp
const char* WELCOME_MESSAGE = "Your Custom Message";
```

### Rain Effect Settings

Adjust rain effect parameters in `src/RainEffect.h`:

```cpp
static const int MAX_RAINDROPS = 12;           // Number of simultaneous drops
static const int RAIN_SPAWN_INTERVAL_MS = 200; // Spawn rate
static const int RAIN_FALL_SPEED_MIN_MS = 50;  // Minimum fall speed
static const int RAIN_FALL_SPEED_MAX_MS = 100; // Maximum fall speed
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Development Guidelines

1. **Code Style**: Follow Arduino C++ conventions
2. **Documentation**: Add comprehensive comments and docstrings
3. **Testing**: Run `pio test -e native` (PC unit tests), then test on actual hardware before submitting
4. **Commits**: Use clear, descriptive commit messages

## Support

If you encounter any issues or have questions:

1. Search existing [Issues](https://github.com/zeevy/gps-led-clock/issues)
2. Create a new issue with detailed information
3. Include hardware setup and error messages
