# Google Calendar Display

Ein Google-Kalender-Dashboard auf dem **LilyGO T5 4.7" E-Paper** (ESP32-S3). Das Display zeigt die nächsten fünf Termine aus allen Google-Kalendern — inklusive freigegebener Kalender — mit Geschlechts-Symbol je Kalender-Besitzer, Datum, Uhrzeit und Titel. Danach legt sich der ESP32 für eine Stunde schlafen.

## Funktionen

- **Mehrere Kalender**: Alle eigenen und freigegebenen Google-Kalender werden zusammengeführt und chronologisch sortiert
- **Kalender-Symbole**: Festes ♂-Symbol für den Hauptkalender, ♀-Symbol für freigegebene Kalender (konfigurierbar per E-Mail-Adresse)
- **Header**: Gezeichnetes Briefumschlag-Icon + Titel
- **Statusleiste**: Datum/Uhrzeit (NTP + RTC PCF8563) und Batteriespannung
- **Deep Sleep**: Nach jedem Refresh schläft der ESP32 1 Stunde; BOOT-Button weckt manuell auf
- **Credentials ausgelagert**: WLAN-Daten und Script-URL liegen in `src/secrets.h` (gitignored)

## Hardware

- LilyGO T5 4.7" E-Paper V2.3 (ESP32-S3)
- LiPo-Akku (optional, für mobilen Betrieb)

## Einrichtung

### 1. Secrets anlegen

```bash
cp src/secrets.h.example src/secrets.h
```

`src/secrets.h` befüllen:

```cpp
#define WIFI_SSID     "dein-wlan-name"
#define WIFI_PASSWORD "dein-wlan-passwort"

#define CALENDAR_URL \
  "https://script.google.com/macros/s/DEINE_SCRIPT_ID/exec"
```

### 2. Google Apps Script deployen

1. [script.google.com](https://script.google.com) öffnen → Neues Projekt
2. Inhalt von `google_apps_script.js` einfügen
3. **Deployen → Neue Bereitstellung → Web-App**
   - Ausführen als: „Ich"
   - Zugriff: „Jeder"
4. Die Bereitstellungs-URL in `src/secrets.h` als `CALENDAR_URL` eintragen

> Die Script-URL ändert sich bei jeder neuen Bereitstellung — danach `secrets.h` aktualisieren und neu flashen.

### 3. Kompilieren und flashen

```bash
pio run                    # kompilieren
pio run --target upload    # flashen (Board per USB verbinden)
pio device monitor         # serieller Monitor, 115200 Baud
```

> Falls das Board im Deep Sleep ist: USB abziehen und wieder einstecken, oder BOOT-Button gedrückt halten beim Anschließen.

## Projektstruktur

```
src/
├── main.cpp              # Haupt-Logik: WiFi, EPD-Rendering, Deep Sleep
├── calendar_client.cpp   # HTTP-Fetch + ArduinoJson-Parsing
├── calendar_client.h     # Datenstrukturen (calendar_event_t, calendar_data_t)
├── secrets.h             # Credentials — nicht eingecheckt (gitignored)
└── secrets.h.example     # Vorlage für secrets.h
google_apps_script.js     # Google Apps Script (Calendar-API → JSON)
platformio.ini            # PlatformIO Board-Konfiguration und Abhängigkeiten
boards/                   # Custom Board-Definition für LilyGO EPD S3
```

## Abhängigkeiten

Werden automatisch über `platformio.ini` aufgelöst:

- `LilyGo-EPD47` — E-Paper-Treiber, Board-Abstraktionen, GT911-Touch
- `bblanchon/ArduinoJson` — JSON-Parsing
- `WiFi`, `HTTPClient` — WLAN und HTTP

## Anpassen

**Kalender-Symbol-Zuordnung**: In `src/main.cpp` die Erkennung in `loop()` anpassen — aktuell wird per `strstr(email, "anja")` zwischen ♂ und ♀ entschieden.

**Anzahl Termine**: In `google_apps_script.js` den `limit`-Wert ändern (aktuell 5), sowie `events[5]` in `calendar_client.h`.

**Refresh-Intervall**: In `src/main.cpp` den Wert in `esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL)` anpassen (Angabe in Mikrosekunden).
