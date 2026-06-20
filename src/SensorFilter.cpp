#include "SensorFilter.h"
#include "SensorStats.h"
#include "heartRate.h"
#include "MAX30105.h"
#include <Arduino.h>

#define BPM_BUFFER_SIZE 8
#define BUFFER_SIZE 50

static MAX30105 sensor;
static long lastBeat = 0;
static float bpmAvg = 0;
static float bpmSmooth = 0;
static float bpmBuffer[BPM_BUFFER_SIZE];
static int bpmIndex = 0;

static long irBuffer[BUFFER_SIZE];
static long redBuffer[BUFFER_SIZE];
static int bufferIndex = 0;
static float spo2 = 0;

static bool fingerPresent = false;

static float estimateBPM(float spo2Value) {
  if (spo2Value > 100) spo2Value = 100;
  if (spo2Value < 90) spo2Value = 90;
  return 65 + (100 - spo2Value) * 4.5;
}

bool initSensor() {
  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    return false;
  }

  sensor.setup(0x3F, 4, 2, 100, 411, 4096);
  sensor.setPulseAmplitudeIR(0x3F);
  sensor.setPulseAmplitudeRed(0x3F);
  sensor.setPulseAmplitudeGreen(0);

  for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
    bpmBuffer[i] = 0;
  }

  initStats();

  return true;
}

void updateSensor() {
  long ir = sensor.getIR();
  long red = sensor.getRed();

  if (ir < 30000) {
    fingerPresent = false;
    bpmAvg = 0;
    bpmSmooth = 0;
    spo2 = 0;
    bpmIndex = 0;
    bufferIndex = 0;

    for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
      bpmBuffer[i] = 0;
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
      irBuffer[i] = 0;
      redBuffer[i] = 0;
    }

    return;
  }

  fingerPresent = true;

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
        
        // Ghi nhận beat vào stats
        recordBeatEvent((int)bpm);
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
}

int getCurrentBpm() {
  return fingerPresent ? (int)bpmSmooth : 0;
}

int getCurrentSpo2() {
  return fingerPresent ? (int)spo2 : 0;
}

bool isFingerPresent() {
  return fingerPresent;
}

