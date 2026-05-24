/**
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co.,
 * Ltd
 * @date      2024-04-05
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 *
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include "epd_driver.h"
#include "esp_adc_cal.h"
#include "firasans.h"
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "calendar_client.h"
#include "utilities.h"
#include <SensorPCF8563.hpp>
#include <TouchDrvGT911.hpp>
#include <WiFi.h>
#include <Wire.h>
#include <esp_sntp.h>

#define WIFI_SSID "TP-Link_IoT_2G"
#define WIFI_PASSWORD "Holger&star1103"
#define PCF8563_SLAVE_ADDRESS 0x51

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const char *time_zone =
    "CET-1CEST,M3.5.0,M10.5.0/3"; // TimeZone rule for Europe/Berlin including
                                  // daylight adjustment rules (optional)

SensorPCF8563 rtc;
TouchDrvGT911 touch;

uint8_t *framebuffer = NULL;
bool touchOnline = false;
uint32_t interval = 0;
uint32_t calendar_interval = 0;
int vref = 1100;
char buf[128];

struct _point {
  uint8_t buttonID;
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
} touchPoint[] = {{0, 10, 10, 80, 80},
                  {1, EPD_WIDTH - 80, 10, 80, 80},
                  {2, 10, EPD_HEIGHT - 80, 80, 80},
                  {3, EPD_WIDTH - 80, EPD_HEIGHT - 80, 80, 80}};

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void timeavailable(struct timeval *t) {
  Serial.println("[WiFi]: Got time adjustment from NTP!");
  rtc.hwClockWrite();
}

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice
   * a year, in such a case time adjustment won't be handled automagicaly.
   */
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  configTzTime(time_zone, ntpServer1, ntpServer2);

  /**
   * SD Card test
   * Only as a test SdCard hardware, use example reference
   * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD/examples
   */
  SPI.begin(SD_SCLK, SD_SCLK, SD_MOSI, SD_CS);
  bool rlst = SD.begin(SD_CS, SPI);
  if (!rlst) {
    Serial.println("SD init failed");
    snprintf(buf, 128, "➸ No detected SdCard 😂");
  } else {
    Serial.println("SD init success");
    snprintf(buf, 128, "➸ Detected SdCard insert:%.2f GB😀",
             SD.cardSize() / 1024.0 / 1024.0 / 1024.0);
  }

  // Correct the ADC reference voltage
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref: %umV\r\n", adc_chars.vref);
    vref = adc_chars.vref;
  }

  framebuffer =
      (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) {
    Serial.println("alloc memory failed !!!");
    while (1)
      ;
  }
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  epd_init();

  epd_poweron();
  epd_clear();
  epd_poweroff();

#if defined(CONFIG_IDF_TARGET_ESP32S3)
  // Assuming that the previous touch was in sleep state, wake it up
  pinMode(TOUCH_INT, OUTPUT);
  digitalWrite(TOUCH_INT, HIGH);

  Serial.println(buf);

  Wire.begin(BOARD_SDA, BOARD_SCL);
  Wire.beginTransmission(PCF8563_SLAVE_ADDRESS);
  if (Wire.endTransmission() == 0) {
    rtc.begin(Wire, BOARD_SDA, BOARD_SCL);
    // rtc.setDateTime(2022, 6, 30, 0, 0, 0);
    Serial.println("➸ RTC is online  😀");
  } else {
    Serial.println("➸ RTC is probe failed!  😂");
  }

  /*
   * The touch reset pin uses hardware pull-up,
   * and the function of setting the I2C device address cannot be used.
   * Use scanning to obtain the touch device address.*/
  uint8_t touchAddress = 0x14;

  Wire.beginTransmission(0x14);
  if (Wire.endTransmission() == 0) {
    touchAddress = 0x14;
  }
  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) {
    touchAddress = 0x5D;
  }

  touch.setPins(-1, TOUCH_INT);
  if (touch.begin(Wire, touchAddress, BOARD_SDA, BOARD_SCL)) {
    touch.setMaxCoordinates(EPD_WIDTH, EPD_HEIGHT);
    touch.setSwapXY(true);
    touch.setMirrorXY(false, true);
    touchOnline = true;
    Serial.println("➸ Touch is online  😀");
  } else {
    Serial.println("➸ Touch is probe failed!  😂");
  }

#endif

  epd_poweroff();
}

