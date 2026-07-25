#include "webserver.h"
#include <WiFi.h>
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <FS.h>
#include <DNSServer.h>
#include "config.h"
#include "display.h"
#include "sdcard.h"
#include "buzzer.h"
#include "solenoid.h"
#include "player.h"
#include "pins.h"
#include "ota_wifi.h"

WebServerManager webServer;

namespace {
httpd_handle_t server = nullptr;
DNSServer dnsServer;
bool active = false;
bool needsScan = false;

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>ESP Dashboard</title>
  <style>
  :root { --bg-color: #0f172a; --card-bg: #1e293b; --text-main: #f1f5f9; --text-muted: #94a3b8; --accent: #38bdf8; --danger: #ef4444; --border: #334155; }
  body { font-family: 'Segoe UI', system-ui, sans-serif; background: var(--bg-color); color: var(--text-main); margin: 0; padding: 20px; line-height: 1.5; }
  .card { background: var(--card-bg); padding: 20px; border-radius: 16px; max-width: 600px; margin: 0 auto 20px; border: 1px solid var(--border); }
  h2 { margin-top: 0; color: var(--accent); font-size: 1.25rem; }
  input { padding: 10px; border-radius: 8px; border: 1px solid var(--border); background: #0f172a; color: white; width: 100%; box-sizing: border-box; margin-bottom: 10px; }
  button { padding: 10px; border: none; border-radius: 8px; cursor: pointer; font-weight: 600; background: var(--accent); color: #0f172a; width: 100%; }
  </style>
</head>
<body>
<div class="card">
  <h2>Konfigurasi WiFi STA</h2>
  <input type="text" id="wSSID" placeholder="SSID WiFi" />
  <input type="text" id="wPass" placeholder="Password WiFi" />
  <label><input type="checkbox" id="wEnabled" /> Aktifkan WiFi STA</label>
  <button onclick="saveWifi()">Simpan Konfigurasi</button>
</div>
<script>
  async function loadWifi() {
    const res = await fetch('/api/wifi'); const wifi = await res.json();
    document.getElementById('wSSID').value = wifi.ssid;
    document.getElementById('wPass').value = wifi.pass;
    document.getElementById('wEnabled').checked = wifi.enabled;
  }
  async function saveWifi() {
    const wifi = { 
        ssid: document.getElementById('wSSID').value, 
        pass: document.getElementById('wPass').value, 
        enabled: document.getElementById('wEnabled').checked 
    };
    await fetch('/api/wifi', { method: 'POST', body: JSON.stringify(wifi) });
    alert('Disimpan! Restart untuk menerapkan.');
  }
  loadWifi();
</script>
</body>
</html>
)rawliteral";
}

// Handler functions (kept simple)
esp_err_t root_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, htmlPage, HTTPD_RESP_USE_STRLEN);
}

esp_err_t api_wifi_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    String ssid, pass; bool enabled;
    otaWifi.getConfig(ssid, pass, enabled);
    String json = "{\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"enabled\":" + (enabled ? "true" : "false") + "}";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
      buf[ret] = '\0'; String data(buf);
      bool enabled = (data.indexOf("true") != -1);
      // Simplified parsing
      int sStart = data.indexOf("\"ssid\":\"") + 8;
      String ssid = data.substring(sStart, data.indexOf("\"", sStart));
      int pStart = data.indexOf("\"pass\":\"") + 8;
      String pass = data.substring(pStart, data.indexOf("\"", pStart));
      otaWifi.saveConfig(ssid, pass, enabled);
      httpd_resp_send(req, "OK", 2);
    }
    return ESP_OK;
  }
  return ESP_FAIL;
}

void WebServerManager::begin() {
  if (active) return;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 24;
  if (httpd_start(&server, &config) == ESP_OK) {
    static httpd_uri_t r = { "/", HTTP_GET, root_handler, nullptr };
    httpd_register_uri_handler(server, &r);
    static httpd_uri_t w_g = { "/api/wifi", HTTP_GET, api_wifi_handler, nullptr };
    httpd_register_uri_handler(server, &w_g);
    static httpd_uri_t w_p = { "/api/wifi", HTTP_POST, api_wifi_handler, nullptr };
    httpd_register_uri_handler(server, &w_p);
    active = true;
  }
}

void WebServerManager::startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  dnsServer.start(53, "*", WiFi.softAPIP());
  begin();
}

void WebServerManager::stop() {
  if (server) httpd_stop(server);
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  active = false;
}

bool WebServerManager::isActive() const { return active; }
void WebServerManager::update() { dnsServer.processNextRequest(); }
