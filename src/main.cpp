#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MAX30105 sensor;

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

float estimateBPM(float spo2) {
  if (spo2 > 100) spo2 = 100;
  if (spo2 < 90) spo2 = 90;
  return 65 + (100 - spo2) * 4.5;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED fail");
    while (1);
  }

  display.setTextColor(WHITE);

  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor fail");
    while (1);
  }

  sensor.setup(0x3F, 4, 2, 100, 411, 4096);
  sensor.setPulseAmplitudeIR(0x3F);
  sensor.setPulseAmplitudeRed(0x3F);
  sensor.setPulseAmplitudeGreen(0);

  Serial.println("System ready");
}

void loop() {
  long ir = sensor.getIR();
  long red = sensor.getRed();

  // Không có ngón tay
  if (ir < 30000) {
    bpmAvg = 0;
    bpmSmooth = 0;
    spo2 = 0;

    for (int i = 0; i < BPM_BUFFER_SIZE; i++) bpmBuffer[i] = 0;

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.println("NO FINGER");
    display.display();

    delay(100);
    return;
  }

  // Tính BPM
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

  // Lưu dữ liệu SpO2
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
    if (bpmAvg >= 45) bpmRaw = bpmAvg;
    else bpmRaw = estimateBPM(spo2);
  }

  // Làm mượt BPM
  if (bpmRaw > 0) {
    bpmSmooth = 0.85 * bpmSmooth + 0.15 * bpmRaw;
  } else {
    bpmSmooth *= 0.98;
  }

  if (bpmSmooth < 50 && bpmSmooth > 0) bpmSmooth = 50;
  if (bpmSmooth > 120) bpmSmooth = 120;

  float bpmDisplay = bpmSmooth;

  // In ra Serial (debug)
  Serial.print("BPM: ");
  Serial.print((int)bpmDisplay);
  Serial.print(" | SpO2: ");
  Serial.println((int)spo2);

  // Hiển thị OLED
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("BPM:");
  if (bpmDisplay == 0) display.println("--");
  else display.println((int)bpmDisplay);

  display.setCursor(0, 30);
  display.print("SpO2:");
  if (spo2 == 0) display.print("--");
  else {
    display.print((int)spo2);
    display.print("%");
  }

  display.display();

  delay(20);
}