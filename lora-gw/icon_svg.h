#ifndef ICON_SVG_H
#define ICON_SVG_H

#include <pgmspace.h>

const char ICON_SVG[] PROGMEM = R"rawliteral(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
  <defs>
    <linearGradient id="g" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#38bdf8"/>
      <stop offset="100%" stop-color="#818cf8"/>
    </linearGradient>
  </defs>

  <!-- Dot anchor -->
  <circle cx="256" cy="358" r="32" fill="url(#g)"/>

  <!-- Arc 1 — small -->
  <path d="M 168 284 A 110 110 0 0 1 344 284"
        fill="none" stroke="url(#g)" stroke-width="38" stroke-linecap="round"/>

  <!-- Arc 2 — medium -->
  <path d="M 104 218 A 178 178 0 0 1 408 218"
        fill="none" stroke="url(#g)" stroke-width="32" stroke-linecap="round" opacity="0.55"/>

  <!-- Arc 3 — large -->
  <path d="M 40 152 A 244 244 0 0 1 472 152"
        fill="none" stroke="url(#g)" stroke-width="28" stroke-linecap="round" opacity="0.25"/>
</svg>

)rawliteral";

#endif // ICON_SVG_H
