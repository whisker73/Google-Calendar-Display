#include "calendar_client.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

void calendar_client_init(void) {
  // Nothing required for now
}

int calendar_client_fetch(const char *url, calendar_data_t *out_data) {
  if (!out_data)
    return -1;

  HTTPClient http;
  // Set timeout to 10 seconds (10000 ms) and allow following redirects
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.print("[Calendar] Fetching: ");
  Serial.println(url);

  if (http.begin(url)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[Calendar] GET code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        // Parse response
        String payload = http.getString();

        // We use dynamic document because the array size could vary
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print(F("[Calendar] deserializeJson() failed: "));
          Serial.println(error.f_str());
          http.end();
          return -1;
        }

        out_data->count = 0;

        // Handle JSON Array vs Object fallback just as in ESP-IDF project
        if (doc.is<JsonArray>()) {
          JsonArray array = doc.as<JsonArray>();
          for (JsonObject item : array) {
            if (out_data->count >= 5)
              break; // Maximum 5 items

            // Extract fields with fallbacks
            const char *title = item["title"] | "No Title";
            const char *t_time = item["time"] | "--:--";
            const char *date = item["date"] | "";

            strlcpy(out_data->events[out_data->count].title, title,
                    sizeof(out_data->events[out_data->count].title));
            strlcpy(out_data->events[out_data->count].time, t_time,
                    sizeof(out_data->events[out_data->count].time));
            strlcpy(out_data->events[out_data->count].date, date,
                    sizeof(out_data->events[out_data->count].date));

            out_data->count++;
          }
        } else if (doc.is<JsonObject>()) {
          // Fallback for single element
          JsonObject item = doc.as<JsonObject>();
          const char *title = item["title"] | "No Events";
          const char *t_time = item["time"] | "--:--";
          const char *date = item["date"] | "";

          strlcpy(out_data->events[0].title, title,
                  sizeof(out_data->events[0].title));
          strlcpy(out_data->events[0].time, t_time,
                  sizeof(out_data->events[0].time));
          strlcpy(out_data->events[0].date, date,
                  sizeof(out_data->events[0].date));
          out_data->count = 1;
        }

        http.end();
        return 0; // Success
      }
    } else {
      Serial.printf("[Calendar] GET failed, error: %s\n",
                    http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("[Calendar] Unable to connect");
  }

  return -1;
}
