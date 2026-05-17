# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino Nano GPS clock that displays time on a 32x8 LED matrix (4x MAX7219 modules). GPS provides time sync; when signal is lost, a rain animation plays. Time format (12H/24H) is toggled via 5 rapid power cycles and stored in EEPROM.

## Build and Upload

This is a PlatformIO project targeting `nanoatmega328` (ATmega328P Arduino Nano).

```bash
# Build
pio run

# Build and upload (disconnect GPS TX from Arduino RX0 first)
pio run -t upload

# Serial monitor (115200 baud)
pio device monitor

# Clean build artifacts
pio run -t clean
```

## Simulation

Wokwi simulation is configured via `wokwi.toml` and `diagram.json`. The firmware path is `.pio/build/nanoatmega328/firmware.hex`. A custom GPS simulator chip (NEO-6M emulator) is loaded from `chips/gps-simulator.chip.wasm`; its C source lives in `chips/gps-simulator.chip.c` and must be recompiled to wasm if changed.

## Architecture

All application source is in `src/`, with one local library in `lib/Max72xxPanel/`.

- **`src/config.h`** - Central configuration: pin definitions, timing constants, timezone offset, EEPROM addresses, display messages, `TimeDigits` struct, and all function prototypes. All `#include` directives for external libraries live here. This is the primary file to edit for configuration changes.
- **`src/main.cpp`** - Main application logic: `setup()`/`loop()`, GPS parsing via `Serial`, time display with animated digit transitions (vertical slide), date display with ordinal suffixes, GPS location display, brightness control (night mode 9PM-6AM), scrolling text, and power-cycle-based time format toggling.
- **`src/RainEffect.h/.cpp`** - Self-contained rain animation class shown when GPS signal is lost. Manages raindrop falling, ground impact flashes with fade-out. Operates on `Max72xxPanel&` reference.
- **`src/GpsStabilityFilter.h/.cpp`** - Hybrid median+average filter for GPS coordinates. Uses circular buffers (12 readings), removes top/bottom 20% outliers, averages the middle 60%. Memory-efficient at 146 bytes for Arduino Nano's 2KB SRAM.
- **`lib/Max72xxPanel/`** - Local copy of the Max72xxPanel library (Adafruit_GFX plugin for MAX7219 matrix control via SPI).

## Key Constraints

- **Fixed 32x8 display**: All pixel positions, character placement, and animation logic are hardcoded for exactly 4 horizontal MAX7219 8x8 modules. Changing matrix size requires modifying display logic throughout `main.cpp`.
- **Arduino Nano memory limits**: 2KB SRAM, 32KB flash. Avoid heap allocation; prefer stack/static. The GPS filter alone uses 146 bytes (~7% of SRAM).
- **GPS on hardware Serial (RX0)**: The GPS module TX connects to Arduino pin 0 (hardware serial RX). This means Serial is shared between GPS input and debug output - GPS TX must be disconnected during firmware upload.
- **No `sprintf` float support**: Arduino AVR `sprintf` does not support `%f`. Use integer arithmetic to format decimal values (see `displayGpsLocation()` pattern).
- **EEPROM layout**: Address 0 = time format (1 byte), address 1-4 = power cycle count (4 bytes). Do not overlap.
- **Serial debug toggle**: `ENABLE_SERIAL_DEBUG` in `src/config.h` gates debug output. Disable for production since `Serial` is also the GPS input line.

## Timezone Configuration

Default is IST (UTC+5:30). Change via defines in `src/config.h`:

```cpp
#define TIMEZONE_OFFSET_HOURS   5
#define TIMEZONE_OFFSET_MINUTES 30
```
