#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MAX30105 sensor;
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");

const char* ssid = "Runners Room";
const char* password = "homong123";
const char* apSsid = "HRMonitor_AP";
const char* apPassword = "12345678";

long lastBeat = 0;
float bpmAvg = 0;
float bpmSmooth = 0;

#define BPM_BUFFER_SIZE 8
float bpmBuffer[BPM_BUFFER_SIZE];
int bpmIndex = 0;

#define BUFFER_SIZE 50
long irBuffer[BUFFER_SIZE];
long redBuffer[BUFFER_SIZE];
int bufferIndex = 0;
float spo2 = 0;
unsigned long lastWebSocketSend = 0;

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Healthcare IoT Dashboard</title>
    <link rel="stylesheet" href="/style.css">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
<div class="dashboard">
    <aside class="sidebar">
        <div class="logo">
            <div class="logo-icon">❤</div>
            <div>
                <h2>HealthIoT</h2>
                <p>Realtime Monitor</p>
            </div>
        </div>
        <ul class="menu">
            <li class="active">Dashboard</li>
            <li>Analytics</li>
            <li>Devices</li>
            <li>Reports</li>
            <li>Settings</li>
        </ul>
    </aside>
    <main class="main-content">
        <header class="topbar">
            <div>
                <h1>Healthcare IoT Dashboard</h1>
                <p>ESP32 + MAX30102 + WebSocket Realtime</p>
            </div>
            <div class="connection-box">
                <span class="dot" id="dot"></span>
                <span id="status">Disconnected</span>
            </div>
        </header>
        <section class="cards">
            <div class="card red-card">
                <div class="card-top">
                    <div>
                        <p class="card-title">Heart Rate</p>
                        <h2 class="card-value" id="heartRate">0</h2>
                        <span class="card-unit">BPM</span>
                    </div>
                    <div class="card-icon red-icon">❤</div>
                </div>
                <div class="card-footer">Live monitoring active</div>
            </div>
            <div class="card green-card">
                <div class="card-top">
                    <div>
                        <p class="card-title">Blood Oxygen</p>
                        <h2 class="card-value" id="spo2">0</h2>
                        <span class="card-unit">%</span>
                    </div>
                    <div class="card-icon green-icon">🫁</div>
                </div>
                <div class="card-footer">Oxygen saturation tracking</div>
            </div>
            <div class="card blue-card">
                <div class="card-top">
                    <div>
                        <p class="card-title">Device Status</p>
                        <div class="device-list">
                            <p>ESP32 Connected</p>
                            <p>Sensor Active</p>
                            <p>WebSocket Running</p>
                        </div>
                    </div>
                    <div class="card-icon blue-icon">📡</div>
                </div>
                <div class="card-footer">Edge IoT Device Online</div>
            </div>
        </section>
        <section class="chart-section">
            <div class="chart-header">
                <h2>Realtime Health Analytics</h2>
            </div>
            <canvas id="healthChart"></canvas>
        </section>
    </main>
</div>
<script src="/app.js"></script>
</body>
</html>)rawliteral";

const char app_js[] PROGMEM = R"rawliteral(// =========================
// ELEMENTS
// =========================
const heartRateElement = document.getElementById("heartRate");
const spo2Element = document.getElementById("spo2");
const statusElement = document.getElementById("status");
const dotElement = document.getElementById("dot");

// =========================
// CHART DATA
// =========================
const labels = [];
const heartRateData = [];
const spo2Data = [];

