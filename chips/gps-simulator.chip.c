/**
 * @file gps-simulator.chip.c
 * @brief Wokwi custom chip that simulates a NEO-6M GPS module
 *
 * Sends valid NMEA GPRMC and GPGGA sentences once per second over UART.
 * Simulates a GPS module with a fix near Bangalore, India (IST timezone).
 *
 * Coordinates: 12.9716 N, 77.5946 E, altitude 920m (~3018ft)
 * NMEA format: lat = 1258.2960,N  lon = 07735.6760,E
 *
 * The chip waits a few seconds before sending valid fixes to briefly
 * show the rain effect (no-signal state) during startup.
 */

#include "wokwi-api.h"

#define GPS_BAUD_RATE       115200
#define FIX_DELAY_SECONDS   5

static uart_dev_t uart;
static uint32_t elapsed_seconds = 0;

/* Simple string helpers to avoid pulling in libc snprintf */

static uint32_t str_len(const char *s) {
  uint32_t n = 0;
  while (s[n]) n++;
  return n;
}

static void str_copy(char *dst, const char *src) {
  while (*src) *dst++ = *src++;
  *dst = '\0';
}

static void str_append(char *dst, const char *src) {
  dst += str_len(dst);
  str_copy(dst, src);
}

static void append_char(char *dst, char c) {
  uint32_t n = str_len(dst);
  dst[n] = c;
  dst[n + 1] = '\0';
}

/* Append a zero-padded 2-digit number */
static void append_2d(char *dst, uint8_t val) {
  append_char(dst, '0' + (val / 10));
  append_char(dst, '0' + (val % 10));
}

/* Compute NMEA checksum (XOR of all chars between '$' and '*') */
static uint8_t nmea_checksum(const char *body) {
  uint8_t cs = 0;
  for (uint32_t i = 0; body[i]; i++) {
    cs ^= (uint8_t)body[i];
  }
  return cs;
}

static const char hex[] = "0123456789ABCDEF";

/* Send a complete NMEA sentence: $<body>*<checksum>\r\n */
static void send_nmea(const char *body) {
  char buf[128];
  uint8_t cs = nmea_checksum(body);

  buf[0] = '$';
  buf[1] = '\0';
  str_append(buf, body);
  append_char(buf, '*');
  append_char(buf, hex[(cs >> 4) & 0x0F]);
  append_char(buf, hex[cs & 0x0F]);
  append_char(buf, '\r');
  append_char(buf, '\n');

  uart_write(uart, (uint8_t *)buf, str_len(buf));
}

/* Build time string "HHMMSS.00" from seconds since fix start */
static void build_time_str(char *out, uint32_t sim_time) {
  uint8_t hours   = (sim_time / 3600) % 24;
  uint8_t minutes = (sim_time / 60) % 60;
  uint8_t seconds = sim_time % 60;

  out[0] = '\0';
  append_2d(out, hours);
  append_2d(out, minutes);
  append_2d(out, seconds);
  str_append(out, ".00");
}

static void on_timer(void *user_data) {
  elapsed_seconds++;

  /* Simulate cold-start delay - no fix for the first few seconds */
  if (elapsed_seconds < FIX_DELAY_SECONDS) {
    send_nmea("GPRMC,,V,,,,,,,,,,N");
    send_nmea("GPGGA,,,,,,0,00,99.9,,,,,,");
    return;
  }

  /* Simulated UTC time incrementing each second */
  char time_str[16];
  build_time_str(time_str, elapsed_seconds - FIX_DELAY_SECONDS);

  /*
   * GPRMC - Recommended Minimum Navigation Information
   * Date: 250326 = 25 Mar 2026
   */
  char rmc[128];
  rmc[0] = '\0';
  str_append(rmc, "GPRMC,");
  str_append(rmc, time_str);
  str_append(rmc, ",A,1258.2960,N,07735.6760,E,0.0,0.0,250326,0.0,E,A");
  send_nmea(rmc);

  /*
   * GPGGA - GPS Fix Data
   * Alt: 920.0 M (~3018 ft)
   */
  char gga[128];
  gga[0] = '\0';
  str_append(gga, "GPGGA,");
  str_append(gga, time_str);
  str_append(gga, ",1258.2960,N,07735.6760,E,1,08,0.9,920.0,M,0.0,M,,");
  send_nmea(gga);
}

void chip_init(void) {
  uart_config_t uart_config = {
    .tx = pin_init("TX", OUTPUT),
    .rx = pin_init("RX", INPUT),
    .baud_rate = GPS_BAUD_RATE,
  };
  uart = uart_init(&uart_config);

  timer_config_t timer_config = {
    .callback = on_timer,
    .user_data = (void *)0,
  };
  timer_t timer = timer_init(&timer_config);
  timer_start(timer, 1000000, true); /* Fire every 1 second */
}
