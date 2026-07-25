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
  </style>
</head>
<body>
<div class="card">
  <h2>Unggah File MIDI</h2>
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
    " onclick="document.getElementById('fileInput').click()">Pilih File MIDI</label>
    <input type="file" id="fileInput" accept=".mid,.midi" style="display:none;" onchange="document.querySelector('label[for=\'fileInput\']').innerText = this.files[0].name" />
    <button onclick="uploadFile()" class="primary">Unggah</button>
    </div>
    </div>
<div class="card">
  <h2>Manajer File</h2>
  <table>
    <thead>
      <tr>
        <th class="col-name left">Nama</th>
        <th class="col-size center">Ukuran</th>
        <th class="col-action center">Aksi</th>
      </tr>
    </thead>
    <tbody id="fileBody"></tbody>
  </table>
  <div id="storageInfo" style="margin-top: 10px; font-size: 0.9rem; color: var(--text-muted); text-align: center;"></div>
</div>
<div class="card">
  <h2>Durasi Aktif Aktuator</h2>
  <div class="row" style="justify-content: space-between;">
    <span>Durasi Saat Ini : <strong id="currentTime">...</strong> ms</span>
  </div>
  <div class="row">
    <input type="number" id="sTime" placeholder="Masukkan Durasi Baru" />
    <button onclick="saveTime()" class="primary">Simpan</button>
  </div>
</div>
<div class="card">
  <h2>Manajer Aktuator</h2>
  <div style="display: flex; flex-direction: column; gap: 10px; margin-bottom: 15px;">
    <input type="number" id="sPin" placeholder="GPIO" />
    <input type="text" id="sNote" placeholder="Nada" />
    <input type="number" id="sMidi" placeholder="Nomor Not MIDI" />
    <button onclick="addSolenoid()" class="primary">Tambah dan Simpan</button>
  </div>
  <table>
    <thead>
      <tr>
        <th class="col-pin left">GPIO</th>
        <th class="col-note left">Nada</th>
        <th class="col-midi left">MIDI</th>
        <th class="col-s-action center">Aksi</th>
      </tr>
    </thead>
    <tbody id="solenoidBody"></tbody>
  </table>
</div>
<div class="card">
  <h2>Konfigurasi WiFi STA</h2>
  <div style="display: flex; flex-direction: column; gap: 10px;">
    <input type="text" id="wSSID" placeholder="SSID WiFi" />
    <input type="text" id="wPass" placeholder="Password WiFi" />
    <label style="display: flex; align-items: center; gap: 10px; cursor: pointer;">
      <input type="checkbox" id="wEnabled" style="flex-grow: 0; width: 20px;" /> Aktifkan WiFi STA
    </label>
    <button onclick="saveWifi()" class="primary">Simpan Konfigurasi</button>
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
      const resS = await fetch('/api/solenoids?t=' + t); const solenoids = await resS.json();
      const resF = await fetch('/api/files?t=' + t); const filesRes = await resF.json();
      const resT = await fetch('/api/time?t=' + t); const time = await resT.json();
      
      document.getElementById('currentTime').innerText = time;
      
      render(solenoids, filesRes.files, filesRes.storage);
  }
  function formatSize(bytes) {
      if (bytes < 1024) return bytes + 'b';
      if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + 'kb';
      if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + 'mb';
      return (bytes / (1024 * 1024 * 1024)).toFixed(1) + 'gb';
  }
  function render(solenoids, files, storage) {
    const sBody = document.getElementById('solenoidBody'); sBody.innerHTML = '';
    solenoids.forEach(s => { 
        sBody.innerHTML += `<tr>
            <td class="col-pin left">${s.pin}</td>
            <td class="col-note left">${s.note}</td>
            <td class="col-midi left">${s.midi}</td>
            <td class="col-s-action center"><button class="danger" onclick="removeSolenoid(${s.pin})">Hapus</button></td>
        </tr>`; 
    });
    const fBody = document.getElementById('fileBody'); fBody.innerHTML = '';
    files.forEach(f => { 
        fBody.innerHTML += `<tr>
            <td class="col-name left">${f.name}</td>
            <td class="col-size center">${formatSize(f.size)}</td>
            <td class="col-action center"><button class="danger" onclick="deleteFile('${f.name}')">Hapus</button></td>
        </tr>`; 
    });
    const sInfo = document.getElementById('storageInfo');
    if (storage) {
        sInfo.innerText = `SD Card: ${formatSize(storage.total)} | Sisa: ${formatSize(storage.free)}`;
    } else {
        sInfo.innerText = 'SD Card tidak terdeteksi';
    }
  }
  async function saveTime() {
    const timeInput = document.getElementById('sTime');
    const currentTimeText = document.getElementById('currentTime').innerText;
    const newTime = timeInput.value;
    
    if (!newTime) { alert('Masukkan durasi baru!'); return; }
    if (newTime === currentTimeText) { alert('Durasi sama, tidak disimpan'); return; }
    
    await fetch('/api/time', { method: 'POST', body: newTime });
    timeInput.value = '';
    loadData();
  }
  async function uploadFile() {
    const fileInput = document.getElementById('fileInput'); 
    if (!fileInput.files[0]) { alert('Pilih file MIDI terlebih dahulu!'); return; }

    const formData = new FormData(); 
    formData.append("file", fileInput.files[0]);

    // Kirim file dan tunggu respons
    const response = await fetch('/upload', { method: 'POST', body: formData });
    const text = await response.text();

    if (text === "SKIP") {
        alert('File sudah ada di SD Card!');
    } else if (text === "OK") {
        fileInput.value = ''; 
        document.querySelector('label[for=\'fileInput\']').innerText = 'Pilih File MIDI';
        loadData();
    } else {
        alert('Gagal mengunggah file');
    }
  }
  async function addSolenoid() {
    const pin = parseInt(document.getElementById('sPin').value); 
    let note = document.getElementById('sNote').value; 
    const midi = parseInt(document.getElementById('sMidi').value);
    
    if(!pin || !midi) { alert('GPIO dan Nomor Not MIDI wajib diisi!'); return; }
    if(!note) note = '-';
    
    const resS = await fetch('/api/solenoids'); let solenoids = await resS.json();
    
    // Validasi duplikasi
    if (solenoids.some(s => s.pin === pin || s.midi === midi)) {
        alert('GPIO atau Nomor Not MIDI tersebut sudah digunakan!');
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
  async function deleteFile(name) { await fetch('/api/files?name='+name, { method: 'DELETE' }); loadData(); }
  async function loadWifi() {
    const resW = await fetch('/api/wifi'); const wifi = await resW.json();
    document.getElementById('wSSID').value = wifi.ssid;
    document.getElementById('wPass').value = wifi.pass;
    document.getElementById('wEnabled').checked = wifi.enabled;
  }
  async function saveWifi() {
    const ssid = document.getElementById('wSSID').value;
    const pass = document.getElementById('wPass').value;
    const enabled = document.getElementById('wEnabled').checked;
    if (!ssid || !pass) { alert('SSID dan Password tidak boleh kosong!'); return; }
    
    const wifi = { ssid: ssid, pass: pass, enabled: enabled };
    await fetch('/api/wifi', { method: 'POST', body: JSON.stringify(wifi) });
    // Data tersimpan, load ulang untuk memastikan konsistensi
    loadWifi();
  }
  async function applyWifi() {
    if(!confirm('Perangkat akan reboot untuk menerapkan pengaturan WiFi. Lanjutkan?')) return;
    await fetch('/api/wifi/apply', { method: 'POST' });
    location.reload();
  }
  setInterval(loadData, 2000);
  loadData();
  loadWifi();
  </script>
</body>
</html>
)rawliteral";

}

