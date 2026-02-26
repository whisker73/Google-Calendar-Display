#pragma once
#include <Arduino.h>

#define CALENDAR_URL_PLACEHOLDER                                               \
  "https://script.google.com/macros/s/"                                        \
  "AKfycbzJNm_C5rlaq1K9xiUE7IvjK8j72zKnEIM3m3EcpqZuh4JlvjgUusx52qX4OeFzXzOL/"  \
  "exec"

typedef struct {
  char title[64];
  char time[32];
  char date[16];
} calendar_event_t;

typedef struct {
  calendar_event_t events[5];
  int count;
} calendar_data_t;

void calendar_client_init(void);

// Fetches the events and populates the out_data struct. Returns 0 on success.
int calendar_client_fetch(const char *url, calendar_data_t *out_data);
