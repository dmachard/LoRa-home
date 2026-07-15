#ifndef ADMIN_HTML_H
#define ADMIN_HTML_H

#include <pgmspace.h>

const char ADMIN_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Administration - LoRa Gateway</title> <style>@import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@300;400;500;600;700;800&display=swap');body{font-family:'Plus Jakarta Sans',system-ui,-apple-system,sans-serif;background:radial-gradient(circle at 50% 0%,#111827 0%,#030712 100%);color:#f8fafc;margin:0;padding:24px;min-height:100vh;}.container{max-width:1200px;margin:0 auto;}header{display:flex;justify-content:space-between;align-items:center;margin-bottom:32px;border-bottom:1px solid rgba(255,255,255,0.05);padding-bottom:20px;}h1{font-size:26px;font-weight:800;letter-spacing:-0.02em;background:linear-gradient(135deg,#ffffff 0%,#cbd5e1 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;margin:0;}.gw-stats{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:20px;margin-bottom:32px;}.stat-card{background:rgba(30,41,59,0.4);backdrop-filter:blur(16px);border:1px solid rgba(255,255,255,0.05);border-radius:16px;padding:24px;display:flex;flex-direction:column;box-shadow:0 4px 20px rgba(0,0,0,0.15);}.gw-stat-card{background:rgba(17,24,39,0.4);backdrop-filter:blur(24px);border:1px solid rgba(255,255,255,0.05);border-radius:16px;padding:16px 20px;display:flex;align-items:center;justify-content:space-between;box-shadow:0 10px 25px -5px rgba(0,0,0,0.3);}.stat-info{display:flex;flex-direction:column;}.stat-val{font-size:22px;font-weight:800;color:#ffffff;margin-top:2px;letter-spacing:-0.01em;}.stat-label{font-size:10px;color:#64748b;text-transform:uppercase;letter-spacing:0.08em;font-weight:700;}.wifi-signal-wrapper{display:flex;align-items:center;gap:12px;}.signal-bars{display:flex;align-items:flex-end;gap:3px;height:18px;}.signal-bar{width:4px;background:rgba(255,255,255,0.15);border-radius:2px;}.signal-bar.active{background:#38bdf8;}.signal-bar:nth-child(1){height:30%;}.signal-bar:nth-child(2){height:50%;}.signal-bar:nth-child(3){height:70%;}.signal-bar:nth-child(4){height:100%;}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(360px,1fr));gap:28px;}.node-card{background:rgba(17,24,39,0.45);backdrop-filter:blur(24px);border:1px solid rgba(16,185,129,0.18);border-radius:20px;padding:24px;box-shadow:0 20px 40px -15px rgba(0,0,0,0.4);transition:all 0.3s cubic-bezier(0.4,0,0.2,1);display:flex;flex-direction:column;position:relative;overflow:hidden;}.node-card:hover{transform:translateY(-4px);border-color:rgba(16,185,129,0.4);box-shadow:0 25px 50px -12px rgba(16,185,129,0.12);}.node-card.offline{border-color:rgba(239,68,68,0.15);background:rgba(17,24,39,0.25);opacity:0.65;}.node-card.offline:hover{border-color:rgba(239,68,68,0.35);box-shadow:0 25px 50px -12px rgba(239,68,68,0.08);opacity:0.85;}.node-card.warning{border-color:rgba(245,158,11,0.18);}.node-card.warning:hover{border-color:rgba(245,158,11,0.4);box-shadow:0 25px 50px -12px rgba(245,158,11,0.12);}.node-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px;}.node-title-group{display:flex;flex-direction:column;}.node-name{font-size:20px;font-weight:800;color:#ffffff;letter-spacing:-0.02em;}.node-meta{font-size:11px;color:#64748b;margin-top:2px;font-weight:500;text-transform:uppercase;letter-spacing:0.05em;}.node-badge{padding:4px 12px;border-radius:9999px;font-size:10px;font-weight:700;letter-spacing:0.05em;text-transform:uppercase;border:1px solid transparent;}.node-readings{display:flex;flex-wrap:wrap;gap:28px;padding:20px 0;border-top:1px solid rgba(255,255,255,0.05);border-bottom:1px solid rgba(255,255,255,0.05);margin-bottom:16px;}.metric-item{display:flex;flex-direction:column;gap:4px;min-width:105px;}.metric-value-wrapper{display:flex;align-items:center;gap:8px;}.metric-icon{width:18px;height:18px;flex-shrink:0;}.metric-icon.temp{color:#f43f5e;}.metric-icon.hum{color:#38bdf8;}.metric-icon.press{color:#a855f7;}.metric-icon.light{color:#eab308;}.metric-icon.batt{color:#10b981;}.metric-value{font-size:22px;font-weight:800;color:#f8fafc;letter-spacing:-0.02em;}.metric-unit{font-size:13px;color:#64748b;margin-left:1px;font-weight:600;}.metric-label{font-size:10px;color:#64748b;font-weight:700;text-transform:uppercase;letter-spacing:0.05em;}.node-footer{display:flex;flex-direction:column;gap:10px;margin-top:auto;padding-top:12px;border-top:1px solid rgba(255,255,255,0.04);}.node-radio-info{display:flex;gap:16px;}.node-radio-info span{display:flex;align-items:center;gap:4px;}.node-radio-icon{width:14px;height:14px;color:#64748b;}.node-activity-row{display:flex;justify-content:space-between;align-items:center;font-size:11px;color:#64748b;width:100%;}.connection-stats{display:flex;align-items:center;gap:6px;}.connection-stats span b{color:#cbd5e1;}.stat-divider{color:rgba(255,255,255,0.08);font-weight:bold;}.status-indicator{display:flex;align-items:center;gap:8px;}.dot{width:8px;height:8px;border-radius:50%;background:#10b981;box-shadow:0 0 10px rgba(16,185,129,0.8);position:relative;}.dot::after{content:'';position:absolute;top:0;left:0;width:100%;height:100%;border-radius:50%;background:inherit;animation:dot-pulse 1.8s infinite ease-in-out;}.dot.offline{background:#ef4444;box-shadow:0 0 10px rgba(239,68,68,0.8);}.dot.warning{background:#f59e0b;box-shadow:0 0 10px rgba(245,158,11,0.8);}@keyframes dot-pulse{0%{transform:scale(1);opacity:0.8;}100%{transform:scale(2.4);opacity:0;}}.status-text-on{color:#10b981;font-weight:600;}.status-text-warning{color:#f59e0b;font-weight:600;}.status-text-off{color:#ef4444;font-weight:600;}.btn-update{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);color:white;border:none;padding:9px 18px;border-radius:10px;font-weight:700;text-decoration:none;font-size:13px;transition:all 0.2s;box-shadow:0 4px 12px rgba(239,68,68,0.2);}.btn-update:hover{transform:translateY(-1px);box-shadow:0 6px 16px rgba(239,68,68,0.3);}.btn-metrics{background:rgba(255,255,255,0.05);color:#94a3b8;border:1px solid rgba(255,255,255,0.08);padding:9px 18px;border-radius:10px;font-weight:700;text-decoration:none;font-size:13px;margin-right:8px;transition:all 0.2s;}.btn-metrics:hover{color:#fff;border-color:rgba(255,255,255,0.2);}.pulse-ring{width:12px;height:12px;background:#38bdf8;border-radius:50%;box-shadow:0 0 0 0 rgba(56,189,248,0.7);animation:pulse 1.6s infinite;margin:0 auto 12px;}@keyframes pulse{0%{transform:scale(0.95);box-shadow:0 0 0 0 rgba(56,189,248,0.7);}70%{transform:scale(1);box-shadow:0 0 0 10px rgba(56,189,248,0);}100%{transform:scale(0.95);box-shadow:0 0 0 0 rgba(56,189,248,0);}}.tabs{display:flex;gap:8px;margin-bottom:28px;}.tab-btn{background:rgba(255,255,255,0.05);border:1px solid rgba(255,255,255,0.08);color:#94a3b8;padding:10px 20px;border-radius:12px;font-weight:700;cursor:pointer;transition:all 0.2s;font-size:14px;}.tab-btn:hover{background:rgba(255,255,255,0.1);color:#fff;}.tab-btn.active{background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);color:#fff;border-color:transparent;box-shadow:0 4px 12px rgba(56,189,248,0.2);}.tab-content{display:none;}.tab-content.active{display:block;}.form-group{margin-bottom:20px;}label{display:block;font-size:11px;font-weight:600;text-transform:uppercase;color:#64748b;margin-bottom:8px;letter-spacing:0.05em;}input[type="text"],input[type="number"],select{width:100%;background:rgba(15,23,42,0.6);border:1px solid rgba(255,255,255,0.08);padding:12px 16px;border-radius:12px;color:#f8fafc;font-size:14px;transition:all 0.2s;box-sizing:border-box;}input:focus,select:focus{border-color:#38bdf8;outline:none;box-shadow:0 0 0 3px rgba(56,189,248,0.15);}.btn{background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);color:#fff;border:none;padding:12px 24px;border-radius:12px;font-weight:700;cursor:pointer;transition:all 0.2s;box-shadow:0 4px 12px rgba(56,189,248,0.2);font-size:13px;display:inline-flex;align-items:center;justify-content:center;}.btn:hover{transform:translateY(-1px);opacity:0.95;}.btn:disabled{opacity:0.5;cursor:not-allowed;transform:none;}.btn-secondary{background:rgba(255,255,255,0.08);color:#fff;border:1px solid rgba(255,255,255,0.1);box-shadow:none;}.btn-secondary:hover{background:rgba(255,255,255,0.12);}.btn-danger{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);box-shadow:0 4px 12px rgba(239,68,68,0.2);}.btn-success{background:linear-gradient(135deg,#10b981 0%,#059669 100%);box-shadow:0 4px 12px rgba(16,185,129,0.2);}.hidden{display:none !important;}.alert-notice{background:rgba(239,68,68,0.1);border:1px solid rgba(239,68,68,0.2);color:#fca5a5;padding:18px;border-radius:16px;margin-bottom:24px;font-size:13px;line-height:1.6;}.alert-notice a{color:#38bdf8;text-decoration:underline;font-weight:600;}.body-center{display:flex;align-items:center;justify-content:center;}.card-update{background:linear-gradient(145deg,rgba(30,41,59,0.5) 0%,rgba(15,23,42,0.75) 100%);backdrop-filter:blur(20px);border:1px solid rgba(255,255,255,0.05);border-radius:24px;padding:32px;width:100%;max-width:450px;box-shadow:0 20px 40px -15px rgba(0,0,0,0.5);text-align:center;}.card-update h3{font-size:20px;font-weight:800;margin-top:0;margin-bottom:24px;background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}.card-update.error h3{background:linear-gradient(135deg,#ef4444 0%,#dc2626 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}.card-update.success h3{background:linear-gradient(135deg,#10b981 0%,#059669 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}.card-update p{color:#94a3b8;font-size:14px;margin-bottom:16px;}.file-input-wrapper{margin-bottom:24px;}input[type="file"]{display:none;}.custom-file-upload{display:inline-block;padding:12px 24px;background:rgba(15,23,42,0.6);border:1px dashed rgba(56,189,248,0.4);border-radius:12px;cursor:pointer;font-size:14px;color:#38bdf8;transition:all 0.2s;width:80%;}.custom-file-upload:hover{background:rgba(56,189,248,0.1);border-color:#38bdf8;}.btn-submit{background:linear-gradient(135deg,#38bdf8 0%,#818cf8 100%);border:none;color:#fff;padding:12px 28px;font-size:14px;font-weight:600;border-radius:12px;cursor:pointer;box-shadow:0 4px 12px rgba(56,189,248,0.2);transition:all 0.2s;}.btn-submit:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(56,189,248,0.35);}.btn-submit:active{transform:translateY(0);}.file-name{margin-top:8px;font-size:12px;color:#64748b;}.btn-back{display:inline-block;margin-top:16px;font-size:13px;color:#64748b;text-decoration:none;transition:color 0.2s;}.btn-back:hover{color:#f8fafc;}.legend-card{margin-bottom:24px;background:rgba(17,24,39,0.3);border:1px solid rgba(255,255,255,0.04);border-radius:16px;padding:16px 20px;display:flex;flex-direction:column;gap:12px;}.legend-header{display:flex;align-items:center;gap:8px;font-size:12px;font-weight:700;color:#94a3b8;text-transform:uppercase;letter-spacing:0.05em;}.legend-icon{width:14px;height:14px;color:#94a3b8;}.legend-body{display:flex;flex-wrap:wrap;gap:24px;}.legend-section{display:flex;align-items:center;gap:12px;}.legend-section-title{font-size:11px;font-weight:700;color:#64748b;text-transform:uppercase;letter-spacing:0.02em;}.legend-indicators{display:flex;gap:12px;flex-wrap:wrap;}.legend-tag{display:flex;align-items:center;gap:6px;font-size:11px;font-weight:600;color:#94a3b8;padding:4px 10px;background:rgba(15,23,42,0.4);border-radius:8px;border:1px solid rgba(255,255,255,0.02);}.legend-dot{width:6px;height:6px;border-radius:50%;}.legend-tag.strong{border-color:rgba(16,185,129,0.15);}.legend-tag.strong .legend-dot{background:#10b981;}.legend-tag.medium{border-color:rgba(245,158,11,0.15);}.legend-tag.medium .legend-dot{background:#f59e0b;}.legend-tag.weak{border-color:rgba(239,68,68,0.15);}.legend-tag.weak .legend-dot{background:#ef4444;}.radar-container{position:relative;width:80px;height:80px;display:flex;align-items:center;justify-content:center;margin-bottom:8px;}.radar-icon{width:32px;height:32px;color:#38bdf8;z-index:2;filter:drop-shadow(0 0 8px rgba(56,189,248,0.5));}.radar-circle{position:absolute;top:50%;left:50%;width:100%;height:100%;border:1px solid rgba(56,189,248,0.25);border-radius:50%;transform:translate(-50%,-50%) scale(0.2);opacity:0;pointer-events:none;}.circle-1{animation:radar-pulse 3s infinite linear;}.circle-2{animation:radar-pulse 3s infinite 1s linear;}.circle-3{animation:radar-pulse 3s infinite 2s linear;}@keyframes radar-pulse{0%{transform:translate(-50%,-50%) scale(0.2);opacity:0.8;}100%{transform:translate(-50%,-50%) scale(1.2);opacity:0;}}.scanning-dots::after{content:'';animation:scanning-dots-anim 1.5s infinite;}@keyframes scanning-dots-anim{0%{content:'';}33%{content:'.';}66%{content:'..';}100%{content:'...';}}</style> </head><body><div class="container"><header><div><h1>Gateway Administration</h1><div style="font-size:12px; color:#64748b; margin-top:4px;">System configuration and node provisioning</div></div><div><a href="/metrics" class="btn-metrics">Prometheus Metrics</a><a href="/update" class="btn-update">Firmware Update</a><a href="/" class="btn" style="background:rgba(255, 255, 255, 0.08); border:1px solid rgba(255,255,255,0.1); margin-left:8px;">← Dashboard</a></div></header><div class="tabs"><button class="tab-btn active" id="tab-btn-gw-config" onclick="switchTab('gw-config')">Gateway Configuration</button><button class="tab-btn" id="tab-btn-ble" onclick="switchTab('ble')">Node Configuration (BLE)</button></div><div id="tab-gw-config" class="tab-content active"><div id="gw-ble-security-notice" class="alert-notice hidden"><strong>⚠️ Bluetooth disabled by browser (Security)</strong><br> The browser requires a secure connection (HTTPS or localhost) to allow Bluetooth.<br> To use local BLE, open this page locally (double-click) or access the local IP via HTTP. </div><div id="gw-ble-container" style="margin-bottom: 24px;"><div class="stat-card" style="padding: 24px;"><h2 style="margin-top:0; font-size: 18px; font-weight: 700;">Bluetooth Low Energy Connection (Gateway)</h2><p style="font-size: 13px; color: #94a3b8; margin-bottom: 16px;"> Put the gateway into config mode (press the BOOT button or start it without config; the blue LED will start flashing), then connect via local BLE below. </p><div style="display: flex; gap: 16px; align-items: center;"><button type="button" class="btn" id="btn-gw-connect">Connect to Gateway</button><div style="display: flex; align-items: center; gap: 8px; font-size: 14px;"><span id="gw-status-dot" class="dot offline"></span><span id="gw-status-text" style="font-weight:600; color:#ef4444;">Disconnected</span></div></div></div></div><form id="gw-config-form" class="hidden"><div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 24px; margin-bottom: 24px;"><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">WiFi Network & Security</h3><div class="form-group"><label for="gw_wifi_ssid">Network Name (SSID)</label><input type="text" id="gw_wifi_ssid" required placeholder="e.g. MyWiFiNetwork"></div><div class="form-group"><label for="gw_wifi_pass">WiFi Password</label><input type="text" id="gw_wifi_pass" required placeholder="WPA Key"></div><div class="form-group" style="margin-top:24px; border-top:1px solid rgba(255,255,255,0.05); padding-top:16px;"><label for="gw_admin_pass">Administration Password (HTTP Auth)</label><input type="text" id="gw_admin_pass" required placeholder="admin" value="admin"><span style="font-size:10px; color:#64748b; display:block; margin-top:4px;">Default: admin</span></div></div><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">IP Configuration</h3><div class="form-group" style="display: flex; align-items: center; gap: 10px; margin-bottom: 20px;"><input type="checkbox" id="gw_use_static" style="width:20px; height:20px; cursor:pointer;"><label for="gw_use_static" style="margin-bottom:0; cursor:pointer;">Use static IP address</label></div><div id="gw-static-ip-fields" style="display: none;"><div class="form-group"><label for="gw_local_ip">Local IP Address</label><input type="text" id="gw_local_ip" placeholder="e.g. 192.168.1.100"></div><div class="form-group"><label for="gw_gateway_ip">Default Gateway</label><input type="text" id="gw_gateway_ip" placeholder="e.g. 192.168.1.1"></div><div class="form-group"><label for="gw_subnet_mask">Subnet Mask</label><input type="text" id="gw_subnet_mask" placeholder="e.g. 255.255.255.0"></div></div></div><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">LoRa Security (AES-GCM Encryption)</h3><div class="form-group"><label for="gw_aes_key">Shared AES Key (32 hex chars / 16 bytes)</label><input type="text" id="gw_aes_key" maxlength="32" minlength="32" required placeholder="Hexadecimal AES Key"></div><button type="button" class="btn btn-secondary" onclick="generateRandomGwKey()" style="width:100%; margin-top:8px;">Generate Random Key</button></div></div><div style="display: flex; gap: 16px; margin-top: 24px; border-top: 1px solid rgba(255,255,255,0.05); padding-top: 24px;"><input type="submit" value="Save Configuration" class="btn" style="flex: 1;"><button type="button" class="btn btn-danger" id="btn-gw-reset" style="padding: 12px 28px;">Factory Reset</button></div></form></div><div id="tab-ble" class="tab-content"><div id="ble-security-notice" class="alert-notice hidden"><strong>⚠️ Bluetooth disabled by browser (Security)</strong><br> The browser requires a secure connection (HTTPS or localhost) to allow Bluetooth.<br> To use local BLE, open this page locally (double-click) or access the local IP via HTTP. </div><div id="ble-container"><div class="stat-card" style="margin-bottom: 24px; padding: 24px;"><h2 style="margin-top:0; font-size: 18px; font-weight: 700;">Bluetooth Low Energy Connection (Node)</h2><p style="font-size: 13px; color: #94a3b8; margin-bottom: 16px;"> Put the LoRa node into configuration mode (by pressing the BOOT button once; the blue LED will start flashing), then connect below. </p><div style="display: flex; gap: 16px; align-items: center;"><button type="button" class="btn" id="btn-connect">Connect to Node</button><div style="display: flex; align-items: center; gap: 8px; font-size: 14px;"><span id="status-dot" class="dot offline"></span><span id="status-text" style="font-weight:600; color:#ef4444;">Disconnected</span></div></div></div><form id="config-form" class="hidden"><div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 24px; margin-bottom: 24px;"><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">Node Identity</h3><div class="form-group"><label for="node_id">Unique Node ID</label><input type="number" id="node_id" min="1" max="254" required></div><div class="form-group"><label for="node_name">Node Name</label><input type="text" id="node_name" maxlength="15" placeholder="e.g. LivingRoom" required></div><div class="form-group"><label for="tx_interval">Transmission Interval (sec)</label><input type="number" id="tx_interval" min="5" max="3600" required></div></div><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">LoRa Radio Settings</h3><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_freq">Frequency (MHz)</label><input type="number" id="lora_freq" step="0.1" required></div><div class="form-group"><label for="lora_bw">Bandwidth (kHz)</label><select id="lora_bw" required><option value="7.8">7.8</option><option value="10.4">10.4</option><option value="15.6">15.6</option><option value="20.8">20.8</option><option value="31.25">31.25</option><option value="41.7">41.7</option><option value="62.5">62.5</option><option value="125.0">125.0</option><option value="250.0">250.0</option><option value="500.0">500.0</option></select></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_sf">Spreading Factor (SF)</label><input type="number" id="lora_sf" min="6" max="12" required></div><div class="form-group"><label for="lora_cr">Coding Rate (CR)</label><input type="number" id="lora_cr" min="5" max="8" required></div></div><div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label for="lora_sync">Sync Word (Hex)</label><input type="text" id="lora_sync" required></div><div class="form-group"><label for="lora_power">TX Power (dBm)</label><input type="number" id="lora_power" min="2" max="20" required></div></div><div class="form-group"><label for="lora_preamble">LoRa Preamble</label><input type="number" id="lora_preamble" min="6" max="65535" required></div></div><div class="stat-card" style="padding: 24px;"><h3 style="margin-top:0; font-size: 15px; font-weight: 700; color: #38bdf8; border-bottom:1px solid rgba(255,255,255,0.05); padding-bottom:10px; margin-bottom:15px;">Node Security & Update</h3><div class="form-group"><label for="aes_key">Shared AES Key (32 hex chars / 16 bytes)</label><input type="text" id="aes_key" maxlength="32" minlength="32" required placeholder="AES Encryption Key"></div><button type="button" class="btn btn-secondary" onclick="generateRandomKey()" style="width:100%; margin-bottom: 20px;">Generate Random Key</button><div style="border-top: 1px solid rgba(255,255,255,0.05); padding-top: 16px;"><h4 style="margin-top:0; font-size: 13px; color:#f8fafc; font-weight:700;">Node OTA Update</h4><div class="form-group" style="margin-top:12px;"><label for="ota-file" class="custom-file-upload" style="text-align:center;">Choose firmware (.bin)</label><input type="file" id="ota-file" accept=".bin"><div id="ota-file-name" class="file-name">No file selected</div></div><button type="button" class="btn btn-success" id="btn-start-ota" style="display:none; width:100%; margin-top:8px;">Start OTA Update</button><div id="ota-progress-container" style="display:none; margin-top: 16px;"><div style="display:flex; justify-content:space-between; font-size:12px; margin-bottom:6px;"><span id="ota-status-label" style="color:#94a3b8;">Uploading file...</span><span id="ota-percent" style="font-weight:700; color:#38bdf8;">0%</span></div><div style="background:rgba(0,0,0,0.3); border-radius:10px; height:8px; overflow:hidden;"><div id="ota-bar" style="background:linear-gradient(90deg, #38bdf8, #818cf8); width:0%; height:100%; transition:width 0.1s;"></div></div></div></div></div></div><div style="display: flex; gap: 16px; margin-top: 24px; border-top: 1px solid rgba(255,255,255,0.05); padding-top: 24px;"><input type="submit" value="Save Node Configuration" class="btn" style="flex: 1;"><button type="button" class="btn btn-danger" id="btn-reset" style="padding: 12px 28px;">Factory Reset Node</button></div></form></div></div></div> <script>class NimBLEDataPipe {
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
const bleCard = document.querySelector('#gw-ble-container > .stat-card');
if (bleCard) {
bleCard.innerHTML = `
<h2 style="margin-top:0; font-size: 18px; font-weight: 700;">Secure HTTP Connection (Admin)</h2>
<p style="font-size: 13px; color: #94a3b8; margin-bottom: 0;">
You are connected directly to the gateway. Configurations are sent via password-secured HTTP requests.
</p>
`;
}
gwConfigForm.classList.remove('hidden');
const loadConfigHttp = async () => {
try {
const res = await fetch('/api/gw_config');
const json = await res.json();
document.getElementById('gw_wifi_ssid').value = json.wifi_ssid || '';
document.getElementById('gw_wifi_pass').value = json.wifi_pass || '';
document.getElementById('gw_admin_pass').value = json.admin_pass || '';
document.getElementById('gw_use_static').checked = !!json.use_static;
document.getElementById('gw_local_ip').value = json.local_ip || '';
document.getElementById('gw_gateway_ip').value = json.gateway_ip || '';
document.getElementById('gw_subnet_mask').value = json.subnet_mask || '';
document.getElementById('gw_aes_key').value = json.aes_key || '';
updateGwStaticFieldsVisibility();
} catch (e) {
log(`HTTP load error: ${e.message}`, "error");
}
};
loadConfigHttp();
}
gwPipe.setOnJson((json) => {
if (json.aes_key) {
document.getElementById('gw_wifi_ssid').value = json.wifi_ssid || '';
document.getElementById('gw_wifi_pass').value = json.wifi_pass || '';
document.getElementById('gw_admin_pass').value = json.admin_pass || '';
document.getElementById('gw_use_static').checked = !!json.use_static;
document.getElementById('gw_local_ip').value = json.local_ip || '';
document.getElementById('gw_gateway_ip').value = json.gateway_ip || '';
document.getElementById('gw_subnet_mask').value = json.subnet_mask || '';
document.getElementById('gw_aes_key').value = json.aes_key || '';
updateGwStaticFieldsVisibility();
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
gwStatusDot.className = "dot";
gwStatusText.innerText = "Connected";
gwStatusText.style.color = "#10b981";
btnGwConnect.innerText = "Disconnect";
gwConfigForm.classList.remove('hidden');
await gwPipe.sendJson({ cmd: "get_gw_config" });
} catch (err) {
log(`Connection error: ${err.message}`, "error");
gwPipe.handleDisconnect();
}
});
gwConfigForm.addEventListener('submit', async (e) => {
e.preventDefault();
const payload = {
wifi_ssid: document.getElementById('gw_wifi_ssid').value.trim(),
wifi_pass: document.getElementById('gw_wifi_pass').value.trim(),
admin_pass: document.getElementById('gw_admin_pass').value.trim(),
use_static: document.getElementById('gw_use_static').checked,
local_ip: document.getElementById('gw_local_ip').value.trim(),
gateway_ip: document.getElementById('gw_gateway_ip').value.trim(),
subnet_mask: document.getElementById('gw_subnet_mask').value.trim(),
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
document.getElementById('lora_freq').value = json.lora_freq;
document.getElementById('lora_bw').value = json.lora_bw.toFixed(1);
document.getElementById('lora_sf').value = json.lora_sf;
document.getElementById('lora_cr').value = json.lora_cr;
document.getElementById('lora_sync').value = '0x' + json.lora_sync.toString(16);
document.getElementById('lora_power').value = json.lora_power;
document.getElementById('lora_preamble').value = json.lora_preamble;
document.getElementById('aes_key').value = json.aes_key;
document.getElementById('tx_interval').value = json.tx_interval;
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
pipe.setOnDisconnect(() => {
statusDot.className = "dot offline";
statusText.innerText = "Disconnected";
statusText.style.color = "#ef4444";
btnConnect.innerText = "Connect to Node";
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
statusDot.className = "dot";
statusText.innerText = "Connected";
statusText.style.color = "#10b981";
btnConnect.innerText = "Disconnect";
configForm.classList.remove('hidden');
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
try {
const arrayBuffer = e.target.result;
const bytes = new Uint8Array(arrayBuffer);
const totalSize = bytes.length;
const chunkSize = 480;
let offset = 0;
while (offset < totalSize) {
const size = Math.min(chunkSize, totalSize - offset);
const chunk = bytes.slice(offset, offset + size);
await pipe.sendBinary(0x02, chunk);
offset += size;
const percent = Math.floor((offset / totalSize) * 100);
otaPercent.innerText = `${percent}%`;
otaBar.style.width = `${percent}%`;
otaStatusLabel.innerText = `Sent: ${Math.round(offset / 1024)} / ${Math.round(totalSize / 1024)} KB`;
}
otaStatusLabel.innerText = "Finalizing and rebooting...";
} catch (err) {
if (offset >= totalSize) {
otaStatusLabel.innerText = "Update successful! Rebooting...";
alert("OTA update successful! The node is rebooting.");
resetOtaUi();
} else {
alert(`Transfer error: ${err.message}`);
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
otaFileName.innerText = "No file selected";
otaFileInput.value = "";
};</script> </body></html>
)rawliteral";

#endif // ADMIN_HTML_H