String sanitizeFilename(String filename) {
  String clean = "/";
  filename.toLowerCase();
  int lastSlash = filename.lastIndexOf('/');
  if (lastSlash >= 0) filename = filename.substring(lastSlash + 1);
  int lastBackslash = filename.lastIndexOf('\\');
  if (lastBackslash >= 0) filename = filename.substring(lastBackslash + 1);
  for (size_t i = 0; i < filename.length(); i++) {
    char c = filename[i];
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') clean += c;
    else clean += '_';
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
      json += "{\"pin\":" + String(items[i].getPin()) + ",\"note\":\"" + items[i].getNote() + "\",\"midi\":" + String(items[i].getMidiNote()) + "}";
      if (i < solenoid.getCount() - 1) json += ",";
    }
    json += "]";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[1024];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret > 0) {
      while (solenoid.getCount() > 0) solenoid.removeSolenoid(solenoid.getItems()[0].getPin());
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

esp_err_t api_wifi_handler(httpd_req_t *req) {
  Serial.println("[API_WIFI] Handler called!");
  if (req->method == HTTP_GET) {
    String ssid, pass;
    bool enabled;
    otaWifi.getConfig(ssid, pass, enabled);
    String json = "{\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"enabled\":" + (enabled ? "true" : "false") + "}";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json.c_str(), json.length());
  } else if (req->method == HTTP_POST) {
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
      buf[ret] = '\0';
      Serial.printf("[API_WIFI] Received Body: %s\n", buf);
      
      // Simple parsing
      String data(buf);
      bool enabled = (data.indexOf("true") != -1); // Simple check
      // SSID/PASS parsing is tricky in simple string, assume standard JSON format
      
      // Send OK
      httpd_resp_send(req, "OK", 2);
    }
    return ESP_OK;
  }
  return ESP_FAIL;
}

esp_err_t api_wifi_apply_handler(httpd_req_t *req) {
  if (req->method == HTTP_POST) {
    httpd_resp_send(req, "REBOOTING", 9);
    delay(1000);
    ESP.restart();
    return ESP_OK;
  }
  return ESP_FAIL;
}


