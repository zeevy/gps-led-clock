/**
 * @file ClockLogic.cpp
 * @brief Clock logic that needs no hardware (see ClockLogic.h)
 *
 * @license MIT
 */

#include "ClockLogic.h"

#include <stdio.h>

static const char* const WEEKDAY_NAMES[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
static const char* const MONTH_NAMES[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                             "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

const char* ordinalSuffix(int number) {
  int lastTwoDigits = number % 100;
  if (lastTwoDigits >= 11 && lastTwoDigits <= 13) {
    return "th";
  }
  switch (number % 10) {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    default: return "th";
  }
}

TimeDigits toDigits(uint8_t hour, uint8_t minute, uint8_t second, bool is24h) {
  if (!is24h) {
    hour %= 12;
    if (hour == 0) hour = 12;
  }

  TimeDigits digits;
  digits.hourTens = '0' + hour / 10;
  digits.hourOnes = '0' + hour % 10;
  digits.minTens = '0' + minute / 10;
  digits.minOnes = '0' + minute % 10;
  digits.secTens = '0' + second / 10;
  digits.secOnes = '0' + second % 10;
  return digits;
}

float trimmedMean(float* values, uint8_t count) {
  if (count == 0) return 0;

  // Insertion sort: the GPS filter holds only a dozen readings
  for (uint8_t i = 1; i < count; i++) {
    float value = values[i];
    uint8_t j = i;
    while (j > 0 && values[j - 1] > value) {
      values[j] = values[j - 1];
      j--;
    }
    values[j] = value;
  }

  uint8_t skip = count > 5 ? count / 5 : 0;
  float sum = 0;
  for (uint8_t i = skip; i < count - skip; i++) {
    sum += values[i];
  }
  return sum / (count - 2 * skip);
}

uint32_t nextBootCount(uint32_t stored, uint32_t threshold) {
  return (stored > threshold * 2 ? 0 : stored) + 1;
}

void formatFixed(char* buf, size_t size, float value, uint8_t decimals) {
  uint32_t scale = 1;
  for (uint8_t i = 0; i < decimals; i++) {
    scale *= 10;
  }

  // Round on the magnitude, so the sign is kept for values between -1 and 0
  bool negative = value < 0;
  float magnitude = negative ? -value : value;
  uint32_t scaled = (uint32_t)(magnitude * scale + 0.5f);
  const char* sign = (negative && scaled != 0) ? "-" : "";
  unsigned long whole = scaled / scale;

  // Zero padded fraction without the "*" width, which avr-libc lacks:
  // 716 with scale 10000 prints as "10716", shown without its first digit
  char fraction[12];
  snprintf(fraction, sizeof(fraction), "%lu", (unsigned long)(scale + scaled % scale));
  snprintf(buf, size, "%s%lu.%s", sign, whole, fraction + 1);
}

void formatDate(char* buf, size_t size, uint8_t weekday, uint8_t day, uint8_t month, uint16_t year) {
  if (weekday > 6) weekday = 6;
  if (month < 1) month = 1;
  if (month > 12) month = 12;

  snprintf(buf, size, "%s %u%s %s %u", WEEKDAY_NAMES[weekday], (unsigned)day, ordinalSuffix(day),
           MONTH_NAMES[month - 1], (unsigned)year);
}
