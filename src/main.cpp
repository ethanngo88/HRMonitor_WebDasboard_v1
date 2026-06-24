#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#include "EmbeddedWebAssets.h"
#include "SensorFilter.h"
#include "SensorStats.h"
#include "WebServer.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "phongrunners";
const char* password = "homong123";
const char* apSsid = "HRMonitor_AP";
const char* apPassword = "12345678";

unsigned long lastWebSocketSend = 0;
unsigned long lastStatsUpdate = 0;

void connectWiFi() {
  Serial.println("\n--- Bat dau ket noi WiFi ---");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to WiFi '%s'...\n", ssid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

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
  } else {
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

  if (!initSensor()) {
    Serial.println("Sensor fail");
    while (true);
  }

  Serial.println("System ready");
  connectWiFi();
  setupWebServer();
}

void loop() {
  updateSensor();
  int displayBpm = getCurrentBpm();
  int displaySpo2 = getCurrentSpo2();

  if (millis() - lastStatsUpdate > 1000)
{
    updateStatistics(displayBpm);
    lastStatsUpdate = millis();
}
  
  if (!isFingerPresent()) {
    resetStats();
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.println("NO FINGER");
    display.display();
    static unsigned long lastStatsUpdate = 0;

    if (millis() - lastStatsUpdate > 1000)
    {
        updateStatistics(displayBpm);
        lastStatsUpdate = millis();
    }

    if (millis() - lastWebSocketSend > 1000) {
      sendWebSocketData(0, 0, getStatsPtr());
      lastWebSocketSend = millis();
    }

    cleanupWebSocketClients();
    delay(100);
    return;
  }

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
    if (millis() - lastWebSocketSend > 1000) {
    sendWebSocketData(displayBpm, displaySpo2, getStatsPtr());
    lastWebSocketSend = millis();
}
  }

  cleanupWebSocketClients();
  delay(20);
}
