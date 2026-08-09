#ifndef ADMIN_HTML_H
#define ADMIN_HTML_H

#include <pgmspace.h>

const char ADMIN_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Administration - LoRa@Home</title> <style>@import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@300;400;500;600;700;800&display=swap');:root{--bg:#0f172a;--text:#f8fafc;--text-title:#fff;--text-h1:#f8fafc;--header-border:rgba(255,255,255,.05);--pill-bg:rgba(255,255,255,.05);--pill-border:rgba(255,255,255,.08);--pill-color:#cbd5e1;--card-bg:rgba(30,41,59,.5);--card-border-def:rgba(255,255,255,.05);--card-shadow:0 4px 6px -1px rgba(0,0,0,.2);--val-color:#f8fafc;--label-color:#64748b;--divider-border:rgba(255,255,255,.05);--footer-border:rgba(255,255,255,.04);--footer-color:#64748b;--btn-bg:rgba(255,255,255,.06);--btn-border:rgba(255,255,255,.12);--btn-color:#f8fafc}[data-theme="light"]{--bg:#f8fafc;--text:#0f172a;--text-title:#0f172a;--text-h1:linear-gradient(135deg,#0f172a 0%,#334155 100%);--header-border:#e2e8f0;--pill-bg:#fff;--pill-border:#cbd5e1;--pill-color:#334155;--card-bg:#fff;--card-border-def:#e2e8f0;--card-shadow:0 10px 30px -5px rgba(0,0,0,.06),0 4px 12px -2px rgba(0,0,0,.03);--val-color:#0f172a;--label-color:#64748b;--divider-border:#f1f5f9;--footer-border:#e2e8f0;--footer-color:#64748b;--btn-bg:#fff;--btn-border:#cbd5e1;--btn-color:#0f172a}a{text-decoration:none}body{font-family:'Plus Jakarta Sans',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);margin:0;padding:24px;min-height:100vh}.container{max-width:1200px;margin:0 auto}header{display:flex;justify-content:space-between;align-items:center;margin-bottom:32px;border-bottom:1px solid var(--header-border);padding-bottom:20px}h1{font-size:26px;font-weight:800;background:var(--text-h1);-webkit-background-clip:text;-webkit-text-fill-color:transparent;margin:0}.header-status-bar{display:inline-flex;align-items:center;gap:8px;background:var(--pill-bg);border:1px solid var(--pill-border);padding:4px 10px;border-radius:6px;font-size:12px;font-weight:600;color:var(--pill-color);margin-top:6px}.status-item{display:inline-flex;align-items:center;gap:5px}.status-sep{opacity:.35;font-size:10px}.altitude-pill input::-webkit-inner-spin-button,.altitude-pill input::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}.altitude-pill input{-moz-appearance:textfield}.pill-dot{width:7px;height:7px;border-radius:50%;background:#10b981;box-shadow:0 0 8px rgba(16,185,129,.6)}.header-actions{display:flex;align-items:center;gap:8px}.stat-card{background:var(--card-bg);backdrop-filter:blur(8px);border:1px solid var(--card-border-def);border-radius:8px;padding:24px;display:flex;flex-direction:column;box-shadow:var(--card-shadow)}.gw-stat-card{background:rgba(17,24,39,.4);backdrop-filter:blur(24px);border:1px solid rgba(255,255,255,.05);border-radius:8px;padding:16px 20px;display:flex;align-items:center;justify-content:space-between;box-shadow:0 10px 25px -5px rgba(0,0,0,.3)}.stat-info{display:flex;flex-direction:column}.stat-val{font-size:22px;font-weight:800;color:#fff;margin-top:2px;letter-spacing:-.01em}.stat-label{font-size:10px;color:#64748b;text-transform:uppercase;letter-spacing:.08em;font-weight:700}.signal-bars{display:flex;align-items:flex-end;gap:3px;height:18px}.signal-bar{width:4px;background:rgba(255,255,255,.15);border-radius:2px}.signal-bar.active{background:#38bdf8}.signal-bar:nth-child(1){height:30%}.signal-bar:nth-child(2){height:50%}.signal-bar:nth-child(3){height:70%}.signal-bar:nth-child(4){height:100%}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:20px;align-items:start}.node-card{background:var(--card-bg);backdrop-filter:blur(8px);border:1px solid var(--card-border,var(--card-border-def));border-radius:8px;padding:24px;box-shadow:var(--card-shadow);transition:all .3s cubic-bezier(.4,0,.2,1);display:flex;flex-direction:column;position:relative;overflow:hidden;box-sizing:border-box}.node-card:hover{border-color:var(--card-border-hover,rgba(255,255,255,.15))}.node-card.offline{border-color:rgba(239,68,68,.15);background:var(--card-bg);opacity:.65}.node-card.offline:hover{border-color:rgba(239,68,68,.35);opacity:.85}.node-card.warning{border-color:rgba(245,158,11,.35)}.node-header{display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:4px}.node-title-group{display:flex;flex-direction:column}.node-name{font-size:20px;font-weight:800;color:var(--text-title);letter-spacing:-.02em}.node-badge{padding:3px 8px;border-radius:4px;font-size:10px;font-weight:700;letter-spacing:.05em;text-transform:uppercase;border:1px solid transparent}.node-readings{display:grid;grid-template-columns:repeat(2,1fr);gap:16px 20px;padding:16px 0;border-top:1px solid var(--divider-border);margin-bottom:12px}.metric-item{display:flex;flex-direction:column;gap:4px;min-width:0}.metric-value-wrapper{display:flex;align-items:center;gap:8px}.metric-icon{width:18px;height:18px;flex-shrink:0}.metric-icon.temp{color:#f43f5e}.metric-icon.hum{color:#0ea5e9}.metric-icon.press{color:#8b5cf6}.metric-icon.light{color:#f59e0b}.metric-icon.co2{color:#10b981}.metric-icon.batt{color:#10b981}.metric-value{font-size:20px;font-weight:700;color:var(--val-color);letter-spacing:-.01em}.metric-value.text-val{font-size:16px;font-weight:700;letter-spacing:0}.metric-unit{font-size:13px;font-weight:600;color:var(--label-color);margin-left:2px}.metric-label{font-size:10px;color:var(--label-color);font-weight:700;text-transform:uppercase;letter-spacing:.05em}.node-footer{display:flex;flex-direction:column;gap:10px;padding-top:12px;border-top:1px solid var(--footer-border);font-size:11px;color:var(--footer-color)}.node-footer-simple{display:flex;align-items:center;justify-content:space-between;gap:8px;font-size:11px;color:var(--footer-color)}.footer-metric-simple{display:inline-flex;align-items:center;gap:6px}.footer-metric-simple b{color:var(--val-color)}.btn-diag-toggle{background:var(--btn-bg);border:1px solid var(--btn-border);color:var(--pill-color);padding:3px 8px;border-radius:4px;font-size:11px;font-weight:600;cursor:pointer;display:inline-flex;align-items:center;gap:4px;transition:all .15s ease}.btn-diag-toggle:hover{background:rgba(255,255,255,.12);color:var(--text-title)}.node-diag-drawer{margin-top:10px;padding:10px 12px;background:rgba(0,0,0,.03);border:1px solid var(--divider-border);border-radius:6px;font-size:11px}[data-theme="dark"] .node-diag-drawer{background:rgba(0,0,0,.25)}.diag-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:6px 12px}.diag-item{display:flex;align-items:center;gap:4px;color:var(--footer-color)}.diag-item b{color:var(--val-color)}.node-radio-icon{width:14px;height:14px;color:#64748b}.stat-divider{color:rgba(255,255,255,.08);font-weight:bold}.radio-badge{font-size:8px;font-weight:700;padding:1px 5px;border-radius:4px;text-transform:uppercase;letter-spacing:.03em;margin-left:6px;display:inline-block}.radio-badge.rssi-strong,.radio-badge.snr-clean{background:rgba(16,185,129,.1);color:#34d399;border:1px solid rgba(16,185,129,.15)}.radio-badge.rssi-medium,.radio-badge.snr-noisy{background:rgba(245,158,11,.1);color:#fbbf24;border:1px solid rgba(245,158,11,.15)}.radio-badge.rssi-weak,.radio-badge.snr-poor{background:rgba(239,68,68,.1);color:#f87171;border:1px solid rgba(239,68,68,.15)}.status-indicator{display:flex;align-items:center;gap:8px}.dot{width:8px;height:8px;border-radius:50%;background:#10b981;box-shadow:0 0 10px rgba(16,185,129,.8);position:relative}.dot::after{content:'';position:absolute;top:0;left:0;width:100%;height:100%;border-radius:50%;background:inherit;animation:dot-pulse 1.8s infinite ease-in-out}.dot.offline{background:#ef4444;box-shadow:0 0 10px rgba(239,68,68,.8)}.dot.warning{background:#f59e0b;box-shadow:0 0 10px rgba(245,158,11,.8)}@keyframes dot-pulse{0%{transform:scale(1);opacity:.8}100%{transform:scale(2.4);opacity:0}}.header-btn,.btn-lang-toggle,.btn-theme-toggle{background:var(--pill-bg);color:var(--pill-color);border:1px solid var(--pill-border);padding:0 12px;height:34px;border-radius:6px;font-weight:600;font-size:12px;letter-spacing:.01em;cursor:pointer;display:inline-flex;align-items:center;justify-content:center;gap:6px;transition:all .15s ease;backdrop-filter:blur(12px);white-space:nowrap;box-sizing:border-box;text-decoration:none !important}.header-btn:hover,.btn-lang-toggle:hover,.btn-theme-toggle:hover{background:rgba(255,255,255,.12);border-color:rgba(255,255,255,.22);color:var(--text-title);transform:translateY(-1px)}[data-theme="light"] .header-btn,[data-theme="light"] .btn-lang-toggle,[data-theme="light"] .btn-theme-toggle{background:#fff;border:1px solid #cbd5e1;color:#334155}[data-theme="light"] .header-btn:hover,[data-theme="light"] .btn-lang-toggle:hover,[data-theme="light"] .btn-theme-toggle:hover{background:#f8fafc;border-color:#94a3b8;color:#0f172a}.header-btn.btn-primary{background:#0284c7;color:#fff !important;border:1px solid #0369a1;box-shadow:0 2px 6px rgba(2,132,199,.25)}.header-btn.btn-primary:hover{background:#0369a1;border-color:#075985}.header-btn.btn-icon-only{padding:0 10px}.header-btn:active,.btn-lang-toggle:active,.btn-theme-toggle:active{transform:translateY(0)}.btn-update{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);color:white;border:none;padding:8px 14px;border-radius:6px;font-weight:700;text-decoration:none;font-size:12px;transition:all .2s;box-shadow:0 4px 12px rgba(239,68,68,.2)}.btn-update:hover{transform:translateY(-1px);box-shadow:0 6px 16px rgba(239,68,68,.3)}.btn-metrics{background:var(--btn-bg);color:var(--pill-color);border:1px solid var(--btn-border);padding:8px 14px;border-radius:6px;font-weight:700;text-decoration:none;font-size:13px;margin-right:8px;transition:all .2s}.btn-metrics:hover{color:var(--text-title);border-color:var(--btn-border);opacity:.85}.pulse-ring{width:12px;height:12px;background:#38bdf8;border-radius:50%;box-shadow:0 0 0 0 rgba(56,189,248,.7);animation:pulse 1.6s infinite;margin:0 auto 12px}@keyframes pulse{0%{transform:scale(.95);box-shadow:0 0 0 0 rgba(56,189,248,.7)}70%{transform:scale(1);box-shadow:0 0 0 10px rgba(56,189,248,0)}100%{transform:scale(.95);box-shadow:0 0 0 0 rgba(56,189,248,0)}}.tabs{display:flex;gap:8px;margin-bottom:28px}.tab-btn{background:rgba(255,255,255,.05);border:1px solid rgba(255,255,255,.08);color:#94a3b8;padding:8px 16px;border-radius:6px;font-weight:700;cursor:pointer;transition:all .2s;font-size:13px}.tab-btn:hover{background:rgba(255,255,255,.1);color:#fff}.tab-btn.active{background:#38bdf8;color:#fff;border-color:transparent}.tab-content{display:none}.tab-content.active{display:block}.form-group{margin-bottom:20px}label{display:block;font-size:11px;font-weight:600;text-transform:uppercase;color:#64748b;margin-bottom:8px;letter-spacing:.05em}input[type="text"],input[type="number"],select{width:100%;background:rgba(15,23,42,.6);border:1px solid rgba(255,255,255,.08);padding:10px 14px;border-radius:6px;color:#f8fafc;font-size:13px;transition:all .2s;box-sizing:border-box}input:focus,select:focus{border-color:#38bdf8;outline:none;box-shadow:0 0 0 3px rgba(56,189,248,.15)}.btn{background:#38bdf8;color:#fff;border:none;padding:8px 16px;border-radius:6px;font-weight:600;cursor:pointer;transition:all .2s;font-size:13px;display:inline-flex;align-items:center;justify-content:center;text-decoration:none !important}.btn:hover{transform:translateY(-1px);opacity:.95}.btn:disabled{opacity:.5;cursor:not-allowed;transform:none}.btn-secondary{background:rgba(255,255,255,.08);color:#fff;border:1px solid rgba(255,255,255,.1);box-shadow:none}.btn-secondary:hover{background:rgba(255,255,255,.12)}.btn-danger{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);box-shadow:0 4px 12px rgba(239,68,68,.2)}.btn-success{background:linear-gradient(135deg,#10b981 0%,#059669 100%);box-shadow:0 4px 12px rgba(16,185,129,.2)}.hidden{display:none !important}.alert-notice{background:rgba(239,68,68,.1);border:1px solid rgba(239,68,68,.2);color:#fca5a5;padding:18px;border-radius:16px;margin-bottom:24px;font-size:13px;line-height:1.6}.alert-notice a{color:#38bdf8;text-decoration:underline;font-weight:600}.body-center{display:flex;align-items:center;justify-content:center}.card-update{background:linear-gradient(145deg,rgba(30,41,59,.5) 0%,rgba(15,23,42,.75) 100%);backdrop-filter:blur(20px);border:1px solid rgba(255,255,255,.05);border-radius:24px;padding:32px;width:100%;max-width:450px;box-shadow:0 20px 40px -15px rgba(0,0,0,.5);text-align:center}.card-update h3{font-size:20px;font-weight:800;margin-top:0;margin-bottom:24px;background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent}.card-update.error h3{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent}.card-update.success h3{background:linear-gradient(135deg,#10b981 0%,#059669 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent}.card-update p{color:#94a3b8;font-size:14px;margin-bottom:16px}.file-input-wrapper{margin-bottom:24px}input[type="file"]{display:none}.custom-file-upload{display:inline-block;padding:12px 24px;background:rgba(15,23,42,.6);border:1px dashed rgba(56,189,248,.4);border-radius:12px;cursor:pointer;font-size:14px;color:#38bdf8;transition:all .2s;width:80%}.custom-file-upload:hover{background:rgba(56,189,248,.1);border-color:#38bdf8}.btn-submit{background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);border:none;color:#fff;padding:12px 28px;font-size:14px;font-weight:600;border-radius:12px;cursor:pointer;box-shadow:0 4px 12px rgba(56,189,248,.2);transition:all .2s}.btn-submit:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(56,189,248,.35)}.btn-submit:active{transform:translateY(0)}.file-name{margin-top:8px;font-size:12px;color:#64748b}.btn-back{display:inline-block;margin-top:16px;font-size:13px;color:#64748b;text-decoration:none;transition:color .2s}.btn-back:hover{color:#f8fafc}.radio-scanner{position:relative;width:72px;height:72px;display:flex;align-items:center;justify-content:center;background:rgba(14,165,233,.08);border:1px solid rgba(14,165,233,.2);border-radius:50%;margin-bottom:4px;box-shadow:0 0 25px rgba(14,165,233,.15)}.radio-tower-icon{width:36px;height:36px;color:#38bdf8;filter:drop-shadow(0 0 8px rgba(56,189,248,.6))}.radio-tower-icon .wave{animation:radio-wave-pulse 2s infinite ease-in-out}.radio-tower-icon .wave-1{animation-delay:0s}.radio-tower-icon .wave-2{animation-delay:.6s}@keyframes radio-wave-pulse{0%,100%{opacity:.25}50%{opacity:1}}.scanning-card{grid-column:1 / -1;text-align:center;padding:60px 40px;color:#94a3b8;justify-content:center;display:flex;flex-direction:column;align-items:center;gap:16px}.scanning-text{display:flex;align-items:center;gap:4px;font-weight:700;font-size:13px;letter-spacing:.05em;text-transform:uppercase;color:#38bdf8}.loading-banner{display:flex;align-items:center;gap:12px;background:rgba(14,165,233,.1);border:1px solid rgba(14,165,233,.3);color:#38bdf8;padding:16px 20px;border-radius:14px;font-weight:600;font-size:14px;margin-bottom:24px;box-shadow:0 4px 15px rgba(0,0,0,.2);transition:all .3s ease}.spinner-icon{width:20px;height:20px;animation:spin-anim 1.2s linear infinite;flex-shrink:0}@keyframes spin-anim{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}@media (max-width:768px){body{padding:12px}header{flex-direction:column;align-items:stretch;gap:16px;margin-bottom:20px;padding-bottom:16px}.header-actions{width:100%;justify-content:space-between}.header-status-bar{flex-wrap:wrap;gap:6px;margin-top:10px}.status-pill{font-size:11px;padding:4px 10px}.grid{grid-template-columns:1fr;gap:16px}.node-card{padding:18px;border-radius:16px}.node-name{font-size:18px}.node-readings{display:grid;grid-template-columns:repeat(2,1fr);gap:18px 12px;padding:16px 0}.metric-item{min-width:0}.metric-value{font-size:18px}.node-footer{flex-direction:column;align-items:flex-start;gap:8px}.node-footer-right{align-items:flex-start}}@media (max-width:480px){h1{font-size:22px}.header-actions{flex-wrap:wrap;gap:8px}.btn-metrics,.btn,.btn-theme-toggle,.btn-lang-toggle{font-size:12px;padding:8px 12px}.node-readings{grid-template-columns:repeat(2,1fr);gap:14px 8px}}.legend-panel{background:var(--card-bg);border:1px solid var(--card-border-def);border-radius:8px;padding:16px 20px;margin-bottom:24px;box-shadow:var(--card-shadow);backdrop-filter:blur(12px)}.legend-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px}.legend-header h3{margin:0;font-size:15px;font-weight:700;color:var(--text-title)}.legend-close{background:none;border:none;color:var(--label-color);font-size:16px;cursor:pointer;padding:4px 8px}.legend-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:16px}.legend-item{display:flex;gap:12px;align-items:flex-start;font-size:12px;line-height:1.5}.legend-item p{margin:2px 0 0 0;color:var(--label-color)}.legend-icon{font-size:18px;line-height:1}.logs-card-header{user-select:none;transition:opacity .2s ease}.logs-card-header:hover{opacity:.9}.btn-logs-toggle{background:rgba(56,189,248,.1);border:1px solid rgba(56,189,248,.25);color:#38bdf8;padding:5px 12px;border-radius:20px;font-size:11px;font-weight:600;display:inline-flex;align-items:center;gap:6px;transition:all .2s ease}[data-theme="light"] .btn-logs-toggle{background:rgba(2,132,199,.08);border:1px solid rgba(2,132,199,.2);color:#0284c7}.logs-card-header:hover .btn-logs-toggle{background:rgba(56,189,248,.2);border-color:rgba(56,189,248,.4);transform:translateY(-1px)}[data-theme="light"] .logs-card-header:hover .btn-logs-toggle{background:rgba(2,132,199,.15);border-color:rgba(2,132,199,.35)}.badge-count{background:rgba(255,255,255,.1);color:var(--pill-color);padding:2px 6px;border-radius:4px;font-size:11px;font-weight:700;margin-left:4px;min-width:14px;text-align:center;display:inline-block;transition:all .2s ease}.badge-count.has-errors{background:#ef4444;color:#fff;box-shadow:0 0 8px rgba(239,68,68,.6)}</style> </head><body><div class="container"><header><div><h1 data-i18n="admin_title">Administration LoRa@Home</h1><div style="font-size:12px; color:#64748b; margin-top:4px;" data-i18n="admin_subtitle">System configuration and node provisioning</div></div><div style="display:flex; align-items:center; gap:8px;"><button id="lang-toggle" class="btn-lang-toggle" title="Switch Language" onclick="toggleLanguage()">🇫🇷 FR</button><a href="/metrics" class="btn-metrics" data-i18n="btn_metrics">Prometheus Metrics</a><a href="/update" class="btn-update" data-i18n="btn_ota">Firmware Update</a><a href="/" class="btn btn-secondary" style="margin-left:8px;" data-i18n="btn_back_dash">← Dashboard</a></div></header><div class="tabs"><button class="tab-btn active" id="tab-btn-gw-config" onclick="switchTab('gw-config')" data-i18n="tab_gw_config">Gateway Configuration</button><button class="tab-btn" id="tab-btn-ble" onclick="switchTab('ble')" data-i18n="tab_node_config">Node Configuration (BLE)</button></div><div id="tab-gw-config" class="tab-content active"><div id="gw-ble-security-notice" class="alert-notice hidden"><strong data-i18n="ble_notice_title">⚠️ Bluetooth disabled by browser (Security)</strong><br><span data-i18n="ble_notice_desc">The browser requires a secure connection (HTTPS or localhost) to allow Bluetooth.<br> To use local BLE, open this page locally (double-click) or access the local IP via HTTP.</span></div><div id="gw-ble-container" style="margin-bottom: 24px;"><div class="stat-card"><h2 style="margin-top:0; font-size: 18px; font-weight: 700;" data-i18n="gw_ble_title">Bluetooth Low Energy Connection (Gateway)</h2><p style="font-size: 13px; color: #94a3b8; margin-bottom: 16px;" data-i18n="gw_ble_desc"> Put the gateway into config mode (press the BOOT button or start it without config; the blue LED will start flashing), then connect via local BLE below. </p><div style="display: flex; gap: 16px; align-items: center;"><button type="button" class="btn" id="btn-gw-connect" data-i18n="btn_connect_gw">Connect to Gateway</button><div style="display: flex; align-items: center; gap: 8px; font-size: 14px;"><span id="gw-status-dot" class="dot offline"></span><span id="gw-status-text" style="font-weight:600; color:#ef4444;" data-i18n="status_disconnected">Disconnected</span></div></div></div></div><form id="gw-config-form" class="hidden"><div id="gw-loading-banner" class="loading-banner hidden"><svg class="spinner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="2" x2="12" y2="6"/><line x1="12" y1="18" x2="12" y2="22"/><line x1="4.93" y1="4.93" x2="7.76" y2="7.76"/><line x1="16.24" y1="16.24" x2="19.07" y2="19.07"/><line x1="2" y1="12" x2="6" y2="12"/><line x1="18" y1="12" x2="22" y2="12"/><line x1="4.93" y1="19.07" x2="7.76" y2="16.24"/><line x1="16.24" y1="7.76" x2="19.07" y2="4.93"/></svg><span data-i18n="fetching_gw_params">Fetching gateway configuration parameters...</span></div><div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr)); gap: 24px; align-items: start;"><div style="display: flex; flex-direction: column; gap: 24px;"><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid var(--divider-border); padding-bottom:10px; margin-bottom:15px;" data-i18n="gw_sec_wifi">WiFi Network & Security</h3><div class="form-group"><label for="gw_wifi_ssid" data-i18n="gw_wifi_ssid">Network Name (SSID)</label><input type="text" id="gw_wifi_ssid" required placeholder="e.g. MyWiFiNetwork"></div><div class="form-group"><label for="gw_wifi_pass" data-i18n="gw_wifi_pass">WiFi Password</label><input type="text" id="gw_wifi_pass" required placeholder="WPA Key"></div><div class="form-group" style="margin-top:20px; border-top:1px solid var(--divider-border); padding-top:16px;"><label for="gw_admin_pass" data-i18n="gw_admin_pass">Administration Password (HTTP Auth)</label><input type="text" id="gw_admin_pass" required placeholder="admin" value="admin"><span style="font-size:10px; color:#64748b; display:block; margin-top:4px;" data-i18n="gw_admin_default">Default: admin</span></div></div><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid var(--divider-border); padding-bottom:10px; margin-bottom:15px;" data-i18n="gw_sec_ip">IP Configuration</h3><div class="form-group" style="display: flex; align-items: center; gap: 10px; margin-bottom: 15px;"><input type="checkbox" id="gw_use_static" style="width:18px; height:18px; cursor:pointer;"><label for="gw_use_static" style="margin-bottom:0; cursor:pointer;" data-i18n="gw_use_static">Use static IP address</label></div><div id="gw-static-ip-fields" style="display: none;"><div class="form-group"><label for="gw_local_ip" data-i18n="gw_local_ip">Local IP Address</label><input type="text" id="gw_local_ip" placeholder="e.g. 192.168.1.100"></div><div class="form-group"><label for="gw_gateway_ip" data-i18n="gw_gateway_ip">Default Gateway</label><input type="text" id="gw_gateway_ip" placeholder="e.g. 192.168.1.1"></div><div class="form-group"><label for="gw_subnet_mask" data-i18n="gw_subnet_mask">Subnet Mask</label><input type="text" id="gw_subnet_mask" placeholder="e.g. 255.255.255.0"></div></div></div><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid var(--divider-border); padding-bottom:10px; margin-bottom:15px;" data-i18n="gw_sec_loc">Station Location & Environment</h3><div class="form-group" style="margin-bottom:0;"><label for="gw_station_alt" data-i18n="gw_station_alt">Station Altitude (meters above sea level)</label><input type="number" id="gw_station_alt" min="0" max="9000" placeholder="e.g. 96" value="96" required><span style="font-size:10px; color:#64748b; display:block; margin-top:4px;" data-i18n="gw_station_alt_desc">Used for Sea-Level Barometric Pressure (QNH) calculation</span></div></div></div><div style="display: flex; flex-direction: column; gap: 24px;"><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid var(--divider-border); padding-bottom:10px; margin-bottom:15px;" data-i18n="gw_sec_hw">Gateway Radio Hardware</h3><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="gw_lora_chip" data-i18n="gw_lora_chip">Gateway Transceiver Chip</label><select id="gw_lora_chip" required><option value="2">SX1262</option><option value="1">SX1278</option></select></div><div class="form-group"><label for="gw_lora_sync" data-i18n="gw_lora_sync">Sync Word (Hex)</label><input type="text" id="gw_lora_sync" required placeholder="0x12" value="0x12"></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="gw_lora_freq" data-i18n="gw_lora_freq">Frequency (MHz)</label><select id="gw_lora_freq" required><option value="433.0">433.0 MHz (Europe/Asia)</option><option value="868.1">868.1 MHz (EU868 Ch 1)</option><option value="868.3">868.3 MHz (EU868 Ch 2)</option><option value="868.5">868.5 MHz (EU868 Ch 3)</option><option value="869.525">869.525 MHz (EU868 Max Power)</option><option value="915.0">915.0 MHz (Americas)</option></select></div><div class="form-group"><label for="gw_lora_bw" data-i18n="gw_lora_bw">Bandwidth (kHz)</label><select id="gw_lora_bw" required><option value="7.8">7.8</option><option value="10.4">10.4</option><option value="15.6">15.6</option><option value="20.8">20.8</option><option value="31.25">31.25</option><option value="41.7">41.7</option><option value="62.5">62.5</option><option value="125.0">125.0</option><option value="250.0">250.0</option><option value="500.0">500.0</option></select></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="gw_lora_sf" data-i18n="gw_lora_sf">Spreading Factor (SF)</label><input type="number" id="gw_lora_sf" min="6" max="12" value="9" required></div><div class="form-group"><label for="gw_lora_cr" data-i18n="gw_lora_cr">Coding Rate (CR)</label><input type="number" id="gw_lora_cr" min="5" max="8" value="5" required></div></div><div class="form-group" style="margin-top:12px;"><label for="gw_aes_key" data-i18n="gw_aes_key">Shared AES Key (32 hex chars / 16 bytes)</label><input type="text" id="gw_aes_key" maxlength="32" minlength="32" required placeholder="Hexadecimal AES Key"></div><button type="button" class="btn btn-secondary" onclick="generateRandomGwKey()" style="width:100%; margin-top:4px;" data-i18n="btn_gen_key">Generate Random Key</button></div><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid var(--divider-border); padding-bottom:10px; margin-bottom:12px;">🇪🇺 Repères Réglementation 868 MHz (ETSI / Europe)</h3><div style="overflow-x: auto; border: 1px solid var(--divider-border); border-radius: 6px;"><table style="width: 100%; border-collapse: collapse; font-size: 11px; text-align: left;"><thead><tr style="background: rgba(0,0,0,0.12); color: var(--label-color); border-bottom: 1px solid var(--divider-border);"><th style="padding: 6px 8px;">Bande</th><th style="padding: 6px 8px;">Fréquences</th><th style="padding: 6px 8px;">P Max</th><th style="padding: 6px 8px;">Duty Cycle</th><th style="padding: 6px 8px;">Usage</th></tr></thead><tbody style="color: var(--pill-color);"><tr style="border-bottom: 1px solid var(--divider-border);"><td style="padding: 6px 8px; font-weight:700;">Bande M</td><td style="padding: 6px 8px;">868,0 – 868,6 MHz</td><td style="padding: 6px 8px; color: #fbbf24; font-weight:700;">14 dBm</td><td style="padding: 6px 8px;">1% (36s/h)</td><td style="padding: 6px 8px;">LoRaWAN (868.1, 868.3, 868.5)</td></tr><tr><td style="padding: 6px 8px; font-weight:700;">Bande P</td><td style="padding: 6px 8px;">869,4 – 869,65 MHz</td><td style="padding: 6px 8px; color: #34d399; font-weight:700;">27 dBm</td><td style="padding: 6px 8px;">10% (360s/h)</td><td style="padding: 6px 8px;">Privé haute portée (869.525)</td></tr></tbody></table></div><div style="font-size:10px; color:#fbbf24; margin-top:10px; line-height:1.4;"> ⚠️ <b>Sync Word :</b><code>0x12</code> (18) = Réseau privé (LoRa@Home). N'utiliser <code>0x34</code> que pour LoRaWAN public. </div></div></div></div><div style="display: flex; justify-content: flex-end; margin-top: 24px; border-top: 1px solid var(--divider-border); padding-top: 20px;"><button type="submit" id="btn-save-gw-submit" class="header-btn btn-primary" style="height: 42px; font-size: 14px; padding: 0 32px; font-weight: 700; cursor: pointer;" data-i18n="btn_save_gw">Save Configuration</button></div><div class="stat-card" style="margin-top: 32px; border-color: rgba(239, 68, 68, 0.25); background: rgba(239, 68, 68, 0.04);"><div style="display: flex; justify-content: space-between; align-items: center; gap: 16px; flex-wrap: wrap;"><div><h4 style="margin: 0; font-size: 14px; font-weight: 700; color: #f87171;">Danger Zone</h4><p style="margin: 4px 0 0 0; font-size: 12px; color: #94a3b8;">Reset all gateway parameters and credentials to factory default settings.</p></div><button type="button" class="btn btn-danger" id="btn-gw-reset" style="height: 38px; font-size: 12px; padding: 0 18px;" data-i18n="btn_factory_reset">Factory Reset</button></div></div></form></div><div id="tab-ble" class="tab-content"><div id="ble-security-notice" class="alert-notice hidden"><strong data-i18n="ble_notice_title">⚠️ Bluetooth disabled by browser (Security)</strong><br><span data-i18n="ble_notice_desc">The browser requires a secure connection (HTTPS or localhost) to allow Bluetooth.<br> To use local BLE, open this page locally (double-click) or access the local IP via HTTP.</span></div><div id="ble-container"><div class="stat-card" style="margin-bottom: 24px;"><h2 style="margin-top:0; font-size: 18px; font-weight: 700;" data-i18n="node_ble_title">Bluetooth Low Energy Connection (Node)</h2><p style="font-size: 13px; color: #94a3b8; margin-bottom: 16px;" data-i18n="node_ble_desc"> Put the LoRa node into configuration mode (by pressing the BOOT button once; the blue LED will start flashing), then connect below. </p><div style="display: flex; gap: 16px; align-items: center;"><button type="button" class="btn" id="btn-connect" data-i18n="btn_connect_node">Connect to Node</button><div style="display: flex; align-items: center; gap: 8px; font-size: 14px;"><span id="status-dot" class="dot offline"></span><span id="status-text" style="font-weight:600; color:#ef4444;" data-i18n="status_disconnected">Disconnected</span></div></div></div><form id="config-form" class="hidden"><div id="node-loading-banner" class="loading-banner hidden"><svg class="spinner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="2" x2="12" y2="6"/><line x1="12" y1="18" x2="12" y2="22"/><line x1="4.93" y1="4.93" x2="7.76" y2="7.76"/><line x1="16.24" y1="16.24" x2="19.07" y2="19.07"/><line x1="2" y1="12" x2="6" y2="12"/><line x1="18" y1="12" x2="22" y2="12"/><line x1="4.93" y1="19.07" x2="7.76" y2="16.24"/><line x1="16.24" y1="7.76" x2="19.07" y2="4.93"/></svg><span data-i18n="fetching_node_params">Connecting & fetching node parameters over Bluetooth...</span></div><div class="grid"><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;" data-i18n="node_sec_identity">Node Identity</h3><div class="form-group"><label for="node_id" data-i18n="node_id">Unique Node ID</label><input type="number" id="node_id" min="1" max="254" required></div><div class="form-group"><label for="node_name" data-i18n="node_name">Node Name</label><input type="text" id="node_name" maxlength="15" placeholder="e.g. LivingRoom" required></div><div class="form-group"><label for="tx_interval" data-i18n="tx_interval">Transmission Interval (sec)</label><input type="number" id="tx_interval" min="5" max="3600" required></div></div><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;" data-i18n="node_sec_radio">LoRa Radio Settings</h3><div class="form-group" style="margin-bottom:12px;"><label for="lora_chip" data-i18n="lora_chip">LoRa Radio Chip</label><select id="lora_chip" required><option value="2">SX1262</option><option value="1">SX1278</option></select></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_freq" data-i18n="lora_freq">Frequency (MHz)</label><select id="lora_freq" required><option value="433.0">433.0 MHz (Europe/Asia)</option><option value="868.1">868.1 MHz (EU868 Ch 1)</option><option value="868.3">868.3 MHz (EU868 Ch 2)</option><option value="868.5">868.5 MHz (EU868 Ch 3)</option><option value="869.525">869.525 MHz (EU868 Max Power)</option><option value="915.0">915.0 MHz (Americas)</option></select></div><div class="form-group"><label for="lora_bw" data-i18n="lora_bw">Bandwidth (kHz)</label><select id="lora_bw" required><option value="7.8">7.8</option><option value="10.4">10.4</option><option value="15.6">15.6</option><option value="20.8">20.8</option><option value="31.25">31.25</option><option value="41.7">41.7</option><option value="62.5">62.5</option><option value="125.0">125.0</option><option value="250.0">250.0</option><option value="500.0">500.0</option></select></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_sf" data-i18n="lora_sf">Spreading Factor (SF)</label><input type="number" id="lora_sf" min="6" max="12" required></div><div class="form-group"><label for="lora_cr" data-i18n="lora_cr">Coding Rate (CR)</label><input type="number" id="lora_cr" min="5" max="8" required></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_sync" data-i18n="lora_sync">Sync Word (Hex)</label><input type="text" id="lora_sync" required><span style="font-size:10px; color:#fbbf24; display:block; margin-top:4px;">⚠️ 0x12 (18) = Réseau privé (LoRa@Home). N'utiliser 0x34 que pour LoRaWAN public.</span></div><div class="form-group"><label for="lora_power" data-i18n="lora_power">TX Power (dBm)</label><input type="number" id="lora_power" min="2" max="20" required></div></div><div class="form-group"><label for="lora_preamble" data-i18n="lora_preamble">LoRa Preamble</label><input type="number" id="lora_preamble" min="6" max="65535" required></div></div><div class="stat-card"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;" data-i18n="node_sec_security">Node Security & Update</h3><div class="form-group"><label for="aes_key" data-i18n="node_aes_key">Shared AES Key (32 hex chars / 16 bytes)</label><input type="text" id="aes_key" maxlength="32" minlength="32" required placeholder="AES Encryption Key"></div><button type="button" class="btn btn-secondary" onclick="generateRandomKey()" style="width:100%; margin-bottom: 20px;" data-i18n="btn_gen_key">Generate Random Key</button><div style="border-top: 1px solid rgba(255,255,255,0.05); padding-top: 16px;"><h4 style="margin-top:0; font-size: 13px; color:#f8fafc; font-weight:700;" data-i18n="node_ota_title">Node OTA Update</h4><div class="form-group" style="margin-top:12px;"><label for="ota-file" class="custom-file-upload" style="text-align:center;" data-i18n="label_choose_fw">Choose firmware (.bin)</label><input type="file" id="ota-file" accept=".bin"><div id="ota-file-name" class="file-name" data-i18n="no_file_selected">No file selected</div></div><button type="button" class="btn btn-success" id="btn-start-ota" style="display:none; width:100%; margin-top:8px;" data-i18n="btn_start_ota">Start OTA Update</button><div id="ota-progress-container" style="display:none; margin-top: 16px;"><div style="display:flex; justify-content:space-between; font-size:12px; margin-bottom:6px;"><span id="ota-status-label" style="color:#94a3b8;" data-i18n="uploading_file">Uploading file...</span><span id="ota-percent" style="font-weight:700; color:#38bdf8;">0%</span></div><div style="background:rgba(0,0,0,0.3); border-radius:10px; height:8px; overflow:hidden;"><div id="ota-bar" style="background:linear-gradient(90deg, #38bdf8, #818cf8); width:0%; height:100%; transition:width 0.1s;"></div></div></div></div></div></div><div style="display: flex; justify-content: space-between; align-items: center; margin-top: 24px; border-top: 1px solid var(--divider-border); padding-top: 20px; flex-wrap: wrap; gap: 12px;"><button type="button" class="btn btn-secondary" id="btn-fetch-logs" onclick="fetchNodeLogs()" style="height: 42px; font-size: 13px; padding: 0 16px;" data-i18n="btn_fetch_logs">Fetch Diagnostic Logs 📋</button><button type="submit" id="btn-save-node-submit" class="header-btn btn-primary" style="height: 42px; font-size: 14px; padding: 0 28px; font-weight: 700; cursor: pointer;" data-i18n="btn_save_node">Save Node Configuration</button></div><div class="stat-card" style="margin-top: 32px; border-color: rgba(239, 68, 68, 0.25); background: rgba(239, 68, 68, 0.04);"><div style="display: flex; justify-content: space-between; align-items: center; gap: 16px; flex-wrap: wrap;"><div><h4 style="margin: 0; font-size: 14px; font-weight: 700; color: #f87171;">Danger Zone</h4><p style="margin: 4px 0 0 0; font-size: 12px; color: #94a3b8;">Reset node memory and restore factory default settings.</p></div><button type="button" class="btn btn-danger" id="btn-reset" style="height: 38px; font-size: 12px; padding: 0 18px;" data-i18n="btn_reset_node">Factory Reset Node</button></div></div><div id="node-log-container" style="display:none; margin-top:20px;" class="stat-card"><h3 style="margin-top:0; font-size:14px; color:#38bdf8;" data-i18n="node_logs_title">Node Diagnostic Logs (BLE)</h3><pre id="node-log-console" style="background:#0f172a; color:#34d399; padding:12px; border-radius:10px; font-family:monospace; font-size:11px; max-height:160px; overflow-y:auto; margin:0; border:1px solid #1e293b;"></pre></div></form></div></div></div> <script>const translations = {
en: {
admin_title: "LoRa@Home Administration",
admin_subtitle: "System configuration and node provisioning",
btn_metrics: "Prometheus Metrics",
btn_ota: "Firmware Update",
btn_back_dash: "← Dashboard",
tab_gw_config: "LoRa@Home Configuration",
tab_node_config: "Node Configuration (BLE)",
ble_notice_title: "⚠️ Bluetooth disabled by browser (Security)",
ble_notice_desc: "The browser requires a secure connection (HTTPS or localhost) to allow Bluetooth.<br>To use local BLE, open this page locally (double-click) or access the local IP via HTTP.",
gw_ble_title: "Bluetooth Low Energy Connection (Gateway)",
gw_ble_desc: "Put the gateway into config mode (press the BOOT button or start it without config; the blue LED will start flashing), then connect via local BLE below.",
btn_connect_gw: "Connect to Gateway",
btn_disconnect: "Disconnect",
status_disconnected: "Disconnected",
status_connected: "Connected",
status_searching: "Searching...",
status_reading: "Reading parameters...",
fetching_gw_params: "Fetching gateway configuration parameters...",
gw_sec_wifi: "WiFi Network & Security",
gw_wifi_ssid: "Network Name (SSID)",
gw_wifi_pass: "WiFi Password",
gw_admin_pass: "Administration Password (HTTP Auth)",
gw_admin_default: "Default: admin",
gw_sec_ip: "IP Configuration",
gw_use_static: "Use static IP address",
gw_local_ip: "Local IP Address",
gw_gateway_ip: "Default Gateway",
gw_subnet_mask: "Subnet Mask",
gw_sec_loc: "Station Location & Environment",
gw_station_alt: "Station Altitude (meters above sea level)",
gw_station_alt_desc: "Used for Sea-Level Barometric Pressure (QNH) calculation",
gw_sec_hw: "Gateway Radio Hardware",
gw_lora_chip: "Gateway Transceiver Chip",
gw_lora_freq: "Gateway LoRa Frequency (MHz)",
gw_lora_bw: "Gateway Bandwidth (kHz)",
gw_lora_sf: "Gateway Spreading Factor (SF)",
gw_lora_cr: "Gateway Coding Rate (CR)",
gw_lora_sync: "Gateway Sync Word (Hex)",
gw_aes_key: "Shared AES Key (32 hex chars / 16 bytes)",
btn_gen_key: "Generate Random Key",
btn_save_gw: "Save Configuration",
btn_factory_reset: "Factory Reset",
node_ble_title: "Bluetooth Low Energy Connection (Node)",
node_ble_desc: "Put the LoRa node into configuration mode (by pressing the BOOT button once; the blue LED will start flashing), then connect below.",
btn_connect_node: "Connect to Node",
fetching_node_params: "Connecting & fetching node parameters over Bluetooth...",
node_sec_identity: "Node Identity",
node_id: "Unique Node ID",
node_name: "Node Name",
tx_interval: "Transmission Interval (sec)",
node_sec_radio: "LoRa Radio Settings",
lora_chip: "LoRa Radio Chip",
lora_freq: "Frequency (MHz)",
lora_bw: "Bandwidth (kHz)",
lora_sf: "Spreading Factor (SF)",
lora_cr: "Coding Rate (CR)",
lora_sync: "Sync Word (Hex)",
lora_power: "TX Power (dBm)",
lora_preamble: "LoRa Preamble",
node_sec_security: "Node Security & Update",
node_aes_key: "Shared AES Key (32 hex chars / 16 bytes)",
node_ota_title: "Node OTA Update",
label_choose_fw: "Choose firmware (.bin)",
no_file_selected: "No file selected",
btn_start_ota: "Start OTA Update",
uploading_file: "Uploading file...",
btn_save_node: "Save Node Configuration",
btn_fetch_logs: "Fetch Diagnostic Logs 📋",
btn_reset_node: "Factory Reset Node",
node_logs_title: "Node Diagnostic Logs (BLE)",
alert_reset_gw: "Are you sure you want to reset the gateway to factory default settings?",
alert_reset_node: "Are you sure you want to reset the node?",
alert_start_ota: "Start node OTA update?",
alert_no_logs: "No log entries recorded on node yet.",
alert_connect_first: "Please connect to the node via BLE first!",
msg_fetching_logs: "Fetching logs from node..."
},
fr: {
admin_title: "Administration LoRa@Home",
admin_subtitle: "Configuration système et avitaillement des nœuds",
btn_metrics: "Métriques Prometheus",
btn_ota: "Mise à jour Firmware",
btn_back_dash: "← Tableau de bord",
tab_gw_config: "Configuration LoRa@Home",
tab_node_config: "Configuration Nœud (BLE)",
ble_notice_title: "⚠️ Bluetooth désactivé par le navigateur (Sécurité)",
ble_notice_desc: "Le navigateur requiert une connexion sécurisée (HTTPS ou localhost) pour autoriser le Bluetooth.<br>Pour utiliser le BLE local, ouvrez cette page localement ou accédez à l'IP via HTTP.",
gw_ble_title: "Connexion Bluetooth Low Energy (Passerelle)",
gw_ble_desc: "Mettez la passerelle en mode config (appuyez sur le bouton BOOT ou démarrez sans config ; la LED bleue clignotera), puis connectez-vous via BLE ci-dessous.",
btn_connect_gw: "Se connecter à la passerelle",
btn_disconnect: "Déconnecter",
status_disconnected: "Déconnecté",
status_connected: "Connecté",
status_searching: "Recherche...",
status_reading: "Lecture des paramètres...",
fetching_gw_params: "Récupération des paramètres de la passerelle...",
gw_sec_wifi: "Réseau WiFi & Sécurité",
gw_wifi_ssid: "Nom du réseau (SSID)",
gw_wifi_pass: "Mot de passe WiFi",
gw_admin_pass: "Mot de passe d'administration (Auth HTTP)",
gw_admin_default: "Par défaut : admin",
gw_sec_ip: "Configuration IP",
gw_use_static: "Utiliser une adresse IP statique",
gw_local_ip: "Adresse IP locale",
gw_gateway_ip: "Passerelle par défaut",
gw_subnet_mask: "Masque de sous-réseau",
gw_sec_loc: "Localisation & Environnement de la station",
gw_station_alt: "Altitude de la station (mètres au-dessus du niveau de la mer)",
gw_station_alt_desc: "Utilisé pour le calcul de la pression barométrique au niveau de la mer (QNH)",
gw_sec_hw: "Matériel Radio de la Passerelle",
gw_lora_chip: "Puce émetteur-récepteur de la passerelle",
gw_lora_freq: "Fréquence LoRa Passerelle (MHz)",
gw_lora_bw: "Largeur de bande Passerelle (kHz)",
gw_lora_sf: "Facteur d'étalement Passerelle (SF)",
gw_lora_cr: "Taux de codage Passerelle (CR)",
gw_lora_sync: "Mot de passe Réseau Passerelle (Sync Word)",
gw_aes_key: "Clé AES partagée (32 caract. hex / 16 octets)",
btn_gen_key: "Générer une clé aléatoire",
btn_save_gw: "Enregistrer la configuration",
btn_factory_reset: "Réinitialisation d'usine",
node_ble_title: "Connexion Bluetooth Low Energy (Nœud)",
node_ble_desc: "Mettez le nœud LoRa en mode configuration (en appuyant une fois sur le bouton BOOT ; la LED bleue clignotera), puis connectez-vous ci-dessous.",
btn_connect_node: "Se connecter au Nœud",
fetching_node_params: "Connexion et récupération des paramètres via Bluetooth...",
node_sec_identity: "Identité du Nœud",
node_id: "ID unique du Nœud",
node_name: "Nom du Nœud",
tx_interval: "Intervalle de transmission (sec)",
node_sec_radio: "Paramètres Radio LoRa",
lora_chip: "Puce Radio LoRa",
lora_freq: "Fréquence (MHz)",
lora_bw: "Largeur de bande (kHz)",
lora_sf: "Facteur d'étalement (SF)",
lora_cr: "Taux de codage (CR)",
lora_sync: "Mot de synchro (Hex)",
lora_power: "Puissance d'émission (dBm)",
lora_preamble: "Préambule LoRa",
node_sec_security: "Sécurité & Mise à jour du Nœud",
node_aes_key: "Clé AES partagée (32 caract. hex / 16 octets)",
node_ota_title: "Mise à jour OTA du Nœud",
label_choose_fw: "Choisir le fichier firmware (.bin)",
no_file_selected: "Aucun fichier sélectionné",
btn_start_ota: "Démarrer la mise à jour OTA",
uploading_file: "Envoi du fichier...",
btn_save_node: "Enregistrer la configuration du nœud",
btn_fetch_logs: "Récupérer les journaux de diagnostic 📋",
btn_reset_node: "Réinitialisation d'usine du Nœud",
node_logs_title: "Journaux de diagnostic du Nœud (BLE)",
alert_reset_gw: "Voulez-vous vraiment réinitialiser la passerelle aux paramètres d'usine ?",
alert_reset_node: "Voulez-vous vraiment réinitialiser le nœud aux paramètres d'usine ?",
alert_start_ota: "Démarrer la mise à jour OTA du nœud ?",
alert_no_logs: "Aucune entrée de journal enregistrée sur le nœud.",
alert_connect_first: "Veuillez d'abord vous connecter au nœud via BLE !",
msg_fetching_logs: "Récupération des journaux du nœud..."
}
};
let currentLang = localStorage.getItem('lang') || (navigator.language && navigator.language.startsWith('fr') ? 'fr' : 'en');
function t(key, params = {}) {
const dict = translations[currentLang] || translations.en;
let text = dict[key] || translations.en[key] || key;
for (const [k, v] of Object.entries(params)) {
text = text.replace(`{${k}}`, v);
}
return text;
}
function setLanguage(lang) {
currentLang = lang;
localStorage.setItem('lang', lang);
document.documentElement.setAttribute('lang', lang);
document.querySelectorAll('[data-i18n]').forEach(el => {
const key = el.getAttribute('data-i18n');
el.innerHTML = t(key);
});
const saveGwBtn = document.getElementById('btn-save-gw-submit');
if (saveGwBtn) saveGwBtn.value = t('btn_save_gw');
const saveNodeBtn = document.getElementById('btn-save-node-submit');
if (saveNodeBtn) saveNodeBtn.value = t('btn_save_node');
const btn = document.getElementById('lang-toggle');
if (btn) btn.innerText = currentLang === 'fr' ? '🇫🇷 FR' : '🇬🇧 EN';
}
function toggleLanguage() {
setLanguage(currentLang === 'fr' ? 'en' : 'fr');
}
class NimBLEDataPipe {
constructor(serviceUuid, charUuid) {
this.serviceUuid = serviceUuid.toLowerCase();
this.charUuid = charUuid.toLowerCase();
this.device = null;
this.characteristic = null;
this.rxBuffer = new Uint8Array(0);
this.expectedLen = 0;
this.expectedType = 0;
this.headerReceived = false;
this.onJsonCallback = null;
this.onBinaryCallback = null;
this.onDisconnectCallback = null;
}
setOnJson(callback) { this.onJsonCallback = callback; }
setOnBinary(callback) { this.onBinaryCallback = callback; }
setOnDisconnect(callback) { this.onDisconnectCallback = callback; }
isConnected() {
return this.device && this.device.gatt.connected;
}
async connect(namePrefix = "ESP32-LoRa") {
this.device = await navigator.bluetooth.requestDevice({
filters: [{ namePrefix: namePrefix }],
optionalServices: [this.serviceUuid]
});
this.device.addEventListener('gattserverdisconnected', () => {
this.handleDisconnect();
});
const server = await this.device.gatt.connect();
const service = await server.getPrimaryService(this.serviceUuid);
this.characteristic = await service.getCharacteristic(this.charUuid);
await this.characteristic.startNotifications();
this.characteristic.addEventListener('characteristicvaluechanged', (event) => this.handleReceive(event));
}
async disconnect() {
if (this.isConnected()) {
await this.device.gatt.disconnect();
}
}
handleDisconnect() {
this.characteristic = null;
this.device = null;
this.rxBuffer = new Uint8Array(0);
this.headerReceived = false;
if (this.onDisconnectCallback) {
this.onDisconnectCallback();
}
}
handleReceive(event) {
const chunk = new Uint8Array(event.target.value.buffer);
const tmp = new Uint8Array(this.rxBuffer.length + chunk.length);
tmp.set(this.rxBuffer);
tmp.set(chunk, this.rxBuffer.length);
this.rxBuffer = tmp;
if (!this.headerReceived && this.rxBuffer.length >= 3) {
this.expectedType = this.rxBuffer[0];
this.expectedLen = this.rxBuffer[1] | (this.rxBuffer[2] << 8);
this.rxBuffer = this.rxBuffer.slice(3);
this.headerReceived = true;
}
if (this.headerReceived && this.rxBuffer.length >= this.expectedLen) {
const payload = this.rxBuffer.slice(0, this.expectedLen);
if (this.expectedType === 0x00) {
const text = new TextDecoder().decode(payload);
try {
const json = JSON.parse(text);
if (this.onJsonCallback) this.onJsonCallback(json);
} catch (e) {
console.error("Error parsing JSON:", e);
}
} else {
if (this.onBinaryCallback) this.onBinaryCallback(this.expectedType, payload);
}
this.rxBuffer = new Uint8Array(0);
this.headerReceived = false;
}
}
async sendJson(obj) {
if (!this.characteristic) return;
const text = JSON.stringify(obj);
const payload = new TextEncoder().encode(text);
const len = payload.length;
const buffer = new Uint8Array(3 + len);
buffer[0] = 0x00;
buffer[1] = len & 0xFF;
buffer[2] = (len >> 8) & 0xFF;
buffer.set(payload, 3);
await this.characteristic.writeValueWithResponse(buffer);
}
async sendBinary(type, payload) {
if (!this.characteristic) return;
const len = payload.length;
const buffer = new Uint8Array(3 + len);
buffer[0] = type;
buffer[1] = len & 0xFF;
buffer[2] = (len >> 8) & 0xFF;
buffer.set(payload, 3);
await this.characteristic.writeValueWithResponse(buffer);
}
}
const log = (msg, type = '') => {
console.log(`[Admin] ${msg}`);
};
const switchTab = (tabId) => {
document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
if (tabId === 'gw-config') {
document.getElementById('tab-btn-gw-config').classList.add('active');
document.getElementById('tab-gw-config').classList.add('active');
} else if (tabId === 'ble') {
document.getElementById('tab-btn-ble').classList.add('active');
document.getElementById('tab-ble').classList.add('active');
}
};
if (!window.isSecureContext) {
document.getElementById('ble-security-notice').classList.remove('hidden');
document.getElementById('gw-ble-security-notice').classList.remove('hidden');
}
const GW_SERVICE_UUID = "f1e00003-c32a-4b28-86c7-67ab6b5d7a9f";
const GW_CHAR_UUID = "f1e00004-c32a-4b28-86c7-67ab6b5d7a9f";
const gwPipe = new NimBLEDataPipe(GW_SERVICE_UUID, GW_CHAR_UUID);
const btnGwConnect = document.getElementById('btn-gw-connect');
const gwStatusDot = document.getElementById('gw-status-dot');
const gwStatusText = document.getElementById('gw-status-text');
const gwConfigForm = document.getElementById('gw-config-form');
window.generateRandomGwKey = () => {
const chars = '0123456789abcdef';
let key = '';
for (let i = 0; i < 32; i++) {
key += chars[Math.floor(Math.random() * 16)];
}
document.getElementById('gw_aes_key').value = key;
};
const gwUseStatic = document.getElementById('gw_use_static');
const gwStaticIpFields = document.getElementById('gw-static-ip-fields');
const updateGwStaticFieldsVisibility = () => {
if (gwUseStatic.checked) {
gwStaticIpFields.style.display = 'block';
document.getElementById('gw_local_ip').setAttribute('required', 'true');
document.getElementById('gw_gateway_ip').setAttribute('required', 'true');
document.getElementById('gw_subnet_mask').setAttribute('required', 'true');
} else {
gwStaticIpFields.style.display = 'none';
document.getElementById('gw_local_ip').removeAttribute('required');
document.getElementById('gw_gateway_ip').removeAttribute('required');
document.getElementById('gw_subnet_mask').removeAttribute('required');
}
};
gwUseStatic.addEventListener('change', updateGwStaticFieldsVisibility);
const isHttpMode = window.location.protocol !== 'file:' &&
window.location.hostname !== '' &&
window.location.hostname !== 'localhost' &&
window.location.hostname !== '127.0.0.1';
if (isHttpMode) {
const gwBleContainer = document.getElementById('gw-ble-container');
if (gwBleContainer) {
gwBleContainer.classList.add('hidden');
}
gwConfigForm.classList.remove('hidden');
const loadConfigHttp = async () => {
try {
const res = await fetch('/api/gw_config');
const json = await res.json();
const formatFreq = (f) => { const v = parseFloat(f || 433.0); return v === 869.525 ? "869.525" : v.toFixed(1); };
document.getElementById('gw_wifi_ssid').value = json.wifi_ssid || '';
document.getElementById('gw_wifi_pass').value = json.wifi_pass || '';
document.getElementById('gw_admin_pass').value = json.admin_pass || '';
document.getElementById('gw_use_static').checked = !!json.use_static;
document.getElementById('gw_local_ip').value = json.local_ip || '';
document.getElementById('gw_gateway_ip').value = json.gateway_ip || '';
document.getElementById('gw_subnet_mask').value = json.subnet_mask || '';
document.getElementById('gw_lora_chip').value = json.lora_chip || 2;
document.getElementById('gw_lora_freq').value = formatFreq(json.lora_freq);
document.getElementById('gw_lora_bw').value = parseFloat(json.lora_bw || 125.0).toFixed(1);
document.getElementById('gw_lora_sf').value = json.lora_sf || 9;
document.getElementById('gw_lora_cr').value = json.lora_cr || 5;
document.getElementById('gw_lora_sync').value = json.lora_sync || '0x12';
document.getElementById('gw_aes_key').value = json.aes_key || '';
document.getElementById('gw_station_alt').value = localStorage.getItem('station_altitude') || '96';
updateGwStaticFieldsVisibility();
} catch (e) {
log(`HTTP load error: ${e.message}`, "error");
}
};
loadConfigHttp();
}
gwPipe.setOnJson((json) => {
if (json.aes_key) {
const formatFreq = (f) => { const v = parseFloat(f || 433.0); return v === 869.525 ? "869.525" : v.toFixed(1); };
document.getElementById('gw_wifi_ssid').value = json.wifi_ssid || '';
document.getElementById('gw_wifi_pass').value = json.wifi_pass || '';
document.getElementById('gw_admin_pass').value = json.admin_pass || '';
document.getElementById('gw_use_static').checked = !!json.use_static;
document.getElementById('gw_local_ip').value = json.local_ip || '';
document.getElementById('gw_gateway_ip').value = json.gateway_ip || '';
document.getElementById('gw_subnet_mask').value = json.subnet_mask || '';
document.getElementById('gw_lora_chip').value = json.lora_chip || 2;
document.getElementById('gw_lora_freq').value = formatFreq(json.lora_freq);
document.getElementById('gw_lora_bw').value = parseFloat(json.lora_bw || 125.0).toFixed(1);
document.getElementById('gw_lora_sf').value = json.lora_sf || 9;
document.getElementById('gw_lora_cr').value = json.lora_cr || 5;
document.getElementById('gw_lora_sync').value = json.lora_sync || '0x12';
document.getElementById('gw_aes_key').value = json.aes_key || '';
document.getElementById('gw_station_alt').value = localStorage.getItem('station_altitude') || '96';
updateGwStaticFieldsVisibility();
document.getElementById('gw-loading-banner').classList.add('hidden');
gwStatusDot.className = "dot";
gwStatusText.innerText = "Connected";
gwStatusText.style.color = "#10b981";
} else if (json.status === 'saved') {
alert("Configuration saved via BLE! The gateway will reboot.");
} else if (json.status === 'reseted') {
alert("Gateway configuration reset to factory defaults!");
}
});
gwPipe.setOnDisconnect(() => {
gwStatusDot.className = "dot offline";
gwStatusText.innerText = "Disconnected";
gwStatusText.style.color = "#ef4444";
btnGwConnect.innerText = "Connect to Gateway";
document.getElementById('gw-loading-banner').classList.add('hidden');
if (!isHttpMode) gwConfigForm.classList.add('hidden');
});
btnGwConnect.addEventListener('click', async () => {
if (gwPipe.isConnected()) {
await gwPipe.disconnect();
return;
}
try {
gwStatusDot.className = "dot warning";
gwStatusText.innerText = "Searching...";
gwStatusText.style.color = "#f59e0b";
await gwPipe.connect("ESP32-LoRa-Gateway");
btnGwConnect.innerText = "Disconnect";
gwConfigForm.classList.remove('hidden');
document.getElementById('gw-loading-banner').classList.remove('hidden');
gwStatusDot.className = "dot warning";
gwStatusText.innerText = "Reading parameters...";
gwStatusText.style.color = "#f59e0b";
await gwPipe.sendJson({ cmd: "get_gw_config" });
} catch (err) {
log(`Connection error: ${err.message}`, "error");
gwPipe.handleDisconnect();
}
});
gwConfigForm.addEventListener('submit', async (e) => {
e.preventDefault();
localStorage.setItem('station_altitude', document.getElementById('gw_station_alt').value || '96');
const payload = {
wifi_ssid: document.getElementById('gw_wifi_ssid').value.trim(),
wifi_pass: document.getElementById('gw_wifi_pass').value.trim(),
admin_pass: document.getElementById('gw_admin_pass').value.trim(),
use_static: document.getElementById('gw_use_static').checked,
local_ip: document.getElementById('gw_local_ip').value.trim(),
gateway_ip: document.getElementById('gw_gateway_ip').value.trim(),
subnet_mask: document.getElementById('gw_subnet_mask').value.trim(),
lora_chip: parseInt(document.getElementById('gw_lora_chip').value),
lora_freq: parseFloat(document.getElementById('gw_lora_freq').value),
lora_bw: parseFloat(document.getElementById('gw_lora_bw').value),
lora_sf: parseInt(document.getElementById('gw_lora_sf').value),
lora_cr: parseInt(document.getElementById('gw_lora_cr').value),
lora_sync: document.getElementById('gw_lora_sync').value.trim(),
aes_key: document.getElementById('gw_aes_key').value.trim()
};
if (isHttpMode && !gwPipe.isConnected()) {
try {
const res = await fetch('/api/gw_config', {
method: 'POST',
headers: { 'Content-Type': 'application/json' },
body: JSON.stringify(payload)
});
const json = await res.json();
if (json.status === 'saved') {
alert("Configuration saved! The gateway will reboot.");
} else {
alert("Error saving configuration.");
}
} catch (err) {
alert(`Send error: ${err.message}`);
}
} else {
payload.cmd = "set_gw_config";
await gwPipe.sendJson(payload);
}
});
document.getElementById('btn-gw-reset').addEventListener('click', async () => {
if (!confirm("Are you sure you want to reset the gateway to factory default settings?")) return;
if (isHttpMode && !gwPipe.isConnected()) {
try {
const res = await fetch('/api/gw_reset', { method: 'POST' });
const json = await res.json();
if (json.status === 'reseted') {
alert("Configuration reset successfully! The gateway is rebooting.");
}
} catch (e) {
alert(`Error: ${e.message}`);
}
} else {
await gwPipe.sendJson({ cmd: "reset_gw_config" });
}
});
const SERVICE_UUID = "f1e00001-c32a-4b28-86c7-67ab6b5d7a9f";
const CHAR_UUID = "f1e00002-c32a-4b28-86c7-67ab6b5d7a9f";
const pipe = new NimBLEDataPipe(SERVICE_UUID, CHAR_UUID);
const btnConnect = document.getElementById('btn-connect');
const statusDot = document.getElementById('status-dot');
const statusText = document.getElementById('status-text');
const configForm = document.getElementById('config-form');
const otaFileInput = document.getElementById('ota-file');
const otaFileName = document.getElementById('ota-file-name');
const btnStartOta = document.getElementById('btn-start-ota');
const otaProgressContainer = document.getElementById('ota-progress-container');
const otaStatusLabel = document.getElementById('ota-status-label');
const otaPercent = document.getElementById('ota-percent');
const otaBar = document.getElementById('ota-bar');
window.generateRandomKey = () => {
const chars = '0123456789abcdef';
let key = '';
for (let i = 0; i < 32; i++) {
key += chars[Math.floor(Math.random() * 16)];
}
document.getElementById('aes_key').value = key;
};
pipe.setOnJson((json) => {
if (json.aes_key) {
document.getElementById('node_id').value = json.node_id;
document.getElementById('node_name').value = json.node_name;
const formatFreq = (f) => { const v = parseFloat(f || 433.0); return v === 869.525 ? "869.525" : v.toFixed(1); };
document.getElementById('lora_freq').value = formatFreq(json.lora_freq);
document.getElementById('lora_bw').value = parseFloat(json.lora_bw || 125.0).toFixed(1);
document.getElementById('lora_sf').value = json.lora_sf;
document.getElementById('lora_cr').value = json.lora_cr;
document.getElementById('lora_sync').value = '0x' + json.lora_sync.toString(16);
document.getElementById('lora_power').value = json.lora_power;
document.getElementById('lora_preamble').value = json.lora_preamble;
document.getElementById('lora_chip').value = json.lora_chip || 2;
document.getElementById('aes_key').value = json.aes_key;
document.getElementById('tx_interval').value = json.tx_interval;
document.getElementById('node-loading-banner').classList.add('hidden');
statusDot.className = "dot";
statusText.innerText = "Connected";
statusText.style.color = "#10b981";
} else if (json.cmd === 'node_logs') {
const container = document.getElementById('node-log-container');
const consoleEl = document.getElementById('node-log-console');
container.style.display = 'block';
if (json.logs && json.logs.length > 0) {
consoleEl.innerText = json.logs.map(l => `[+${Math.round(l.t/1000)}s] ${l.msg}`).join('\n');
} else {
consoleEl.innerText = "No log entries recorded on node yet.";
}
} else if (json.status === 'saved') {
alert("Configuration saved! The node will reboot.");
} else if (json.status === 'reseted') {
alert("The node has been reset to factory defaults.");
} else if (json.status === 'ota_started') {
startOtaTransfer();
} else if (json.status === 'ota_error') {
alert(`The node rejected the update: ${json.error}`);
resetOtaUi();
}
});
async function fetchNodeLogs() {
if (!pipe.isConnected()) {
alert("Please connect to the node via BLE first!");
return;
}
document.getElementById('node-log-container').style.display = 'block';
document.getElementById('node-log-console').innerText = 'Fetching logs from node...';
await pipe.sendJson({ cmd: "get_logs" });
}
pipe.setOnDisconnect(() => {
statusDot.className = "dot offline";
statusText.innerText = "Disconnected";
statusText.style.color = "#ef4444";
btnConnect.innerText = "Connect to Node";
document.getElementById('node-loading-banner').classList.add('hidden');
configForm.classList.add('hidden');
resetOtaUi();
});
btnConnect.addEventListener('click', async () => {
if (pipe.isConnected()) {
await pipe.disconnect();
return;
}
try {
statusDot.className = "dot warning";
statusText.innerText = "Searching...";
statusText.style.color = "#f59e0b";
await pipe.connect("ESP32-LoRa");
btnConnect.innerText = "Disconnect";
configForm.classList.remove('hidden');
document.getElementById('node-loading-banner').classList.remove('hidden');
statusDot.className = "dot warning";
statusText.innerText = "Reading parameters...";
statusText.style.color = "#f59e0b";
await pipe.sendJson({ cmd: "get_config" });
} catch (err) {
log(`Connection error: ${err.message}`, "error");
pipe.handleDisconnect();
}
});
configForm.addEventListener('submit', async (e) => {
e.preventDefault();
let syncStr = document.getElementById('lora_sync').value.trim();
let syncVal = syncStr.startsWith('0x') || syncStr.startsWith('0X') ?
parseInt(syncStr, 16) : parseInt(syncStr);
const payload = {
cmd: "set_config",
node_id: parseInt(document.getElementById('node_id').value),
node_name: document.getElementById('node_name').value.trim(),
lora_freq: parseFloat(document.getElementById('lora_freq').value),
lora_bw: parseFloat(document.getElementById('lora_bw').value),
lora_sf: parseInt(document.getElementById('lora_sf').value),
lora_cr: parseInt(document.getElementById('lora_cr').value),
lora_sync: syncVal,
lora_power: parseInt(document.getElementById('lora_power').value),
lora_preamble: parseInt(document.getElementById('lora_preamble').value),
lora_chip: parseInt(document.getElementById('lora_chip').value),
aes_key: document.getElementById('aes_key').value.trim().toLowerCase(),
tx_interval: parseInt(document.getElementById('tx_interval').value)
};
await pipe.sendJson(payload);
});
document.getElementById('btn-reset').addEventListener('click', async () => {
if (!confirm("Are you sure you want to reset the node?")) return;
await pipe.sendJson({ cmd: "reset_config" });
});
otaFileInput.addEventListener('change', () => {
const file = otaFileInput.files[0];
if (file) {
otaFileName.innerText = file.name + ` (${(file.size / 1024).toFixed(1)} KB)`;
btnStartOta.style.display = 'inline-block';
} else {
otaFileName.innerText = "No file selected";
btnStartOta.style.display = 'none';
}
});
btnStartOta.addEventListener('click', async () => {
const file = otaFileInput.files[0];
if (!file) return;
if (!confirm("Start node OTA update?")) return;
btnStartOta.disabled = true;
otaFileInput.disabled = true;
otaProgressContainer.style.display = 'block';
otaStatusLabel.innerText = "Starting OTA...";
otaPercent.innerText = "0%";
otaBar.style.width = "0%";
const reader = new FileReader();
reader.onload = async (e) => {
try {
const arrayBuffer = e.target.result;
const bytes = new Uint8Array(arrayBuffer);
await pipe.sendJson({ cmd: "start_ota", size: bytes.length });
} catch (err) {
alert(`Error sending start_ota: ${err.message}`);
resetOtaUi();
}
};
reader.readAsArrayBuffer(file);
});
const startOtaTransfer = async () => {
const file = otaFileInput.files[0];
if (!file) return;
const reader = new FileReader();
reader.onload = async (e) => {
let offset = 0;
let totalSize = 0;
try {
const arrayBuffer = e.target.result;
const bytes = new Uint8Array(arrayBuffer);
totalSize = bytes.length;
const chunkSize = 240;
offset = 0;
while (offset < totalSize) {
const size = Math.min(chunkSize, totalSize - offset);
const chunk = bytes.slice(offset, offset + size);
offset += size;
await pipe.sendBinary(0x02, chunk);
const percent = Math.floor((offset / totalSize) * 100);
otaPercent.innerText = `${percent}%`;
otaBar.style.width = `${percent}%`;
otaStatusLabel.innerText = `Sent: ${Math.round(offset / 1024)} / ${Math.round(totalSize / 1024)} KB`;
}
otaStatusLabel.innerText = "Finalizing and rebooting...";
alert("OTA update successful! The node is rebooting.");
resetOtaUi();
} catch (err) {
if (totalSize > 0 && offset >= (totalSize - 480)) {
otaPercent.innerText = "100%";
otaBar.style.width = "100%";
otaStatusLabel.innerText = "Update successful! Rebooting...";
alert("OTA update successful! The node is rebooting.");
resetOtaUi();
} else {
console.error("OTA Transfer Error:", err);
alert(`Transfer error: ${err.message || err}`);
resetOtaUi();
}
}
};
reader.readAsArrayBuffer(file);
};
const resetOtaUi = () => {
btnStartOta.disabled = false;
otaFileInput.disabled = false;
btnStartOta.style.display = 'none';
otaProgressContainer.style.display = 'none';
otaFileName.innerText = t('no_file_selected');
otaFileInput.value = "";
};
setLanguage(currentLang);</script> </body></html>
)rawliteral";

#endif // ADMIN_HTML_H