// =========================
// CHART CONFIG
// =========================
const ctx = document.getElementById("healthChart").getContext("2d");
const healthChart = new Chart(ctx, {
    type: "line",
    data: {
        labels: labels,
        datasets: [
            {
                label: "Heart Rate",
                data: heartRateData,
                borderColor: "#ef4444",
                backgroundColor: "rgba(239,68,68,0.2)",
                borderWidth: 3,
                fill: true,
                tension: 0.4
            },
            {
                label: "SpO2",
                data: spo2Data,
                borderColor: "#22c55e",
                backgroundColor: "rgba(34,197,94,0.2)",
                borderWidth: 3,
                fill: true,
                tension: 0.4
            }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        interaction: {
            intersect: false,
            mode: "index"
        },
        plugins: {
            legend: {
                labels: {
                    color: "white",
                    font: { size: 14 }
                }
            }
        },
        scales: {
            x: {
                ticks: { color: "#cbd5e1" },
                grid: { color: "rgba(255,255,255,0.05)" }
            },
            y: {
                beginAtZero: true,
                ticks: { color: "#cbd5e1" },
                grid: { color: "rgba(255,255,255,0.05)" }
            }
        }
    }
});

// =========================
// WEBSOCKET
// =========================
const gateway = `ws://${window.location.hostname}/ws`;
let websocket;

function initWebSocket() {
    console.log("Connecting WebSocket...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen() {
    console.log("Connected");
    statusElement.innerHTML = "Connected";
    dotElement.style.background = "#22c55e";
}

function onClose() {
    console.log("Disconnected");
    statusElement.innerHTML = "Disconnected";
    dotElement.style.background = "#ef4444";
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    const data = JSON.parse(event.data);
    const heartRate = data.heartRate;
    const spo2 = data.spo2;
    heartRateElement.innerHTML = heartRate;
    spo2Element.innerHTML = spo2;
    const time = new Date().toLocaleTimeString();
    labels.push(time);
    heartRateData.push(heartRate);
    spo2Data.push(spo2);
    if (labels.length > 20) {
        labels.shift();
        heartRateData.shift();
        spo2Data.shift();
    }
    healthChart.update();
}

window.addEventListener("load", initWebSocket);
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(*{margin:0;padding:0;box-sizing:border-box;}body{font-family:'Poppins',sans-serif;background:linear-gradient(135deg,#0f172a,#111827,#1e293b);color:white;min-height:100vh;}.dashboard{display:flex;min-height:100vh;}.sidebar{width:260px;background:rgba(255,255,255,0.05);backdrop-filter:blur(15px);border-right:1px solid rgba(255,255,255,0.08);padding:30px 20px;}.logo{display:flex;align-items:center;gap:15px;margin-bottom:50px;}.logo-icon{width:55px;height:55px;border-radius:15px;display:flex;justify-content:center;align-items:center;font-size:24px;background:linear-gradient(135deg,#ef4444,#dc2626);}.logo h2{font-size:28px;}.logo p{color:#94a3b8;font-size:14px;}.menu{list-style:none;}.menu li{padding:16px 18px;border-radius:14px;margin-bottom:12px;cursor:pointer;transition:0.3s;}.menu li:hover,.menu .active{background:rgba(255,255,255,0.08);}.main-content{flex:1;padding:30px;}.topbar{display:flex;justify-content:space-between;align-items:center;margin-bottom:35px;}.topbar h1{font-size:38px;margin-bottom:8px;}.topbar p{color:#94a3b8;}.connection-box{display:flex;align-items:center;gap:12px;padding:14px 20px;border-radius:14px;background:rgba(255,255,255,0.06);border:1px solid rgba(255,255,255,0.08);}.dot{width:12px;height:12px;border-radius:50%;background:red;animation:pulse 1.5s infinite;}@keyframes pulse{0%{transform:scale(1);}50%{transform:scale(1.4);}100%{transform:scale(1);}}.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:25px;margin-bottom:30px;}.card{background:rgba(255,255,255,0.07);backdrop-filter:blur(15px);border:1px solid rgba(255,255,255,0.08);border-radius:28px;padding:28px;transition:0.4s;overflow:hidden;position:relative;}.card:hover{transform:translateY(-8px);box-shadow:0 10px 35px rgba(0,0,0,0.3);}.card::before{content:"";position:absolute;width:200px;height:200px;border-radius:50%;top:-100px;right:-100px;opacity:0.1;}.red-card::before{background:#ef4444;}.green-card::before{background:#22c55e;}.blue-card::before{background:#38bdf8;}.card-top{display:flex;justify-content:space-between;align-items:flex-start;}.card-title{color:#cbd5e1;margin-bottom:10px;}.card-value{font-size:72px;font-weight:700;margin-bottom:5px;}.card-unit{color:#94a3b8;}.card-icon{width:70px;height:70px;border-radius:20px;display:flex;justify-content:center;align-items:center;font-size:32px;}.red-icon{background:rgba(239,68,68,0.15);}.green-icon{background:rgba(34,197,94,0.15);}.blue-icon{background:rgba(56,189,248,0.15);}.card-footer{margin-top:25px;padding-top:18px;border-top:1px solid rgba(255,255,255,0.08);color:#94a3b8;}.device-list p{margin-bottom:10px;}.chart-section{background:rgba(255,255,255,0.07);backdrop-filter:blur(15px);border:1px solid rgba(255,255,255,0.08);border-radius:28px;padding:30px;}.chart-header{margin-bottom:25px;}.chart-header h2{font-size:28px;}canvas{width:100% !important;height:450px !important;}@media(max-width:1000px){.dashboard{flex-direction:column;}.sidebar{width:100%;}}@media(max-width:700px){.topbar{flex-direction:column;align-items:flex-start;gap:20px;}.card-value{font-size:52px;}.topbar h1{font-size:28px;}}
)rawliteral";

float estimateBPM(float spo2Value) {
  if (spo2Value > 100) spo2Value = 100;
  if (spo2Value < 90) spo2Value = 90;
  return 65 + (100 - spo2Value) * 4.5;
}

void onWebSocketEvent(AsyncWebSocket *serverPtr, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket client connected: %u\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket client disconnected: %u\n", client->id());
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "application/javascript", app_js);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/css", style_css);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  webSocket.onEvent(onWebSocketEvent);
  server.addHandler(&webSocket);
  server.begin();
}

void connectWiFi() {
  Serial.println("\n--- Bat dau ket noi WiFi ---");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to WiFi '%s'...\n", ssid);

  unsigned long start = millis();
  // Vòng lặp chờ tối đa 10 giây để kết nối WiFi
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  // KHẮC PHỤC TẠI ĐÂY: Đợi thêm cho đến khi thực sự có IP hợp lệ (khác 0.0.0.0)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Da ket noi Router. Dang cho cap IP...");
    
    int timeout = 0;
    while (WiFi.localIP().toString() == "0.0.0.0" && timeout < 10) {
      delay(500);
      Serial.print("#");
      timeout++;
    }

    Serial.println("\n=================================");
    Serial.print("WiFi CONNECTED SUCCESSFULLY!\n");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); 
    Serial.println("=================================");
  } 
  else {
    Serial.println("\n[FAIL] Không the ket noi WiFi, chuyen sang AP mode...");
    WiFi.softAP(apSsid, apPassword);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED fail");
    while (true);
  }

  display.setTextColor(WHITE);

  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor fail");
    while (true);
  }

  sensor.setup(0x3F, 4, 2, 100, 411, 4096);
  sensor.setPulseAmplitudeIR(0x3F);
  sensor.setPulseAmplitudeRed(0x3F);
  sensor.setPulseAmplitudeGreen(0);

  Serial.println("System ready");
  Serial.println("AAAAAAAAAAA");
  connectWiFi();
  Serial.println("2");
  setupWebServer();
  Serial.println("3");
}

