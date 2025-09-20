# GPS LED Clock - Arduino Project with PlatformIO

A sophisticated Arduino-based GPS clock that displays time on a 32x8 LED matrix with beautiful animations and effects. The clock automatically synchronizes with GPS time and includes a rain effect animation when GPS signal is lost.

**Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.**

## Working Effectively

### Prerequisites and Setup

- Install PlatformIO first - this is required for all build operations:
  ```bash
  python3 -m pip install platformio
  export PATH="$PATH:$HOME/.local/bin"
  ```
- Verify installation: `pio --version`

### Project Structure

The project follows standard PlatformIO structure:
- `src/` - Main source code (main.cpp, config.h, RainEffect.cpp/h)
- `lib/` - Custom libraries (Max72xxPanel for LED matrix control)
- `platformio.ini` - Project configuration and dependencies
- `test/` - Unit tests directory (currently empty)
- `.pio/` - Build output and cache (gitignored)

### Building and Development

**CRITICAL BUILD NOTES:**
- Build requires internet connection to download AVR platform and libraries
- If build fails with `HTTPClientError`, you have network connectivity issues
- Build time varies (30 seconds to 10 minutes depending on platform download needs)
- NEVER CANCEL builds - set timeout to 10 minutes: `timeout=600`

#### Bootstrap and Build Process:
```bash
cd /path/to/gps-led-clock
export PATH="$PATH:$HOME/.local/bin"

# Install dependencies and build (first run may take up to 10 minutes)
pio run

# Build specific targets
pio run -t build        # Build only
pio run -t upload       # Upload to connected device
pio run -t clean        # Clean build artifacts
```

#### Development Workflow:
```bash
# Check code quality (requires internet for first run)
pio check

# Monitor serial output (115200 baud)
pio device monitor --baud 115200

# List available build targets
pio run --list-targets

# View system information
pio system info
```

## Validation and Testing

### Serial Monitoring for Debug
- **Connect to serial monitor** to validate functionality: `pio device monitor --baud 115200`
- **Debug mode**: Set `ENABLE_SERIAL_DEBUG` to `true` in `src/config.h` for verbose output
- **Expected output**: GPS data, timestamp updates, signal status messages

### Manual Validation Scenarios
Since this is embedded hardware, manual validation requires actual hardware:

1. **Power-on sequence validation**:
   - Connect GPS module and LED matrix as per README pin connections
   - Power on device and observe startup animation
   - Verify welcome message "Arduino 32x8 GPS Clock" scrolls across display

2. **GPS functionality validation**:
   - Monitor serial output for GPS signal acquisition
   - Verify "Waiting for GPS Signal..." message when no signal
   - Verify time display appears when GPS signal acquired
   - Test rain effect when GPS signal lost (disconnect GPS temporarily)

3. **Time format toggle validation**:
   - Power cycle device 5 times during startup to toggle 12H/24H format
   - Verify format confirmation message displays
   - Test PM indicator (bottom-right pixel) in 12H mode

**Note**: This project requires physical hardware (Arduino Nano + GPS + LED matrix) for full validation. Code compilation and static analysis can be performed without hardware.

### Unit Testing
- **Test framework**: PlatformIO native unit testing
- **Current status**: No unit tests implemented (`test/` directory contains only README)
- **Running tests**: `pio test` (requires AVR platform installation)

## Configuration and Customization

### Key Configuration Files
- `src/config.h` - Hardware pins, timing, timezone, messages
- `platformio.ini` - Build configuration, dependencies, board settings

### Common Customizations
- **Timezone**: Modify `TIMEZONE_OFFSET_*` constants in `config.h`
- **Brightness**: Adjust `LED_BRIGHTNESS_HIGH/LOW` in `config.h`
- **Messages**: Change `WELCOME_MESSAGE` and other text in `config.h`
- **Timing**: Modify `TIME_UPDATE_INTERVAL_MS` and `DATE_DISPLAY_INTERVAL_MS`

### Hardware Configuration
- **Fixed 32x8 LED matrix**: Project designed specifically for 4x MAX7219 modules
- **Pin connections**: Defined in `config.h`, cannot be changed without code modifications
- **GPS module**: Requires NMEA-compatible GPS (NEO-6M recommended)

## Build Troubleshooting

### Network Issues
- **Symptom**: `HTTPClientError` during build
- **Cause**: Cannot download AVR platform or libraries
- **Solutions**: 
  - Check internet connectivity
  - Try `pio platform install atmelavr` separately
  - Use `pio pkg install` to install dependencies
  - Alternative: Use Arduino IDE if PlatformIO fails due to network issues
  - Try offline mode: `pio run --offline` (if platform already installed)