esp_err_t api_files_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    String json = "{\"files\":[";
    // Debug: Try listing regardless of PIN_SD_DET
    File root = SD.open("/");
    if (root) {
        File file = root.openNextFile();
        bool first = true;
        while (file) {
          String name = file.name();
          if (!file.isDirectory() && (name.endsWith(".mid") || name.endsWith(".midi"))) {
            if (!first) json += ",";
            json += "{\"name\":\"" + name + "\",\"size\":" + String(file.size()) + "}";
            first = false;
          }
          file = root.openNextFile();
        }
        json += "], \"storage\":{\"total\":" + String(SD.totalBytes()) + ", \"free\":" + String(SD.totalBytes() - SD.usedBytes()) + "}}";
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
          needsScan = true;
          return httpd_resp_send(req, "OK", 2);
        }
      }
    }
    return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Delete Failed");
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
          if (end > start) filename = sanitizeFilename(chunk.substring(start, end));
        }
        int headerEnd = chunk.indexOf("\r\n\r\n");
        if (headerEnd >= 0) {
          headersParsed = true;
          header_offset = headerEnd + 4;

          if (filename.length() > 0 && SD.exists(filename.c_str())) {
            return httpd_resp_send(req, "SKIP", 4);
          }

          if (filename.length() > 0 && (filename.endsWith(".mid") || filename.endsWith(".midi"))) {
            file = sdcard.openFile(filename.c_str(), FILE_WRITE);
            if (!file) return ESP_FAIL;
            if (recv_len > header_offset) file.write((uint8_t *)(buf + header_offset), recv_len - header_offset);
          } else return ESP_FAIL;
        }
      } else if (file) file.write((uint8_t *)buf, recv_len);
    }
  }
  if (file) {
    file.close();
    needsScan = true;
    return httpd_resp_send(req, "OK", 2);
  }
  return ESP_FAIL;
}

void WebServerManager::begin() {
  if (active) return;
  Serial.println("[WEBSERVER]: Akses http://mydashboard.local/ untuk konfigurasi");

  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  dnsServer.start(53, "*", WiFi.softAPIP());
  MDNS.begin("mydashboard");
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  if (httpd_start(&server, &config) != ESP_OK) return;
  // ... (inside WebServerManager::begin)

  // Root handler must be registered LAST or specifically
  httpd_uri_t root_uri = { "/", HTTP_GET, root_handler, nullptr };
  httpd_register_uri_handler(server, &root_uri);
  
  // API handlers
  httpd_uri_t upload_uri = { "/upload", HTTP_POST, upload_handler, nullptr };
  httpd_register_uri_handler(server, &upload_uri);
  
  httpd_uri_t sol_get = { "/api/solenoids", HTTP_GET, api_solenoids_handler, nullptr };
  httpd_register_uri_handler(server, &sol_get);
  httpd_uri_t sol_post = { "/api/solenoids", HTTP_POST, api_solenoids_handler, nullptr };
  httpd_register_uri_handler(server, &sol_post);
  
  httpd_uri_t time_get = { "/api/time", HTTP_GET, api_time_handler, nullptr };
  httpd_register_uri_handler(server, &time_get);
  httpd_uri_t time_post = { "/api/time", HTTP_POST, api_time_handler, nullptr };
  httpd_register_uri_handler(server, &time_post);
  
  httpd_uri_t file_get = { "/api/files", HTTP_GET, api_files_handler, nullptr };
  httpd_register_uri_handler(server, &file_get);
  httpd_uri_t file_del = { "/api/files", HTTP_DELETE, api_files_handler, nullptr };
  httpd_register_uri_handler(server, &file_del);
  
  httpd_uri_t wifi_get = { "/api/wifi", HTTP_GET, api_wifi_handler, nullptr };
  httpd_register_uri_handler(server, &wifi_get);
  httpd_uri_t wifi_post = { "/api/wifi", HTTP_POST, api_wifi_handler, nullptr };
  httpd_register_uri_handler(server, &wifi_post);
  httpd_uri_t wifi_app = { "/api/wifi/apply", HTTP_POST, api_wifi_apply_handler, nullptr };
  httpd_register_uri_handler(server, &wifi_app);

  // ... (rest of captive portal)

  // Captive Portal Detection Handlers
  const char *captive_paths[] = {
    "/generate_204",
    "/gen_204",
    "/redirect",
    "/connecttest.txt",
    "/ncsi.txt",
    "/hotspot-detect.html"
  };
  for (const char *path : captive_paths) {
    httpd_uri_t uri = { path, HTTP_GET, root_handler, nullptr };
    httpd_register_uri_handler(server, &uri);
  }

  active = true;
}

void WebServerManager::update() {
  if (!active) return;
  dnsServer.processNextRequest();
  if (needsScan) {
    sdcard.scan();
    needsScan = false;
  }
}

void WebServerManager::stop() {
  if (!active) return;
  if (server) {
    httpd_stop(server);
    server = nullptr;
  }
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  display.showStatus("WIFI OFF");
  active = false;
}

bool WebServerManager::isActive() const {
  return active;
}
