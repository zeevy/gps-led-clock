/**
 * @file ClockLogic.h
 * @brief Clock logic that needs no hardware
 *
 * Time digits, date text, number formatting, the GPS filter mean and the
 * power cycle counter. This library must not include Arduino.h, so it
 * also builds and runs as unit tests on a PC: pio test -e native
 *
 * Use only printf features that avr-libc has (no %f, no * width). The PC
 * printf has them, so the unit tests cannot catch this; check the display
 * text in Wokwi or on the board.
 *
 * @license MIT
 */

#ifndef CLOCK_LOGIC_H
#define CLOCK_LOGIC_H

#include <stddef.h>
#include <stdint.h>

/**
 * @struct TimeDigits
 * @brief The digits of a time as ASCII characters, for comparing and
 * animating digit changes. A space means "nothing shown yet".
 */
struct TimeDigits {
  uint8_t hourOnes = ' ';
  uint8_t hourTens = ' ';
  uint8_t minOnes = ' ';
  uint8_t minTens = ' ';
  uint8_t secOnes = ' ';
  uint8_t secTens = ' ';
};

/**
 * @brief Ordinal suffix for a day number: "st", "nd", "rd" or "th"
 * (11th, 12th and 13th use "th")
 */
const char* ordinalSuffix(int number);

/**
 * @brief Splits a time into display digits
 * @param hour Hour in 24 hour form (0 to 23)
 * @param is24h false shows 12 hour form: 0 and 12 show as 12, 13 as 01
 */
TimeDigits toDigits(uint8_t hour, uint8_t minute, uint8_t second, bool is24h);

/**
 * @brief Mean of the middle values: sorts values in place, drops the
 * lowest and highest 20% when there are more than 5, and averages the rest
 * @param values Readings; their order is changed
 * @param count Number of readings; 0 returns 0
 */
float trimmedMean(float* values, uint8_t count);

/**
 * @brief Power cycle count to store for this boot
 * @param stored Value read from EEPROM. A blank EEPROM (0xFFFFFFFF) or any
 * value above 2 x threshold counts as 0
 */
uint32_t nextBootCount(uint32_t stored, uint32_t threshold);

/**
 * @brief Writes value rounded to the given decimals, for example
 * -0.1278 with 4 decimals gives "-0.1278". Never writes "-0.0000".
 * @param decimals 1 to 9
 */
void formatFixed(char* buf, size_t size, float value, uint8_t decimals);

/**
 * @brief Writes a date like "WED 25th MAR 2026"
 * @param weekday 0 = Sunday; values above 6 show as Saturday
 * @param month 1 = January; values out of 1 to 12 are clamped
 */
void formatDate(char* buf, size_t size, uint8_t weekday, uint8_t day, uint8_t month, uint16_t year);

#endif // CLOCK_LOGIC_H
