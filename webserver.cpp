#include "webserver.h"
#include "buzzer.h"
#include "config.h"
#include "display.h"
#include "pins.h"
#include "wifi_manager.h"
#include "player.h"
#include "sdcard.h"
#include "solenoid.h"
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include <esp_http_server.h>

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
  :root {
      --bg-color: #0f172a;
      --card-bg: #1e293b;
      --text-main: #f1f5f9;
      --text-muted: #94a3b8;
      --accent: #38bdf8;
      --danger: #ef4444;
      --border: #334155;
  }
  body { 
      font-family: 'Segoe UI', system-ui, sans-serif; 
      background: var(--bg-color); 
      color: var(--text-main); 
      margin: 0; 
      padding: 20px; 
      line-height: 1.5;
  }
  .card { 
      background: var(--card-bg); 
      padding: 20px; 
      border-radius: 16px; 
      max-width: 600px; 
      margin: 0 auto 20px; 
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.2);
      border: 1px solid var(--border);
  }
  h2 { 
      margin-top: 0; 
      color: var(--accent); 
      font-size: 1.25rem; 
      letter-spacing: -0.025em;
  }
  .row { display: flex; align-items: center; gap: 12px; margin-bottom: 15px; }
  input { 
      padding: 10px 14px; 
      border: 1px solid var(--border); 
      border-radius: 8px; 
      background: #0f172a; 
      color: white; 
      font-size: 0.95rem; 
      flex-grow: 1; 
      transition: border-color 0.2s;
  }
  input:focus { outline: none; border-color: var(--accent); }
  button { 
      padding: 10px 16px; 
      border: none; 
      border-radius: 8px; 
      cursor: pointer; 
      font-weight: 600; 
      font-size: 0.9rem;
      transition: opacity 0.2s;
  }
  button:hover { opacity: 0.9; }
  .primary { background: var(--accent); color: #0f172a; }
  .danger { background: var(--danger); color: white; }
  
  table { width: 100%; border-collapse: separate; border-spacing: 0 8px; }
  th { color: var(--text-muted); font-size: 0.8rem; text-transform: uppercase; padding: 10px; }
  .left { text-align: left; }
  .center { text-align: center; }
  td { padding: 12px 10px; background: rgba(0,0,0,0.1); }
  td:first-child { border-radius: 8px 0 0 8px; }
  td:last-child { border-radius: 0 8px 8px 0; }
  
  .col-name { width: 50%; }
  .col-size { width: 25%; }
  .col-action { width: 25%; }
  .col-pin { width: 20%; }
  .col-note { width: 30%; }
  .col-midi { width: 25%; }
  .col-s-action { width: 25%; }

  /* Toggle Switch */
  .switch { position: relative; display: inline-block; width: 44px; height: 24px; }
  .switch input { opacity: 0; width: 0; height: 0; }
  .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #334155; transition: .4s; border-radius: 24px; }
  .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .4s; border-radius: 50%; }
  input:checked + .slider { background-color: var(--accent); }
  input:checked + .slider:before { transform: translateX(20px); }
  </style>
</head>
<body>
<div class="card">
  <h2>Upload MIDI File</h2>
  <div class="row">
    <label for="fileInput" style="
        padding: 10px 14px; 
        border: 1px solid var(--border); 
        border-radius: 8px; 
        background: #0f172a; 
        color: var(--text-muted); 
        cursor: pointer;
        flex-grow: 1;
        text-align: center;
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
    " onclick="document.getElementById('fileInput').click()">Select MIDI File</label>
    <input type="file" id="fileInput" accept=".mid,.midi" style="display:none;" onchange="document.querySelector('label[for=\'fileInput\']').innerText = this.files[0].name" />
    <button onclick="uploadFile()" class="primary">Upload</button>
    </div>
    </div>