void loop() {
  epd_poweron();
  delay(10);

  uint16_t v = analogRead(BATT_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  if (battery_voltage >= 4.2) battery_voltage = 4.2;

  // Full framebuffer reset to white
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  // === Header: envelope icon + title ===
  // Envelope at (30, 12), 80×55 px — double-thickness border and fold lines
  epd_draw_rect(30, 12, 80, 55, 0, framebuffer);
  epd_draw_rect(31, 13, 78, 53, 0, framebuffer);
  // V-fold from top corners to center
  epd_draw_line(30, 12, 70, 34, 0, framebuffer);
  epd_draw_line(31, 13, 70, 34, 0, framebuffer);
  epd_draw_line(110, 12, 70, 34, 0, framebuffer);
  epd_draw_line(109, 13, 70, 34, 0, framebuffer);

  // Title "Holgers Kalender"
  FontProperties title_props = {
      .fg_color = 0, .bg_color = 15, .fallback_glyph = 0, .flags = 0};
  int cx = 125, cy = 58;
  write_mode((GFXfont *)&FiraSans, "Holgers Kalender", &cx, &cy, framebuffer,
             BLACK_ON_WHITE, &title_props);

  // Separator line (2 px)
  epd_draw_hline(30, 78, EPD_WIDTH - 60, 0, framebuffer);
  epd_draw_hline(30, 79, EPD_WIDTH - 60, 0, framebuffer);

  // === Calendar events ===
  // Available area y=88..450 → 362px / 5 events = 72px spacing
  calendar_data_t cal_data;
  if (calendar_client_fetch(CALENDAR_URL_PLACEHOLDER, &cal_data) == 0 &&
      cal_data.count > 0) {
    FontProperties gray_props = {
        .fg_color = 7, .bg_color = 15, .fallback_glyph = 0, .flags = 0};
    FontProperties black_props = {
        .fg_color = 0, .bg_color = 15, .fallback_glyph = 0, .flags = 0};
    for (int i = 0; i < cal_data.count; i++) {
      int line_y = 141 + i * 72;
      char dt_buf[48];
      snprintf(dt_buf, sizeof(dt_buf), "%s  %s", cal_data.events[i].date,
               cal_data.events[i].time);
      cx = 50;
      cy = line_y;
      write_mode((GFXfont *)&FiraSans, dt_buf, &cx, &cy, framebuffer,
                 BLACK_ON_WHITE, &gray_props);
      cx += 25;
      cy = line_y;
      write_mode((GFXfont *)&FiraSans, cal_data.events[i].title, &cx, &cy,
                 framebuffer, BLACK_ON_WHITE, &black_props);
    }
    Serial.printf("Calendar: %d events\n", cal_data.count);
  } else {
    FontProperties gray_props = {
        .fg_color = 8, .bg_color = 15, .fallback_glyph = 0, .flags = 0};
    cx = 50;
    cy = 160;
    write_mode((GFXfont *)&FiraSans, "Keine Termine abrufbar", &cx, &cy,
               framebuffer, BLACK_ON_WHITE, &gray_props);
    Serial.println("Calendar fetch failed.");
  }

  // === Dark status bar ===
  epd_fill_rect(0, 455, EPD_WIDTH, EPD_HEIGHT - 455, 30, framebuffer);

  // Time (white on dark)
  struct tm timeinfo;
  rtc.getDateTime(&timeinfo);
  strftime(buf, sizeof(buf), "%d.%m.%Y    %H:%M:%S", &timeinfo);
  FontProperties white_props = {
      .fg_color = 15, .bg_color = 0, .fallback_glyph = 0, .flags = 0};
  cx = 50;
  cy = 511;
  write_mode((GFXfont *)&FiraSans, buf, &cx, &cy, framebuffer, WHITE_ON_BLACK,
             &white_props);

  // Battery voltage (white on dark, right side)
  char volt_buf[20];
  snprintf(volt_buf, sizeof(volt_buf), "%.2f V", battery_voltage);
  cx = EPD_WIDTH - 190;
  cy = 511;
  write_mode((GFXfont *)&FiraSans, volt_buf, &cx, &cy, framebuffer,
             WHITE_ON_BLACK, &white_props);

  // Push full framebuffer to display
  Rect_t full_area = {.x = 0, .y = 0, .width = EPD_WIDTH, .height = EPD_HEIGHT};
  epd_draw_grayscale_image(full_area, framebuffer);

  // === Power off and deep sleep ===
  epd_poweroff_all();
  Serial.println("Display updated. Entering deep sleep for 1 hour...");
  WiFi.disconnect(true);
  if (touchOnline) touch.sleep();
  delay(100);
  Wire.end();
  Serial.end();
  esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
  esp_sleep_enable_ext1_wakeup(_BV(0), ESP_EXT1_WAKEUP_ANY_LOW);
  esp_deep_sleep_start();
}
