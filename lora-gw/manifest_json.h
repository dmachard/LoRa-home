#ifndef MANIFEST_JSON_H
#define MANIFEST_JSON_H

#include <pgmspace.h>

const char MANIFEST_JSON[] PROGMEM = R"rawliteral(
{
  "name": "LoRa@Home Gateway",
  "short_name": "LoRa@Home",
  "description": "LoRa Gateway Dashboard - Monitor your sensor nodes in real-time",
  "start_url": "/",
  "display": "standalone",
  "background_color": "#0f172a",
  "theme_color": "#0f172a",
  "orientation": "any",
  "scope": "/",
  "icons": [
    {
      "src": "/icon.svg",
      "sizes": "any",
      "type": "image/svg+xml",
      "purpose": "any maskable"
    }
  ],
  "categories": ["utilities", "productivity"],
  "shortcuts": [
    {
      "name": "Dashboard",
      "url": "/",
      "description": "Open the LoRa node dashboard"
    }
  ]
}

)rawliteral";

#endif // MANIFEST_JSON_H
