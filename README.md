# LilyGO EPD47 ESP32-S3 E-Paper Display

Dieses Projekt steuert das LilyGO T5 4.7 Zoll E-Paper Display basierend auf dem ESP32-S3 Mikrocontroller. Es vereint eine statische Logodarstellung, eine Zeitanzeige (gesynct per NTP) mit Batteriespannung in einer unteren Statusleiste und eine dynamische, zentrierte Auflistung von Google-Kalender-Terminen.

## Funktionen

1. **E-Paper Rendering**: Native Ansteuerung des Displays über die LilyGO Board-Abstraktion.
2. **NTP Zeitsynchronisierung**: Die aktuelle Uhrzeit wird via WiFi (`pool.ntp.org`) geholt und über die intern eingebundene Echtzeituhr (RTC PCF8563) puffert.
3. **Google Kalender Integration**:
   - Ein dedizierter Client in `src/calendar_client.cpp` holt über WLAN Termine ab.
   - Das Abrufen geschieht über ein Google Apps Script (JSON Web-API), dekodiert über `ArduinoJson`.
4. **Angepasste Benutzeroberfläche (UI)**:
   - **Oben links**: Das eigene Logo (konvertiert als Hex-Bitmap aus `logo.h`).
   - **Mitte**: Bis zu 5 zukünftige Termine (Datum, Uhrzeit und Titel).
   - **Unten**: Statusleiste mit Live-Uhrzeit (links) und der aktuellen Batteriespannung (rechts).
5. **Stromsparmodus (Deep Sleep)**:
   - Der ESP32 läuft nicht mehr durchgehend in einer Schleife.
   - Nach dem Start holt er alle Aktualisierungen, zeichnet den Bildschirm neu und legt sich dann sofort für **1 Stunde** komplett schlafen (`esp_deep_sleep_start()`), um den LiPo-Akku bestmöglich zu schonen.
   - Ein manuelles Aufwecken (Force Update) ist durch Drücken des seitlichen **BOOT-Buttons** möglich.

## Hardware Voraussetzungen

- **LilyGO T5 4.7-inch E-paper (V2.3 / ESP32-S3)**
- LiPo-Akku (falls das Gerät mobil betrieben werden soll)

## Softwareanforderungen / Bibliotheken

Das Projekt ist für **PlatformIO** (unter VS Code oder über CLI) konfiguriert.
Die wesentlichen Abhängigkeiten (automatisch per `platformio.ini` aufgelöst) sind:

- `LilyGo-EPD47` (Inklusive e-Paper-Treiber, Board-Definitionen und GT911-Touch)
- `bblanchon/ArduinoJson` für die Termin-Extraktion
- `WiFi` und `HTTPClient` für die Cloud-Anbindung

## Kompilierung und Upload

```bash
# 1. PlatformIO Umgebung laden oder über die VSCode IDE starten
pio run

# 2. Firmware auf das Board flashen
pio run --target upload

# 3. Serial Monitor öffnen (Baudrate: 115200) um den Calendar-Fetch Log zu betrachten
pio device monitor
```

## Projektstruktur

- `/src/main.cpp`: Das Kernprogramm (WiFi, EPD Rendering, Touch-Abfrage, Timer-Schleifen).

- `/src/calendar_client.*`: Kapselung von HTTPClient und JSON-Parsing, adaptiert aus einem früheren ESP-IDF Projekt.
- `/src/logo.h`: C-Array des im oberen Bereich dargestellten Logos.
- `/boards/`: PlatformIO Custom-Board Konfigurationen für den speziellen E-Paper S3-Chip von LilyGO.
- `platformio.ini`: System-Umgebung, Build-Flags (e.g. `BOARD_HAS_PSRAM`) und Abhängigkeiten.