<div class="card">
  <h2>File Manager</h2>
  <table>
    <thead>
      <tr>
        <th class="col-name left">Name</th>
        <th class="col-size center">Size</th>
        <th class="col-action center">Action</th>
      </tr>
    </thead>
    <tbody id="fileBody"></tbody>
  </table>
  <div id="storageInfo" style="margin-top: 10px; font-size: 0.9rem; color: var(--text-muted); text-align: center;"></div>
</div>
<div class="card">
  <h2>Actuator Active Duration</h2>
  <div class="row" style="justify-content: space-between;">
    <span>Current Duration : <strong id="currentTime">...</strong> ms</span>
  </div>
  <div class="row">
    <input type="number" id="sTime" placeholder="Enter New Duration" />
    <button onclick="saveTime()" class="primary">Save</button>
  </div>
</div>
<div class="card">
<h2>Actuator Manager</h2>
<div style="display: flex; flex-direction: column; gap: 10px; margin-bottom: 15px;">
  <input type="number" id="sPin" placeholder="GPIO" />
  <input type="text" id="sNote" placeholder="Note" />
  <input type="number" id="sMidi" placeholder="MIDI Note Number" />
  <button onclick="addSolenoid()" class="primary">Add and Save</button>
</div>
<table>
  <thead>
    <tr>
      <th class="col-pin left">GPIO</th>
      <th class="col-note left">Note</th>
      <th class="col-midi left">MIDI</th>
      <th class="col-s-action center">Action</th>
    </tr>
  </thead>
  <tbody id="solenoidBody"></tbody>
</table>
</div>
<div class="card">
<h2>WiFi Manager</h2>
<div style="display: flex; flex-direction: column; gap: 10px;">
  <input type="text" id="wifiSsid" placeholder="SSID" />
  <input type="text" id="wifiPass" placeholder="Password" />
  <div class="row" style="justify-content: space-between; margin-top: 5px;">
      <label style="font-size: 0.95rem; color: var(--text-muted);">Enable WiFi STA</label>
      <label class="switch">
          <input type="checkbox" id="wifiEnable" onchange="saveWifi(true)">
          <span class="slider"></span>
      </label>
  </div>
  <button onclick="saveWifi()" class="primary">Save and Apply</button>
</div>
</div>
<footer style="text-align: center; color: var(--text-muted); font-size: 0.85rem; margin-top: 30px; margin-bottom: 20px;">

  &copy; 2026 AN ELECTRONIC | Mataram, Nusa Tenggara Barat<br>
  Version: {{FW_VERSION}}
