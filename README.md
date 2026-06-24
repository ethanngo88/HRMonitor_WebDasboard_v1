# ❤️ HRMonitor - ESP32 Heart Rate Monitoring System

A real-time heart rate monitoring system built with **ESP32** and **MAX30102** sensor. The system measures heart rate from PPG signals and streams data to a web-based dashboard through **WebSocket communication**, enabling instant monitoring from any device connected to the same Wi-Fi network.

---

## 📌 Overview

HRMonitor is an IoT-based health monitoring solution designed to collect, process, and visualize heart rate data in real time. The ESP32 acts as an edge device that acquires sensor data from the MAX30102, computes heart rate metrics, and broadcasts the results to a responsive web dashboard.

The project demonstrates the integration of embedded systems, wireless communication, signal processing, and modern web technologies in a single monitoring platform.

---

## 🚀 Key Features

* ❤️ Real-time Heart Rate Monitoring (BPM)
* 📡 Wireless Data Transmission via Wi-Fi
* ⚡ Low-Latency Communication using WebSocket
* 📊 Live Dashboard with Instant Updates
* 📱 Responsive Web Interface for Desktop and Mobile
* 🔧 Modular Software Architecture
* 🔄 Easy Integration of Additional Health Metrics
* 🌐 Browser-Based Access without Installing Applications

---

## 🏗️ System Architecture

```text
┌─────────────┐
│  MAX30102   │
│ PPG Sensor  │
└──────┬──────┘
       │ I2C
       ▼
┌─────────────┐
│    ESP32    │
│ Signal Proc │
│ BPM Compute │
└──────┬──────┘
       │ WiFi
       ▼
┌─────────────┐
│ WebSocket   │
│ Communication
└──────┬──────┘
       ▼
┌─────────────┐
│ Web Dashboard│
│ Real-time UI │
└─────────────┘
```

---

## 🧠 Working Principle

1. The MAX30102 sensor captures PPG (Photoplethysmography) signals.
2. ESP32 acquires raw IR data via the I2C interface.
3. Signal filtering and peak detection algorithms identify heartbeats.
4. BPM is calculated based on RR intervals.
5. Processed data is transmitted through WebSocket.
6. The web dashboard receives updates and displays real-time information.

---

## 🔧 Hardware Requirements

| Component                      | Quantity |
| ------------------------------ | -------- |
| ESP32 Development Board        | 1        |
| MAX30102 Pulse Oximeter Sensor | 1        |
| Breadboard                     | Optional |
| Jumper Wires                   | Several  |
| USB Cable                      | 1        |

---

## 🔌 Hardware Connections

### I2C Wiring

| MAX30102 Pin | ESP32 Pin |
| ------------ | --------- |
| VIN          | 3.3V      |
| GND          | GND       |
| SDA          | GPIO 21   |
| SCL          | GPIO 22   |

---

## 💻 Software Stack

### Embedded Layer

* Arduino Framework
* PlatformIO
* ESP32 WiFi Library
* WebSocket Server
* MAX30102 Sensor Driver

### Frontend Layer

* HTML5
* CSS3
* JavaScript
* WebSocket Client API

### Communication Layer

* Wi-Fi (802.11 b/g/n)
* WebSocket Protocol

---

## 📂 Project Structure

```text
HRMonitor/
│
├── include/
│   ├── SensorFilter.h
│   ├── SensorStats.h
│   └── HeartRate.h
│
├── src/
│   ├── main.cpp
│   ├── SensorFilter.cpp
│   ├── SensorStats.cpp
│   └── HeartRate.cpp
│
├── data/
│   ├── index.html
│   ├── style.css
│   └── app.js
│
├── platformio.ini
└── README.md
```

---

## 📊 Dashboard Features

The web dashboard provides:

* Real-time BPM display
* Historical BPM visualization
* Device connection status
* Sensor status monitoring
* Responsive design for mobile and desktop devices

---

## ⚙️ Installation

### Clone Repository

```bash
git clone https://github.com/your-username/HRMonitor.git
cd HRMonitor
```

### Open with PlatformIO

```bash
platformio init
platformio run
```

### Configure Wi-Fi

Edit your Wi-Fi credentials inside the source code:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### Upload Firmware

```bash
platformio run --target upload
```

### Upload Web Files

```bash
platformio run --target uploadfs
```

---

## ▶️ Usage

1. Power the ESP32 board.
2. Place your finger on the MAX30102 sensor.
3. Wait a few seconds for signal stabilization.
4. Open the Serial Monitor to obtain the ESP32 IP address.
5. Enter the IP address in a web browser.
6. Observe live BPM measurements on the dashboard.

---

## 📈 Future Improvements

* Blood Oxygen Saturation (SpO₂)
* Heart Rate Variability (HRV)
* SDNN and RMSSD Metrics
* Data Logging to Database
* Cloud Synchronization
* Mobile Application Integration
* Alert and Notification System

---

## 🎯 Applications

* Personal Health Monitoring
* Smart Healthcare Systems
* IoT Biomedical Projects
* Remote Patient Monitoring
* Educational and Research Projects

---

## 👨‍💻 Authors

Developed as an IoT-based Heart Rate Monitoring System using ESP32 and MAX30102 for real-time physiological data acquisition and visualization.

---

## 📄 License

This project is released under the MIT License.

Feel free to use, modify, and distribute this project for educational and research purposes.
