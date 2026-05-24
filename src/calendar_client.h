#pragma once
#include <Arduino.h>

#define CALENDAR_URL_PLACEHOLDER                                               \
  "https://script.google.com/macros/s/"                                        \
  "AKfycbx7DtUlkL3k8_9netohaRBES9gMAVGSppqiRAF2ZaO0hdaeKYQUkZiZQ-f1cw_pbJqM"  \
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
