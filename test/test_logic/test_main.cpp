// Unit tests for lib/ClockLogic. They run on the PC: pio test -e native

#include <string.h>
#include <unity.h>

#include "ClockLogic.h"

void setUp() {}
void tearDown() {}

static void test_ordinal_suffix() {
  TEST_ASSERT_EQUAL_STRING("st", ordinalSuffix(1));
  TEST_ASSERT_EQUAL_STRING("nd", ordinalSuffix(2));
  TEST_ASSERT_EQUAL_STRING("rd", ordinalSuffix(3));
  TEST_ASSERT_EQUAL_STRING("th", ordinalSuffix(4));
  TEST_ASSERT_EQUAL_STRING("th", ordinalSuffix(11));
  TEST_ASSERT_EQUAL_STRING("th", ordinalSuffix(12));
  TEST_ASSERT_EQUAL_STRING("th", ordinalSuffix(13));
  TEST_ASSERT_EQUAL_STRING("st", ordinalSuffix(21));
  TEST_ASSERT_EQUAL_STRING("nd", ordinalSuffix(22));
  TEST_ASSERT_EQUAL_STRING("rd", ordinalSuffix(23));
  TEST_ASSERT_EQUAL_STRING("st", ordinalSuffix(31));
}

static void assertDigits(const char* expected, const TimeDigits& d) {
  char actual[7] = { (char)d.hourTens, (char)d.hourOnes, (char)d.minTens,
                     (char)d.minOnes, (char)d.secTens, (char)d.secOnes, '\0' };
  TEST_ASSERT_EQUAL_STRING(expected, actual);
}

static void test_to_digits_12_hour() {
  assertDigits("120509", toDigits(0, 5, 9, false));    // midnight shows 12
  assertDigits("120000", toDigits(12, 0, 0, false));   // noon shows 12
  assertDigits("014500", toDigits(13, 45, 0, false));
  assertDigits("115959", toDigits(23, 59, 59, false));
}

static void test_to_digits_24_hour() {
  assertDigits("000509", toDigits(0, 5, 9, true));
  assertDigits("134500", toDigits(13, 45, 0, true));
  assertDigits("235959", toDigits(23, 59, 59, true));
}

static void test_trimmed_mean_keeps_all_up_to_five() {
  float values[] = { 3, 1, 2 };
  TEST_ASSERT_EQUAL_FLOAT(2.0f, trimmedMean(values, 3));
  float five[] = { 10, 0, 0, 0, 0 };
  TEST_ASSERT_EQUAL_FLOAT(2.0f, trimmedMean(five, 5));
}

static void test_trimmed_mean_drops_outliers() {
  // 10 readings: after sorting, 2 are dropped at each end, so all 4
  // spikes go. Without the sort, or dropping only 1 per end, it fails.
  float values[] = { 1, 100, 1, -100, 1, 90, 1, -90, 1, 1 };
  TEST_ASSERT_EQUAL_FLOAT(1.0f, trimmedMean(values, 10));
}

static void test_trimmed_mean_empty() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, trimmedMean(nullptr, 0));
}

static void test_next_boot_count() {
  TEST_ASSERT_EQUAL_UINT32(1, nextBootCount(0xFFFFFFFF, 5));   // blank EEPROM
  TEST_ASSERT_EQUAL_UINT32(1, nextBootCount(0, 5));
  TEST_ASSERT_EQUAL_UINT32(4, nextBootCount(3, 5));
  TEST_ASSERT_EQUAL_UINT32(11, nextBootCount(10, 5));          // 2 x threshold is still valid
  TEST_ASSERT_EQUAL_UINT32(1, nextBootCount(11, 5));           // above that counts as 0
}

static void test_format_fixed() {
  char buf[16];
  formatFixed(buf, sizeof(buf), 12.97159f, 4);
  TEST_ASSERT_EQUAL_STRING("12.9716", buf);
  formatFixed(buf, sizeof(buf), 77.5946f, 4);
  TEST_ASSERT_EQUAL_STRING("77.5946", buf);
  formatFixed(buf, sizeof(buf), -0.1278f, 4);    // minus sign between -1 and 0
  TEST_ASSERT_EQUAL_STRING("-0.1278", buf);
  formatFixed(buf, sizeof(buf), -122.4194f, 4);
  TEST_ASSERT_EQUAL_STRING("-122.4194", buf);
  formatFixed(buf, sizeof(buf), -0.00004f, 4);   // rounds to zero, no minus
  TEST_ASSERT_EQUAL_STRING("0.0000", buf);
  formatFixed(buf, sizeof(buf), 9.99996f, 4);    // carry into the whole part
  TEST_ASSERT_EQUAL_STRING("10.0000", buf);
  formatFixed(buf, sizeof(buf), 3018.37f, 1);
  TEST_ASSERT_EQUAL_STRING("3018.4", buf);
  formatFixed(buf, sizeof(buf), 12.0507f, 4);    // zeros right after the point
  TEST_ASSERT_EQUAL_STRING("12.0507", buf);
}

static void test_format_fixed_stays_in_buffer() {
  char buf[6];
  formatFixed(buf, sizeof(buf), 12.9716f, 4);
  TEST_ASSERT_EQUAL_STRING("12.97", buf);
}

static void test_format_date() {
  char buf[32];
  formatDate(buf, sizeof(buf), 3, 25, 3, 2026);
  TEST_ASSERT_EQUAL_STRING("WED 25th MAR 2026", buf);
  formatDate(buf, sizeof(buf), 0, 1, 1, 2026);
  TEST_ASSERT_EQUAL_STRING("SUN 1st JAN 2026", buf);
  formatDate(buf, sizeof(buf), 9, 2, 13, 2026);   // out of range values are clamped
  TEST_ASSERT_EQUAL_STRING("SAT 2nd DEC 2026", buf);
  formatDate(buf, sizeof(buf), 1, 3, 0, 2026);
  TEST_ASSERT_EQUAL_STRING("MON 3rd JAN 2026", buf);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ordinal_suffix);
  RUN_TEST(test_to_digits_12_hour);
  RUN_TEST(test_to_digits_24_hour);
  RUN_TEST(test_trimmed_mean_keeps_all_up_to_five);
  RUN_TEST(test_trimmed_mean_drops_outliers);
  RUN_TEST(test_trimmed_mean_empty);
  RUN_TEST(test_next_boot_count);
  RUN_TEST(test_format_fixed);
  RUN_TEST(test_format_fixed_stays_in_buffer);
  RUN_TEST(test_format_date);
  return UNITY_END();
}
