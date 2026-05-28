# ❤️ HRMonitor - ESP32 Heart Rate Monitoring System

A real-time heart rate monitoring system using **ESP32** and **MAX30102 sensor**, with live data visualization on a web dashboard via **WebSocket communication**.

---

## 🚀 Features

- 📡 Real-time heart rate (BPM) measurement using MAX30102
- 🔌 ESP32-based IoT edge device
- 🌐 Live web dashboard (no refresh required)
- ⚡ WebSocket for low-latency data streaming
- 📊 Real-time UI updates (charts / numeric display)
- 📱 Accessible via browser on local WiFi network
- 🔧 Modular architecture for easy extension (SpO2, temperature, etc.)

---

## 🧠 System Architecture

- Sensor collects PPG signal
- ESP32 processes signal → calculates BPM
- ESP32 pushes data via WebSocket
- Frontend updates UI in real-time

---

## 🔧 Hardware Requirements

- ESP32 Dev Board
- MAX30102 Pulse Oximeter Sensor
- Jumper wires
- Breadboard (optional)
- USB cable for power + debugging

---

## 🔌 Wiring (I2C)

| MAX30102 | ESP32 |
|----------|------|
| VIN      | 3.3V |
| GND      | GND  |
| SDA      | GPIO 21 |
| SCL      | GPIO 22 |

---

## 💻 Software Stack

- Arduino Framework / PlatformIO
- ESP32 WiFi library
- WebSocket library (ESPAsyncWebServer or equivalent)
- MAX30102 sensor library
- HTML/CSS/JavaScript frontend

---

## ⚙️ How It Works

1. ESP32 initializes WiFi connection
2. MAX30102 starts collecting IR/red LED signals
3. Algorithm computes BPM from PPG signal
4. Data is broadcast via WebSocket
5. Web dashboard receives and updates UI instantly

---

## 📊 Web Dashboard Features

- Live BPM display
- Real-time chart updates
- Connection status indicator
- Responsive UI for mobile + desktop

---

## 📦 Installation

```bash
git clone https://github.com/your-username/HRMonitor.git
cd HRMonitor
