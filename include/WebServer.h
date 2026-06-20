#pragma once

#include "SensorStats.h"

void setupWebServer();
void sendWebSocketData(
    int bpmValue,
    int spo2Value,
    HeartRateStats* statsPtr
);
void cleanupWebSocketClients();