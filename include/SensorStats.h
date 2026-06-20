#pragma once

#include <Arduino.h>

struct HeartRateStats {
  int maxBpm;
  int minBpm;
  float avgBpm;
  float hrv;
  bool isAnomaly;
  int anomalyType; // 0=normal, 1=too fast, 2=too slow, 3=irregular
};

void initStats();
void recordBeatEvent(int bpm);
HeartRateStats* getStats();

void resetStats();
bool detectAnomaly(int bpm, float avgBpm);
float calculateHRV();
int getHeartRateLevel(int bpm);
HeartRateStats* getStatsPtr();

void updateStatistics(int bpm);
