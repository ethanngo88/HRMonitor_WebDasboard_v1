#include "WebServer.h"
#include "SensorStats.h"
#include "SensorStats.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Arduino.h>

static AsyncWebServer server(80);
static AsyncWebSocket webSocket("/ws");

extern const char index_html[] PROGMEM;
extern const char app_js[] PROGMEM;
extern const char style_css[] PROGMEM;

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

void sendWebSocketData(
    int bpmValue,
    int spo2Value,
    HeartRateStats* stats
)
{
    if (!stats) return;

    char message[256];

    snprintf(
        message,
        sizeof(message),
        "{\"heartRate\":%d,\"spo2\":%d,"
        "\"stats\":{\"maxBpm\":%d,\"minBpm\":%d,"
        "\"avgBpm\":%.1f,\"hrv\":%.1f},"
        "\"isAnomaly\":%s,\"anomalyType\":%d}",
        bpmValue,
        spo2Value,
        stats->maxBpm,
        stats->minBpm,
        stats->avgBpm,
        stats->hrv,
        stats->isAnomaly ? "true" : "false",
        stats->anomalyType
    );

    webSocket.textAll(message);
}

void cleanupWebSocketClients() {
  webSocket.cleanupClients();
}