</footer>
<script>
  let lastFiles = [];
  async function loadData() {
      const t = Date.now();
      try {
          const resS = await fetch('/api/solenoids?t=' + t); const solenoids = await resS.json();
          const resF = await fetch('/api/files?t=' + t); const filesRes = await resF.json();
          const resT = await fetch('/api/time?t=' + t); const time = await resT.json();
          
          document.getElementById('currentTime').innerText = time;
          render(solenoids, filesRes.files, filesRes.storage);
      } catch (e) { console.error("Load error", e); }
  }
  async function loadWifi() {
      try {
          const res = await fetch('/api/wifi?t=' + Date.now());
          const config = await res.json();
          document.getElementById('wifiSsid').value = config.ssid || "";
          document.getElementById('wifiPass').value = config.pass || "";
          document.getElementById('wifiEnable').checked = config.enable || false;
      } catch (e) { console.error("Wifi load error", e); }
  }
  function formatSize(bytes) {
      if (bytes < 1024) return bytes + ' B';
      if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
      if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
      return (bytes / (1024 * 1024 * 1024)).toFixed(1) + ' GB';
  }
  function render(solenoids, files, storage) {
    const sBody = document.getElementById('solenoidBody'); sBody.innerHTML = '';
    solenoids.forEach(s => { 
        sBody.innerHTML += `<tr>
            <td class="col-pin left">${s.pin}</td>
            <td class="col-note left">${s.note}</td>
            <td class="col-midi left">${s.midi}</td>
            <td class="col-s-action center"><button class="danger" onclick="removeSolenoid(${s.pin})">Delete</button></td>
        </tr>`; 
    });
    const fBody = document.getElementById('fileBody'); fBody.innerHTML = '';
    files.forEach(f => { 
        fBody.innerHTML += `<tr>
            <td class="col-name left">${f.name}</td>
            <td class="col-size center">${formatSize(f.size)}</td>
            <td class="col-action center"><button class="danger" onclick="deleteFile('${f.name}')">Delete</button></td>
        </tr>`; 
    });
    const sInfo = document.getElementById('storageInfo');
    if (storage) {
        sInfo.innerText = `SD Card: ${formatSize(storage.total)} | Free: ${formatSize(storage.free)}`;
    } else {
        sInfo.innerText = 'SD Card not detected';
    }
  }
  async function saveTime() {
    const timeInput = document.getElementById('sTime');
    const currentTimeText = document.getElementById('currentTime').innerText;
    const newTime = timeInput.value;
    
    if (!newTime) { alert('Enter new duration!'); return; }
    if (newTime === currentTimeText) { alert('Duration is the same, not saved'); return; }
    
    await fetch('/api/time', { method: 'POST', body: newTime });
    timeInput.value = '';
    loadData();
  }
  async function uploadFile() {
    const fileInput = document.getElementById('fileInput'); 
    if (!fileInput.files[0]) { alert('Select MIDI file first!'); return; }

    const formData = new FormData(); 
    formData.append("file", fileInput.files[0]);

    const response = await fetch('/upload', { method: 'POST', body: formData });
    const text = await response.text();

    if (text === "SKIP") {
        alert('File already exists on SD Card!');
    } else if (text === "OK") {
        fileInput.value = ''; 
        document.querySelector('label[for=\'fileInput\']').innerText = 'Select MIDI File';
        loadData();
    } else {
        alert('Failed to upload file');
    }
  }
  async function addSolenoid() {
    const pin = parseInt(document.getElementById('sPin').value); 
    let note = document.getElementById('sNote').value; 
    const midi = parseInt(document.getElementById('sMidi').value);
    
    if(!pin || !midi) { alert('GPIO and MIDI Note Number are required!'); return; }
    if(!note) note = '-';
    
    const resS = await fetch('/api/solenoids'); let solenoids = await resS.json();
    
    if (solenoids.some(s => s.pin === pin || s.midi === midi)) {
        alert('GPIO or MIDI Note Number is already used!');
        return;
    }
    
    solenoids.push({pin: pin, note: note, midi: midi});
    await fetch('/api/solenoids', { method: 'POST', body: JSON.stringify(solenoids) });
    document.getElementById('sPin').value = ''; document.getElementById('sNote').value = ''; document.getElementById('sMidi').value = '';
    loadData();
  }
  async function removeSolenoid(pin) {
    const resS = await fetch('/api/solenoids'); let solenoids = await resS.json();
    solenoids = solenoids.filter(s => s.pin !== pin);
    await fetch('/api/solenoids', { method: 'POST', body: JSON.stringify(solenoids) });
    loadData();
  }
  async function saveWifi(isToggle = false) {
    const ssid = document.getElementById('wifiSsid').value;
    const pass = document.getElementById('wifiPass').value;
    const enable = document.getElementById('wifiEnable').checked;
    
    if (!ssid && enable) { alert('SSID is required if STA is enabled!'); return; }
    
    const res = await fetch('/api/wifi', { 
        method: 'POST', 
        body: JSON.stringify({ssid: ssid, pass: pass, enable: enable}) 
    });
    
    if (res.ok) {
        // No alert if successful (except on SSID error case, handled above)
        setTimeout(loadWifi, 500);
    } else {
        alert('Failed to save settings');
    }
  }
  async function deleteFile(name) { await fetch('/api/files?name='+name, { method: 'DELETE' }); loadData(); }
  setInterval(loadData, 3000);
  loadData();
  loadWifi();
  </script>
