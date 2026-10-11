/**
 * @file board.h
 * @brief Hardware abstraction layer: pins and board specific calls
 *
 * The app code uses only the pins and functions defined here. Each
 * supported core has one block below. A new board on a supported core
 * needs only a PlatformIO env; a new core needs a new block here.
 * Any pin can be changed per env with a build flag, for example
 * -D PIN_GPS_RX=7.
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

/**
 * @brief GPS port on its own pins, so D0 and D1 stay free for USB upload
 * and the debug log. Built on first use, not before setup().
 */
inline NeoSWSerial& gpsSerial() {
  static NeoSWSerial port(PIN_GPS_RX, PIN_GPS_TX);
  return port;
}

/** @brief Starts the SPI bus for the matrix. Call before ledMatrix.begin(). */
inline void spiBegin() {
  SPI.begin();
}

/** @brief Seeds random() from the noise of the unconnected A0 pin. */
inline void seedRandom() {
  randomSeed(analogRead(A0));
}

#else
#error "board.h has no block for this core yet"
#endif

#endif // BOARD_H
