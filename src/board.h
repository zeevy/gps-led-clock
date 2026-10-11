/**
 * @file board.h
 * @brief Hardware abstraction layer: pins and board specific calls
 *
 * The app code uses only the pins and functions defined here. Each
 * supported core has one block below. A new board on a supported core
 * needs only a PlatformIO env; a new core needs a new block here.
 * The matrix CS pin and the GPS pins can be changed per env with a build
 * flag, for example -D PIN_GPS_RX=7. The matrix DIN and CLK pins are the
 * board's hardware SPI pins (MOSI and SCK).
 *
 * @license MIT
 */

#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include <SPI.h>

// GPS serial speed. 9600 is the NEO-6M factory default, and a software
// serial port on a 16 MHz AVR cannot receive 115200 reliably.
#ifndef GPS_BAUD
#define GPS_BAUD 9600
#endif

// Bytes of EEPROM the clock uses: the time format at address 0 and the
// 4 byte power cycle count at address 1 (see config.h).
#define STORAGE_SIZE 8

// Every core block defines the same calls:
//   gpsSerial()     the GPS port (a Stream)
//   gpsBegin()      starts the GPS port at GPS_BAUD
//   spiBegin()      starts the SPI bus for the matrix, before ledMatrix.begin()
//   storageBegin()  call once before the first EEPROM access
//   storageCommit() call after EEPROM writes, so they reach the flash
//   seedRandom()    seeds random() where the core needs it

#if defined(ARDUINO_ARCH_AVR)

// Arduino Nano (ATmega328P). The matrix is on hardware SPI:
// DIN = D11 (MOSI), CLK = D13 (SCK).

#include <NeoSWSerial.h>

static_assert(GPS_BAUD == 9600 || GPS_BAUD == 19200 || GPS_BAUD == 31250 || GPS_BAUD == 38400,
              "NeoSWSerial supports 9600, 19200, 31250 or 38400 baud only");

#ifndef PIN_MATRIX_CS
#define PIN_MATRIX_CS SS   // D10
#endif

#ifndef PIN_GPS_RX
#define PIN_GPS_RX 8       // GPS module TX goes here
#endif

#ifndef PIN_GPS_TX
#define PIN_GPS_TX 9       // NeoSWSerial drives it high as its TX pin; leave free or wire to GPS RX
#endif

// GPS port on its own pins, so D0 and D1 stay free for USB upload and
// the debug log. Built on first use, not before setup().
inline NeoSWSerial& gpsSerial() {
  static NeoSWSerial port(PIN_GPS_RX, PIN_GPS_TX);
  return port;
}

inline void gpsBegin() {
  gpsSerial().begin(GPS_BAUD);
}

inline void spiBegin() {
  SPI.begin();
}

// The AVR EEPROM is written byte by byte, no buffer to set up or flush.
inline void storageBegin() {}
inline void storageCommit() {}

// Noise of the unconnected A0 pin.
inline void seedRandom() {
  randomSeed(analogRead(A0));
}

#elif defined(ARDUINO_ARCH_ESP8266)

// Wemos D1 mini (ESP8266). The matrix is on hardware SPI:
// DIN = D7 (GPIO13), CLK = D5 (GPIO14). The ESP8266 has one full UART,
// and Serial.swap() would move its RX to GPIO13, the SPI data pin, so
// the GPS uses the core's software serial port.

#include <EEPROM.h>
#include <SoftwareSerial.h>

#ifndef PIN_MATRIX_CS
#define PIN_MATRIX_CS SS   // D8 / GPIO15
#endif

#ifndef PIN_GPS_RX
#define PIN_GPS_RX 4       // D2; GPS module TX goes here
#endif

inline SoftwareSerial& gpsSerial() {
  static SoftwareSerial port(PIN_GPS_RX, -1);   // receive only
  return port;
}

inline void gpsBegin() {
  gpsSerial().begin(GPS_BAUD);
}

inline void spiBegin() {
  SPI.begin();
}

// EEPROM lives in one flash sector, buffered in RAM.
inline void storageBegin() {
  EEPROM.begin(STORAGE_SIZE);
}

inline void storageCommit() {
  EEPROM.commit();
}

// random() uses the hardware random number generator until randomSeed() is called.
inline void seedRandom() {}

#elif defined(ARDUINO_ARCH_ESP32)

// ESP32 family (classic, S2, S3, C3, C6). The matrix uses the board's
// default SPI pins; each env in platformio.ini sets PIN_GPS_RX.

#include <EEPROM.h>

#ifndef PIN_MATRIX_CS
#define PIN_MATRIX_CS SS
#endif

#ifndef PIN_GPS_RX
#error "Set PIN_GPS_RX for this board in platformio.ini, for example -D PIN_GPS_RX=16"
#endif

inline HardwareSerial& gpsSerial() {
  return Serial1;
}

// Receive only: TX -1 leaves the UART TX pin unassigned.
inline void gpsBegin() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, -1);
}

// No MISO: the MAX7219 only receives.
inline void spiBegin() {
  SPI.begin(SCK, -1, MOSI, -1);
}

// EEPROM is kept in NVS, buffered in RAM.
inline void storageBegin() {
  EEPROM.begin(STORAGE_SIZE);
}

inline void storageCommit() {
  EEPROM.commit();
}

// random() uses the hardware random number generator until randomSeed() is called.
inline void seedRandom() {}

#else
#error "board.h has no block for this core yet"
#endif

#endif // BOARD_H