void sendWebSocketData(int bpmValue, int spo2Value) {
  char message[64];
  snprintf(message, sizeof(message), "{\"heartRate\":%d,\"spo2\":%d}", bpmValue, spo2Value);
  webSocket.textAll(message);
}

void loop() {
  long ir = sensor.getIR();
  long red = sensor.getRed();

  if (ir < 30000) {
    bpmAvg = 0;
    bpmSmooth = 0;
    spo2 = 0;

    for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
      bpmBuffer[i] = 0;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.println("NO FINGER");
    display.display();

    if (millis() - lastWebSocketSend > 1000) {
      sendWebSocketData(0, 0);
      lastWebSocketSend = millis();
    }

    webSocket.cleanupClients();
    delay(100);
    return;
  }

  if (checkForBeat(ir)) {
    long now = millis();
    long delta = now - lastBeat;
    lastBeat = now;

    float bpm = 60.0 / (delta / 1000.0);
    if (bpm > 45 && bpm < 150) {
      if (abs(bpm - bpmAvg) < 20 || bpmAvg == 0) {
        bpmBuffer[bpmIndex++] = bpm;
        bpmIndex %= BPM_BUFFER_SIZE;

        float sum = 0;
        for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
          sum += bpmBuffer[i];
        }
        bpmAvg = sum / BPM_BUFFER_SIZE;
      }
    }
  }

  irBuffer[bufferIndex] = ir;
  redBuffer[bufferIndex] = red;
  bufferIndex++;

  if (bufferIndex >= BUFFER_SIZE) {
    bufferIndex = 0;

    float irDC = 0, redDC = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      irDC += irBuffer[i];
      redDC += redBuffer[i];
    }

    irDC /= BUFFER_SIZE;
    redDC /= BUFFER_SIZE;

    float irAC = 0, redAC = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      irAC += abs(irBuffer[i] - irDC);
      redAC += abs(redBuffer[i] - redDC);
    }

    irAC /= BUFFER_SIZE;
    redAC /= BUFFER_SIZE;

    float R = (redAC / redDC) / (irAC / irDC);
    spo2 = 110 - 25 * R;

    if (spo2 > 100) spo2 = 100;
    if (spo2 < 90) spo2 = 90;
  }

  float bpmRaw = 0;
  if (spo2 > 0) {
    bpmRaw = (bpmAvg >= 45) ? bpmAvg : estimateBPM(spo2);
  }

  if (bpmRaw > 0) {
    bpmSmooth = 0.85 * bpmSmooth + 0.15 * bpmRaw;
  } else {
    bpmSmooth *= 0.98;
  }

  if (bpmSmooth < 50 && bpmSmooth > 0) bpmSmooth = 50;
  if (bpmSmooth > 120) bpmSmooth = 120;

  int displayBpm = (int)bpmSmooth;
  int displaySpo2 = (int)spo2;

  Serial.print("BPM: ");
  Serial.print(displayBpm);
  Serial.print(" | SpO2: ");
  Serial.println(displaySpo2);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("BPM:");
  if (displayBpm == 0) display.println("--");
  else display.println(displayBpm);
  display.setCursor(0, 30);
  display.print("SpO2:");
  if (displaySpo2 == 0) display.print("--");
  else {
    display.print(displaySpo2);
    display.print("%");
  }
  display.display();

  if (millis() - lastWebSocketSend > 1000) {
    sendWebSocketData(displayBpm, displaySpo2);
    lastWebSocketSend = millis();
  }

  webSocket.cleanupClients();
  delay(20);
}