</body>
</html>
)rawliteral";

} // namespace

String sanitizeFilename(String filename) {
  String clean = "/";
  filename.toLowerCase();
  int lastSlash = filename.lastIndexOf('/');
  if (lastSlash >= 0)
    filename = filename.substring(lastSlash + 1);
  int lastBackslash = filename.lastIndexOf('\\');
  if (lastBackslash >= 0)
    filename = filename.substring(lastBackslash + 1);
  for (size_t i = 0; i < filename.length(); i++) {
    char c = filename[i];
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' ||
        c == '_' || c == '-')
      clean += c;
    else
      clean += '_';
  }
  return clean;
}

esp_err_t api_solenoids_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    if (digitalRead(PIN_SD_DET) == HIGH) {
      return httpd_resp_send(req, "[]", 2);
    }
    String json = "[";
    Solenoid *items = solenoid.getItems();
    for (uint8_t i = 0; i < solenoid.getCount(); i++) {
      json += "{\"pin\":" + String(items[i].getPin()) + ",\"note\":\"" +
              items[i].getNote() +
              "\",\"midi\":" + String(items[i].getMidiNote()) + "}";
      if (i < solenoid.getCount() - 1)
        json += ",";
    }
    json += "]";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[1024];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret > 0) {
      while (solenoid.getCount() > 0)
        solenoid.removeSolenoid(solenoid.getItems()[0].getPin());
      String data(buf);
      int start = 0;
      while ((start = data.indexOf("{\"pin\":", start)) >= 0) {
        int end = data.indexOf("}", start);
        String obj = data.substring(start, end + 1);
        int pStart = obj.indexOf(":") + 1;
        int pComma = obj.indexOf(",", pStart);
        int pin = obj.substring(pStart, pComma).toInt();
        int nStart = obj.indexOf(":", pComma) + 2;
        int nEnd = obj.indexOf("\"", nStart);
        String note = obj.substring(nStart, nEnd);
        int mStart = obj.indexOf(":", nEnd + 1) + 1;
        int mEnd = obj.indexOf("}", mStart);
        int midi = obj.substring(mStart, mEnd).toInt();
        solenoid.addSolenoid(pin, note, midi);
        start = end;
      }
      solenoid.saveConfig();
      httpd_resp_send(req, "OK", 2);
    }
    return ESP_OK;
  }
  return ESP_FAIL;
}

esp_err_t api_time_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    String json = String(player.getSolenoidTime());
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[16];
    memset(buf, 0, sizeof(buf));
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
      player.setSolenoidTime(String(buf).toInt());
      httpd_resp_send(req, "OK", 2);
    }
    return ESP_OK;
  }
  return ESP_FAIL;
}

esp_err_t api_files_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    String json = "{\"files\":[";
    if (digitalRead(PIN_SD_DET) == LOW) {
      File root = SD.open("/");
      File file = root.openNextFile();
      bool first = true;
      while (file) {
        String name = file.name();
        if (!file.isDirectory() &&
            (name.endsWith(".mid") || name.endsWith(".midi"))) {
          if (!first)
            json += ",";
          json += "{\"name\":\"" + name + "\",\"size\":" + String(file.size()) +
                  "}";
          first = false;
        }
        file = root.openNextFile();
      }
      json += "], \"storage\":{\"total\":" + String(SD.totalBytes()) +
              ", \"free\":" + String(SD.totalBytes() - SD.usedBytes()) + "}}";
      root.close();
    } else {
      json += "], \"storage\":null}";
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_DELETE) {
    char buf[256];
    size_t len = httpd_req_get_url_query_len(req);
    if (len < sizeof(buf)) {
      httpd_req_get_url_query_str(req, buf, len + 1);
      char name[128];
      if (httpd_query_key_value(buf, "name", name, sizeof(name)) == ESP_OK) {
        String decodedName = String(name);
        decodedName.replace("%20", " ");
        if (sdcard.deleteFile(("/" + decodedName).c_str())) {
          Serial.printf("[WEBSERVER]: File dihapus: %s\n", name);
          buzzer.uploadSuccess();
          needsScan = true;
          return httpd_resp_send(req, "OK", 2);
        }
      }
    }
    return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                               "Delete Failed");
  }
  return ESP_FAIL;
}

