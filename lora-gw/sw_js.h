#ifndef SW_JS_H
#define SW_JS_H

#include <pgmspace.h>

const char SW_JS[] PROGMEM = R"rawliteral(
// LoRa@Home Gateway - Service Worker
// Network-first strategy for API calls, cache-first for static assets

const CACHE_NAME = 'lora-home-v5';
const STATIC_ASSETS = ['/'];

// Install: cache the shell
self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(CACHE_NAME).then(cache => cache.addAll(STATIC_ASSETS))
  );
  self.skipWaiting();
});

// Activate: clean old caches
self.addEventListener('activate', event => {
  event.waitUntil(
    caches.keys().then(keys =>
      Promise.all(keys.filter(k => k !== CACHE_NAME).map(k => caches.delete(k)))
    )
  );
  self.clients.claim();
});

// Fetch strategy
self.addEventListener('fetch', event => {
  const url = new URL(event.request.url);

  // Bypass Service Worker completely for pages requiring HTTP Basic Auth (/admin, /update)
  if (url.pathname.startsWith('/admin') || url.pathname.startsWith('/update')) {
    return; // Let browser handle native HTTP basic auth popup directly
  }

  // Network-first for API endpoints (always fresh data)
  if (url.pathname.startsWith('/api/') || url.pathname === '/metrics') {
    event.respondWith(
      fetch(event.request).catch(() =>
        new Response(JSON.stringify({ error: 'offline' }), {
          headers: { 'Content-Type': 'application/json' }
        })
      )
    );
    return;
  }

  // Cache-first for static shell (/, /icon.svg, /manifest.json)
  event.respondWith(
    caches.match(event.request).then(cached => {
      if (cached) return cached;
      return fetch(event.request).then(response => {
        if (response && response.status === 200) {
          const clone = response.clone();
          caches.open(CACHE_NAME).then(cache => cache.put(event.request, clone));
        }
        return response;
      });
    })
  );
});

)rawliteral";

#endif // SW_JS_H
