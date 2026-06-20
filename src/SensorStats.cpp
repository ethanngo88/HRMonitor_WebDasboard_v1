#include "SensorStats.h"
#include <cmath>

#define STATS_HISTORY_SIZE 60  // Lưu 60 beat gần nhất

static int bpmHistory[STATS_HISTORY_SIZE];
static int historyIndex = 0;
static int historyCount = 0;

static HeartRateStats currentStats = {0, 200, 0, 0, false, 0};

// Threshold cho anomaly detection
#define TACHYCARDIA_THRESHOLD 100
#define BRADYCARDIA_THRESHOLD 60
#define NORMAL_HEART_RATE_MIN 60
#define NORMAL_HEART_RATE_MAX 100

void initStats() {
  for (int i = 0; i < STATS_HISTORY_SIZE; i++) {
    bpmHistory[i] = 0;
  }
  historyIndex = 0;
  historyCount = 0;
  currentStats.maxBpm = 0;
  currentStats.minBpm = 200;
  currentStats.avgBpm = 0;
  currentStats.hrv = 0;
  currentStats.isAnomaly = false;
  currentStats.anomalyType = 0;
}

void recordBeatEvent(int bpm) {
  if (bpm <= 0 || bpm > 200) return;

  // Lưu vào lịch sử
  bpmHistory[historyIndex] = bpm;
  historyIndex = (historyIndex + 1) % STATS_HISTORY_SIZE;
  if (historyCount < STATS_HISTORY_SIZE) {
    historyCount++;
  }

  // Cập nhật Max/Min
  if (bpm > currentStats.maxBpm) {
    currentStats.maxBpm = bpm;
  }
  if (bpm < currentStats.minBpm) {
    currentStats.minBpm = bpm;
  }

  // Tính trung bình
  int sum = 0;
  for (int i = 0; i < historyCount; i++) {
    sum += bpmHistory[i];
  }
  currentStats.avgBpm = (float)sum / historyCount;

  // Tính HRV
  currentStats.hrv = calculateHRV();

  // Phát hiện bất thường
  currentStats.isAnomaly = detectAnomaly(bpm, currentStats.avgBpm);
}

float calculateHRV() {
  if (historyCount < 5) return 0;
    
  // Tính độ lệch chuẩn (Standard Deviation) của BPM
  float mean = currentStats.avgBpm;
  float sumSquaredDiff = 0;

  for (int i = 0; i < historyCount; i++) {
    float diff = bpmHistory[i] - mean;
    sumSquaredDiff += diff * diff;
  }

  float variance = sumSquaredDiff / historyCount;
  float stdDev = sqrt(variance);

  // HRV = độ lệch chuẩn (đơn vị: ms giữa các beat)
  // Chuyển đổi từ BPM sang ms
  // 60000 ms / BPM = ms giữa các beat
  // HRV được chuẩn hóa từ 0-100
  float hrv = (stdDev / mean) * 100;  // CV (Coefficient of Variation)

  return hrv > 100 ? 100 : hrv;
}

bool detectAnomaly(int bpm, float avgBpm) {
  // Kiểm tra tachycardia (tim đập quá nhanh)
  if (bpm > TACHYCARDIA_THRESHOLD) {
    currentStats.anomalyType = 1;
    return true;
  }

  // Kiểm tra bradycardia (tim đập quá chậm)
  if (bpm < BRADYCARDIA_THRESHOLD) {
    currentStats.anomalyType = 2;
    return true;
  }

  // Kiểm tra sự không đều (irregular pattern)
  if (historyCount >= 4) {
    int diff1 = abs(bpmHistory[(historyIndex - 1 + STATS_HISTORY_SIZE) % STATS_HISTORY_SIZE] - 
                    bpmHistory[(historyIndex - 2 + STATS_HISTORY_SIZE) % STATS_HISTORY_SIZE]);
    int diff2 = abs(bpmHistory[(historyIndex - 2 + STATS_HISTORY_SIZE) % STATS_HISTORY_SIZE] - 
                    bpmHistory[(historyIndex - 3 + STATS_HISTORY_SIZE) % STATS_HISTORY_SIZE]);
    
    if (diff1 > 20 && diff2 > 20) {
      currentStats.anomalyType = 3;
      return true;
    }
  }

  currentStats.anomalyType = 0;
  return false;
}

HeartRateStats* getStats() {
  return &currentStats;
}

void resetStats() {
  initStats();
}

int getHeartRateLevel(int bpm) {
  // Phân loại mức nhịp tim
  if (bpm < BRADYCARDIA_THRESHOLD) {
    return 1;  // Quá chậm
  } else if (bpm <= NORMAL_HEART_RATE_MAX) {
    return 2;  // Bình thường
  } else if (bpm <= TACHYCARDIA_THRESHOLD) {
    return 3;  // Nhanh
  } else {
    return 4;  // Quá nhanh
  }
}

HeartRateStats* getStatsPtr() {
    return &currentStats;
}
void updateStatistics(int bpm)
{
    if (bpm <= 0) return;

    bpmHistory[historyIndex] = bpm;
    historyIndex = (historyIndex + 1) % STATS_HISTORY_SIZE;

    if (historyCount < STATS_HISTORY_SIZE)
        historyCount++;

    int maxVal = bpmHistory[0];
    int minVal = bpmHistory[0];
    int sum = 0;

    for (int i = 0; i < historyCount; i++)
    {
        if (bpmHistory[i] > maxVal)
            maxVal = bpmHistory[i];

        if (bpmHistory[i] < minVal)
            minVal = bpmHistory[i];

        sum += bpmHistory[i];
    }

    currentStats.maxBpm = maxVal;
    currentStats.minBpm = minVal;
    currentStats.avgBpm = (float)sum / historyCount;
    currentStats.hrv = calculateHRV();
}