esp_err_t api_wifi_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    String ssid, pass;
    bool enable;
    wifiManager.getSettings(ssid, pass, enable);
    
    String json = "{\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"enable\":" + (enable ? "true" : "false") + "}";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
      String data(buf);
      Serial.printf("[WEBSERVER]: Received WiFi config: %s\n", data.c_str());

      String ssid = "", pass = "";
      bool enable = false;

      // Robust extraction
      int sIdx = data.indexOf("\"ssid\":\"");
      if (sIdx != -1) {
          int start = sIdx + 8;
          int end = data.indexOf("\"", start);
          if (end != -1) ssid = data.substring(start, end);
      }

      int pIdx = data.indexOf("\"pass\":\"");
      if (pIdx != -1) {
          int start = pIdx + 8;
          int end = data.indexOf("\"", start);
          if (end != -1) pass = data.substring(start, end);
      }

      int eIdx = data.indexOf("\"enable\":");
      if (eIdx != -1) {
          // Find "true" or "false" after the colon
          int colonIdx = data.indexOf(":", eIdx);
          if (colonIdx != -1) {
              String val = data.substring(colonIdx + 1);
              val.trim();
              if (val.startsWith("true")) enable = true;
              else if (val.startsWith("false")) enable = false;
          }
      }

      Serial.printf("[WEBSERVER]: Parsed WiFi - SSID: '%s', Enable: %s\n", ssid.c_str(), enable ? "ON" : "OFF");
      wifiManager.saveSettings(ssid, pass, enable);

      httpd_resp_send(req, "OK", 2);
    }
    return ESP_OK;
  }
  return ESP_FAIL;
}

