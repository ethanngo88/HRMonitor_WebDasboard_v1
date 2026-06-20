#pragma once

#include <Arduino.h>
#include "SensorStats.h"

bool initSensor();
void updateSensor();
int getCurrentBpm();
int getCurrentSpo2();
bool isFingerPresent();

