#pragma once
#include <Arduino.h>

#define CALENDAR_URL_PLACEHOLDER                                               \
  "https://script.google.com/macros/s/"                                        \
  "AKfycbywT3fR70gxHD-VxnvDcQzF4T37tMra3lSfCd6ju3ta23dZCR07v6nb9ZAwLphm2mX5"  \
  "exec"

typedef struct {
  char title[64];
  char time[32];
  char date[16];
  char calendar[32];
} calendar_event_t;

typedef struct {
  calendar_event_t events[5];
  int count;
} calendar_data_t;

void calendar_client_init(void);

// Fetches the events and populates the out_data struct. Returns 0 on success.
int calendar_client_fetch(const char *url, calendar_data_t *out_data);