esp_err_t root_handler(httpd_req_t *req) {
  String page = String(htmlPage);
  page.replace("{{FW_VERSION}}", FW_VERSION);
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, page.c_str(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t upload_handler(httpd_req_t *req) {
  char buf[1024];
  size_t recv_len;
  String filename = "";
  bool headersParsed = false;
  size_t header_offset = 0;
  File file;

  if (req->content_len > 0) {
    while ((recv_len = httpd_req_recv(req, buf, sizeof(buf))) > 0) {
      if (!headersParsed) {
        String chunk(buf, recv_len);
        int namePos = chunk.indexOf("filename=\"");
        if (namePos >= 0) {
          int start = namePos + 10;
          int end = chunk.indexOf("\"", start);
          if (end > start)
            filename = sanitizeFilename(chunk.substring(start, end));
        }
        int headerEnd = chunk.indexOf("\r\n\r\n");
        if (headerEnd >= 0) {
          headersParsed = true;
          header_offset = headerEnd + 4;

          // VALIDASI SERVER-SIDE: Jika file sudah ada, jangan tulis
          if (filename.length() > 0 && SD.exists(filename.c_str())) {
            Serial.printf("[WEBSERVER]: File sudah ada, skip: %s\n",
                          filename.c_str());
            return httpd_resp_send(req, "SKIP", 4);
          }

          if (filename.length() > 0 &&
              (filename.endsWith(".mid") || filename.endsWith(".midi"))) {
            file = sdcard.openFile(filename.c_str(), FILE_WRITE);
            if (!file)
              return ESP_FAIL;
            if (recv_len > header_offset)
              file.write((uint8_t *)(buf + header_offset),
                         recv_len - header_offset);
          } else
            return ESP_FAIL;
        }
      } else if (file)
        file.write((uint8_t *)buf, recv_len);
    }
  }
  if (file) {
    file.close();
    Serial.printf("[WEBSERVER]: File diunggah: %s\n", filename.c_str());
    needsScan = true;
    buzzer.uploadSuccess();
    return httpd_resp_send(req, "OK", 2);
  }
  return ESP_FAIL;
}

void WebServerManager::begin() {
  if (active)
    return;
  Serial.println(
      "[WEBSERVER]: Starting web server...");

  // DNS Server only runs in AP Mode
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.max_uri_handlers = 20; // Increase from default 8 to accommodate all handlers (we have ~16)
  if (httpd_start(&server, &config) != ESP_OK)
    return;

  // Initialize mDNS and register HTTP service AFTER web server has successfully started
  MDNS.end(); // Clear previous instances
  if (MDNS.begin("mydashboard")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[WEBSERVER]: mDNS started successfully: mydashboard.local");
  } else {
    Serial.println("[WEBSERVER]: ERROR: Failed to start mDNS");
  }

  // Start NetBIOS Name Service (highly reliable fallback for Windows)
  NBNS.begin("mydashboard");
  Serial.println("[WEBSERVER]: NetBIOS started: http://mydashboard");

  httpd_uri_t root_uri = {"/", HTTP_GET, root_handler, nullptr};
  httpd_uri_t upload_uri = {"/upload", HTTP_POST, upload_handler, nullptr};
  httpd_uri_t solenoids_get_uri = {"/api/solenoids", HTTP_GET,
                                   api_solenoids_handler, nullptr};
  httpd_uri_t solenoids_post_uri = {"/api/solenoids", HTTP_POST,
                                    api_solenoids_handler, nullptr};
  httpd_uri_t time_get_uri = {"/api/time", HTTP_GET, api_time_handler, nullptr};
  httpd_uri_t time_post_uri = {"/api/time", HTTP_POST, api_time_handler,
                               nullptr};
  httpd_uri_t files_get_uri = {"/api/files", HTTP_GET, api_files_handler,
                               nullptr};
  httpd_uri_t files_delete_uri = {"/api/files", HTTP_DELETE, api_files_handler,
                                  nullptr};
  httpd_uri_t wifi_get_uri = {"/api/wifi", HTTP_GET, api_wifi_handler,
                               nullptr};
  httpd_uri_t wifi_post_uri = {"/api/wifi", HTTP_POST, api_wifi_handler,
                               nullptr};
  httpd_register_uri_handler(server, &root_uri);
  httpd_register_uri_handler(server, &upload_uri);
  httpd_register_uri_handler(server, &solenoids_get_uri);
  httpd_register_uri_handler(server, &solenoids_post_uri);
  httpd_register_uri_handler(server, &time_get_uri);
  httpd_register_uri_handler(server, &time_post_uri);
  httpd_register_uri_handler(server, &files_get_uri);
  httpd_register_uri_handler(server, &files_delete_uri);
  httpd_register_uri_handler(server, &wifi_get_uri);
  httpd_register_uri_handler(server, &wifi_post_uri);

  // Captive Portal Detection Handlers
  const char *captive_paths[] = {"/generate_204", "/gen_204",
                                 "/redirect",     "/connecttest.txt",
                                 "/ncsi.txt",     "/hotspot-detect.html"};
  for (const char *path : captive_paths) {
    httpd_uri_t uri = {path, HTTP_GET, root_handler, nullptr};
    httpd_register_uri_handler(server, &uri);
  }

  active = true;
  Serial.println("[WEBSERVER]: Web server started successfully.");
}

void WebServerManager::update() {
  if (!active)
    return;
  dnsServer.processNextRequest();
  if (needsScan) {
    sdcard.scan();
    needsScan = false;
  }
}

void WebServerManager::stop() {
  if (!active)
    return;
  
  Serial.println("[WEBSERVER]: Stopping web server...");
  if (server) {
    httpd_stop(server);
    server = nullptr;
  }
  dnsServer.stop();
  NBNS.end(); // Stop NetBIOS service
  
  display.showStatus("WIFI OFF");
  active = false;
}

bool WebServerManager::isActive() const { return active; }