### Platform Issues
- **Symptom**: "Platform atmelavr not found"
- **Solution**: Internet required for first-time platform installation
- **Alternative**: Use Arduino IDE if PlatformIO platform installation fails

### Library Dependencies
The project uses these libraries (auto-installed by PlatformIO):
- TinyGPSPlus (GPS parsing)
- Adafruit GFX Library (graphics primitives)
- Max72xxPanel (custom local library for LED matrix)
- RTClib (date/time handling)
- TickTwo (timer functions)

## Code Quality and CI

### Static Analysis
- **Run code check**: `pio check` 
- **Requires**: Internet connection for first run to download analysis tools
- **Output**: Code quality metrics and potential issues

## Code Navigation and Important Files

### Frequently Visited Files
- **`src/main.cpp`** - Main application logic, setup(), loop(), time display functions
- **`src/config.h`** - All configuration constants, pin definitions, messages, timing
- **`src/RainEffect.cpp/h`** - Rain animation implementation when GPS signal lost
- **`platformio.ini`** - Build configuration, board settings, library dependencies
- **`README.md`** - Hardware setup, pin connections, feature documentation

### Key Code Locations
- **Time display logic**: `main.cpp` functions `displayTime()`, `updateTimeDisplay()`
- **GPS parsing**: `main.cpp` functions `readGPSData()`, `updateTimeFromGPS()`
- **Configuration constants**: All in `config.h` - timezone, brightness, timing
- **Hardware pin definitions**: `config.h` lines 50-52
- **Animation effects**: `RainEffect.cpp` class methods
- **Serial debugging**: `main.cpp` - look for `Serial.println()` calls

### Common Modification Points
- **Timezone changes**: Edit `TIMEZONE_OFFSET_*` constants in `config.h`
- **Display messages**: Update text constants in `config.h` (lines 124-130)
- **Hardware pins**: Modify pin definitions in the pin definitions section of `config.h`
- **Timing intervals**: Adjust `*_INTERVAL_MS` constants in `config.h`
- **Brightness levels**: Change `LED_BRIGHTNESS_*` in `config.h`

### Code Structure
- **Codebase size**: moderate-sized C/C++ project
- **Main files**: 
  - `main.cpp` (main application logic)
  - `config.h` (configuration constants)
  - `RainEffect.cpp/h` (animation effects)
  - `Max72xxPanel` (LED matrix library)

### Common Development Tasks
- **Adding new animations**: Extend `RainEffect` class or create similar
- **Modifying time display**: Edit time formatting functions in `main.cpp`
- **Adding new messages**: Update text arrays in `config.h`
- **Hardware changes**: Update pin definitions in `config.h`

## Important Limitations

### Hardware Dependencies
- **Physical hardware required**: Cannot fully test without Arduino + GPS + LED matrix
- **Upload requires device**: `pio run -t upload` needs connected Arduino
- **Serial monitoring**: Requires connected device with GPS module

### Network Dependencies
- **First build**: Requires internet to download platform and libraries
- **Code analysis**: Requires internet for analysis tools download
- **Updates**: Library updates require internet connectivity

### No CI/CD Pipeline
- **No GitHub workflows**: Project lacks automated CI/CD
- **Manual testing required**: All validation must be done with physical hardware
- **No automated testing**: No integration with test runners

## Quick Reference Commands

```bash
# Essential commands (run from project root)
export PATH="$PATH:$HOME/.local/bin"

# First-time setup (NEVER CANCEL - use 10+ minute timeout)
python3 -m pip install platformio   # Install PlatformIO
pio --version                       # Verify installation

# Build (NEVER CANCEL - use 10+ minute timeout)
pio run                              # Full build (first run downloads platform)
pio run --offline                    # Offline build (if platform installed)

# Hardware interaction (requires connected device)
pio run -t upload                    # Upload to device
pio device monitor --baud 115200    # Monitor serial output  
pio device list                     # List connected devices

# Development
pio check                           # Static analysis
pio run -t clean                    # Clean build
pio system info                     # System information
pio boards nanoatmega328            # Board information

# Testing (requires platform installation)
pio test                           # Run unit tests (currently none)

# Troubleshooting
pio platform install atmelavr      # Install platform separately
pio pkg install                    # Install dependencies
pio run --list-targets             # List all available targets
```

## Key Project Features
- GPS time synchronization with timezone adjustment
- 32x8 LED matrix display with animations
- Rain effect when GPS signal lost
- Automatic brightness control (day/night)
- Time format toggle (12H/24H)
- Date and GPS location display
- Serial debugging capabilities

**Remember**: This is an embedded project requiring physical hardware for complete validation. Always test code changes with actual hardware when possible.