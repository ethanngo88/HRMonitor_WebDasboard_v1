
#include "EmbeddedWebAssets.h"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Healthcare IoT Dashboard</title>

    <link rel="stylesheet" href="/style.css">

    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>

    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap"
        rel="stylesheet">

    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>

<body>

<div class="dashboard">

    <!-- Sidebar -->
    <aside class="sidebar">

        <div class="logo">
            <div class="logo-icon">❤</div>
            <div>
                <h2>Health IoT</h2>
                <p>Realtime Monitor</p>
            </div>
        </div>

        <ul class="menu">

            <li class="active" onclick="showPage('dashboard')">
                Dashboard
            </li>

            <li onclick="showPage('analytics')">
                Analytics
            </li>

            <li>Devices</li>
            <li>Reports</li>
            <li>Settings</li>

        </ul>

    </aside>

    <!-- Main Content -->
    <main class="main-content">

        <!-- ================= DASHBOARD ================= -->

        <div id="dashboard-page">

            <header class="topbar">

                <div>
                    <h1>Healthcare IoT Dashboard</h1>
                    <p>ESP32 + MAX30102 + WebSocket Realtime</p>
                </div>

                <div class="connection-box">
                    <span class="dot" id="dot"></span>
                    <span id="status">Disconnected</span>
                </div>

            </header>

            <section class="cards">

                <div class="card red-card">

                    <div class="card-top">

                        <div>
                            <p class="card-title">Heart Rate</p>
                            <h2 class="card-value" id="heartRate">0</h2>
                            <span class="card-unit">BPM</span>
                        </div>

                        <div class="card-icon red-icon">
                            ❤
                        </div>

                    </div>

                    <div class="card-footer" id="hrStatus">
                        Live monitoring active
                    </div>

                </div>

                <div class="card green-card">

                    <div class="card-top">

                        <div>
                            <p class="card-title">Blood Oxygen</p>
                            <h2 class="card-value" id="spo2">0</h2>
                            <span class="card-unit">%</span>
                        </div>

                        <div class="card-icon green-icon">
                            🫁
                        </div>

                    </div>

                    <div class="card-footer">
                        Oxygen saturation tracking
                    </div>

                </div>

                <div class="card blue-card">

                    <div class="card-top">

                        <div>
                            <p class="card-title">Device Status</p>

                            <div class="device-list">
                                <p>ESP32 Connected</p>
                                <p>Sensor Active</p>
                                <p>WebSocket Running</p>
                            </div>
                        </div>

                        <div class="card-icon blue-icon">
                            📡
                        </div>

                    </div>

                    <div class="card-footer">
                        Edge IoT Device Online
                    </div>

                </div>

            </section>

            <section class="chart-section">

                <div class="chart-header">
                    <h2>Realtime Heart Monitoring</h2>
                </div>

                <canvas id="healthChart"></canvas>

            </section>

        </div>

        <!-- ================= ANALYTICS ================= -->

        <div id="analytics-page" style="display:none;">

            <header class="topbar">

                <div>
                    <h1>Analytics Dashboard</h1>
                    <p>Heart Rate Analysis & Statistics</p>
                </div>

            </header>

            <section class="stats-section">

                <h2>Heart Rate Analytics</h2>

                <div class="stats-grid">

                    <div class="stat-card">
                        <div class="stat-label">MAX BPM</div>
                        <div class="stat-value" id="maxBpm">--</div>
                    </div>

                    <div class="stat-card">
                        <div class="stat-label">MIN BPM</div>
                        <div class="stat-value" id="minBpm">--</div>
                    </div>

                    <div class="stat-card">
                        <div class="stat-label">AVG BPM</div>
                        <div class="stat-value" id="avgBpm">--</div>
                    </div>

                    <div class="stat-card">
                        <div class="stat-label">HRV</div>
                        <div class="stat-value" id="hrv">--</div>
                    </div>

                    <div class="stat-card anomaly">
                        <div class="stat-label">STATUS</div>
                        <div class="stat-value" id="anomalyStatus">Normal</div>
                    </div>

                    <div class="stat-card">
                        <div class="stat-label">HR LEVEL</div>
                        <div class="stat-value" id="hrLevel">--</div>
                    </div>

                </div>

            </section>

            <section class="chart-section">

                <div class="chart-header">
                    <h2>Analytics History</h2>
                </div>

                <canvas id="analyticsChart"></canvas>

            </section>

        </div>

    </main>

</div>

<script>

function showPage(page){

    document.getElementById("dashboard-page").style.display = "none";
    document.getElementById("analytics-page").style.display = "none";

    document.getElementById(page + "-page").style.display = "block";

    document.querySelectorAll(".menu li").forEach(item => {
        item.classList.remove("active");
    });

    if(page === "dashboard"){
        document.querySelectorAll(".menu li")[0].classList.add("active");
    }

    if(page === "analytics"){
        document.querySelectorAll(".menu li")[1].classList.add("active");
    }
}

</script>

<script src="/app.js"></script>

</body>
</html>
)rawliteral";

const char app_js[] PROGMEM = R"rawliteral(// =========================
// ELEMENTS
// =========================
const heartRateElement = document.getElementById("heartRate");
const spo2Element = document.getElementById("spo2");
const statusElement = document.getElementById("status");
const dotElement = document.getElementById("dot");

// =========================
// CHART DATA
// =========================
const labels = [];
const heartRateData = [];
const spo2Data = [];

// =========================
// CHART CONFIG
// =========================
const ctx = document.getElementById("healthChart").getContext("2d");
const healthChart = new Chart(ctx, {
    type: "line",
    data: {
        labels: labels,
        datasets: [
            {
                label: "Heart Rate",
                data: heartRateData,
                borderColor: "#ef4444",
                backgroundColor: "rgba(239,68,68,0.2)",
                borderWidth: 3,
                fill: true,
                tension: 0.4
            },
            {
                label: "SpO2",
                data: spo2Data,
                borderColor: "#22c55e",
                backgroundColor: "rgba(34,197,94,0.2)",
                borderWidth: 3,
                fill: true,
                tension: 0.4
            }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        interaction: {
            intersect: false,
            mode: "index"
        },
        plugins: {
            legend: {
                labels: {
                    color: "white",
                    font: { size: 14 }
                }
            }
        },
        scales: {
            x: {
                ticks: { color: "#cbd5e1" },
                grid: { color: "rgba(255,255,255,0.05)" }
            },
            y: {
                beginAtZero: true,
                ticks: { color: "#cbd5e1" },
                grid: { color: "rgba(255,255,255,0.05)" }
            }
        }
    }
});

// =========================
// WEBSOCKET
// =========================
const gateway = `ws://${window.location.hostname}/ws`;
let websocket;

function initWebSocket() {
    console.log("Connecting WebSocket...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen() {
    console.log("Connected");
    statusElement.innerHTML = "Connected";
    dotElement.style.background = "#22c55e";
}

function onClose() {
    console.log("Disconnected");
    statusElement.innerHTML = "Disconnected";
    dotElement.style.background = "#ef4444";
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    const data = JSON.parse(event.data);
    const heartRate = data.heartRate;
    const spo2 = data.spo2;
    
    heartRateElement.innerHTML = heartRate;
    spo2Element.innerHTML = spo2;
    
    // Cập nhật thống kê
    if (data.stats) {
        const stats = data.stats;
        document.getElementById("maxBpm").innerHTML =
            stats.maxBpm;

        document.getElementById("minBpm").innerHTML =
            stats.minBpm == 200 ? "--" : stats.minBpm;

        document.getElementById("avgBpm").innerHTML =
            stats.avgBpm.toFixed(1);

        document.getElementById("hrv").innerHTML =
            stats.hrv.toFixed(1);
        
        // Cập nhật trạng thái bất thường
        const anomalyStatus = document.getElementById("anomalyStatus");
        const statusElement = document.getElementById("hrStatus");
        
        if (data.isAnomaly) {
            anomalyStatus.innerHTML = getAnomalyLabel(data.anomalyType);
            anomalyStatus.parentElement.classList.add("anomaly");
            statusElement.innerHTML = "⚠️ " + getAnomalyMessage(data.anomalyType);
            statusElement.style.color = "#ef4444";
        } else {
            anomalyStatus.innerHTML = "Normal";
            anomalyStatus.parentElement.classList.remove("anomaly");
            statusElement.innerHTML = "✓ Normal";
            statusElement.style.color = "#22c55e";
        }
        
        // Cập nhật mức nhịp tim
        const hrLevelElement = document.getElementById("hrLevel");
        hrLevelElement.innerHTML = getHeartRateLevel(data.heartRate);
    }
    
    const time = new Date().toLocaleTimeString();
    labels.push(time);
    heartRateData.push(heartRate);
    spo2Data.push(spo2);
    if (labels.length > 20) {
        labels.shift();
        heartRateData.shift();
        spo2Data.shift();
    }
    healthChart.update();
}

function getAnomalyLabel(type) {
    switch(type) {
        case 1: return "Tachycardia";
        case 2: return "Bradycardia";
        case 3: return "Irregular";
        default: return "Normal";
    }
}

function getAnomalyMessage(type) {
    switch(type) {
        case 1: return "Heart rate too fast (>100 BPM)";
        case 2: return "Heart rate too slow (<60 BPM)";
        case 3: return "Irregular heartbeat detected";
        default: return "Normal monitoring";
    }
}

function getHeartRateLevel(bpm) {
    if (bpm < 60) return "Slow 🔵";
    if (bpm <= 100) return "Normal 🟢";
    if (bpm >= 100) return "Fast 🟡";
    return "Very Fast 🔴";
}

window.addEventListener("load", initWebSocket);
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(

/* =========================
   RESET
========================= */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Poppins', sans-serif;
    background: linear-gradient(135deg, #0f172a, #111827, #1e293b);
    color: white;
    min-height: 100vh;
}

/* =========================
   LAYOUT
========================= */
.dashboard {
    display: flex;
    min-height: 100vh;
}

.main-content {
    flex: 1;
    padding: 30px;
    overflow-y: auto;
    max-height: 100vh;
}

/* =========================
   SIDEBAR
========================= */
.sidebar {
    width: 260px;
    padding: 30px 20px;
    background: rgba(255,255,255,0.05);
    backdrop-filter: blur(15px);
    border-right: 1px solid rgba(255,255,255,0.08);
}

.logo {
    display: flex;
    align-items: center;
    gap: 15px;
    margin-bottom: 50px;
}

.logo-icon {
    width: 55px;
    height: 55px;
    border-radius: 15px;
    background: linear-gradient(135deg, #ef4444, #dc2626);

    display: flex;
    justify-content: center;
    align-items: center;

    font-size: 24px;
}

.logo h2 {
    font-size: 28px;
}

.logo p {
    color: #94a3b8;
    font-size: 14px;
}

/* =========================
   MENU
========================= */
.menu {
    list-style: none;
}

.menu li {
    padding: 16px 18px;
    margin-bottom: 12px;
    border-radius: 14px;
    cursor: pointer;
    transition: 0.3s;
}

.menu li:hover,
.menu .active {
    background: rgba(255,255,255,0.08);
}

/* =========================
   TOPBAR
========================= */
.topbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 35px;
}

.topbar h1 {
    font-size: 38px;
    margin-bottom: 8px;
}

.topbar p {
    color: #94a3b8;
}

/* =========================
   CONNECTION STATUS
========================= */
.connection-box {
    display: flex;
    align-items: center;
    gap: 12px;

    padding: 14px 20px;
    border-radius: 14px;

    background: rgba(255,255,255,0.06);
    border: 1px solid rgba(255,255,255,0.08);
}

.dot {
    width: 12px;
    height: 12px;
    border-radius: 50%;

    background: red;
    animation: pulse 1.5s infinite;
}

@keyframes pulse {
    0%   { transform: scale(1); }
    50%  { transform: scale(1.4); }
    100% { transform: scale(1); }
}

/* =========================
   CARDS
========================= */
.cards {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 25px;
    margin-bottom: 30px;
}

.card {
    position: relative;
    overflow: hidden;

    padding: 28px;
    border-radius: 28px;

    background: rgba(255,255,255,0.07);
    backdrop-filter: blur(15px);
    border: 1px solid rgba(255,255,255,0.08);

    transition: 0.4s;
}

.card:hover {
    transform: translateY(-8px);
    box-shadow: 0 10px 35px rgba(0,0,0,0.3);
}

.card::before {
    content: "";
    position: absolute;

    width: 200px;
    height: 200px;

    top: -100px;
    right: -100px;

    border-radius: 50%;
    opacity: 0.1;
}

.red-card::before {
    background: #ef4444;
}

.green-card::before {
    background: #22c55e;
}

.blue-card::before {
    background: #38bdf8;
}

.card-top {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
}

.card-title {
    color: #cbd5e1;
    margin-bottom: 10px;
}

.card-value {
    font-size: 72px;
    font-weight: 700;
    margin-bottom: 5px;
}

.card-unit {
    color: #94a3b8;
}

.card-icon {
    width: 70px;
    height: 70px;
    border-radius: 20px;

    display: flex;
    justify-content: center;
    align-items: center;

    font-size: 32px;
}

.red-icon {
    background: rgba(239,68,68,0.15);
}

.green-icon {
    background: rgba(34,197,94,0.15);
}

.blue-icon {
    background: rgba(56,189,248,0.15);
}

.card-footer {
    margin-top: 25px;
    padding-top: 18px;

    border-top: 1px solid rgba(255,255,255,0.08);
    color: #94a3b8;
}

.device-list p {
    margin-bottom: 10px;
}

/* =========================
   STATS SECTION
========================= */
.stats-section {
    margin-bottom: 30px;
    padding: 30px;
    border-radius: 28px;

    background: rgba(255,255,255,0.07);
    backdrop-filter: blur(15px);
    border: 1px solid rgba(255,255,255,0.08);
}

.stats-section h2 {
    font-size: 24px;
    margin-bottom: 25px;
    color: #fff;
}

.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(150px,1fr));
    gap: 15px;
}

.stat-card {
    padding: 20px;
    text-align: center;
    border-radius: 16px;

    background: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.08);

    transition: 0.3s;
}

.stat-card:hover {
    background: rgba(255,255,255,0.08);
    transform: translateY(-4px);
}

.stat-label {
    margin-bottom: 10px;

    color: #cbd5e1;
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.stat-value {
    font-size: 28px;
    font-weight: 700;
    color: #22c55e;
}

.stat-card.anomaly {
    border-color: rgba(239,68,68,0.3);
    background: rgba(239,68,68,0.05);
}

.stat-card.anomaly .stat-value {
    color: #ef4444;
}

/* =========================
   CHART SECTION
========================= */
.chart-section {
    padding: 30px;
    border-radius: 28px;

    background: rgba(255,255,255,0.07);
    backdrop-filter: blur(15px);
    border: 1px solid rgba(255,255,255,0.08);
}

.chart-header {
    margin-bottom: 25px;
}

.chart-header h2 {
    font-size: 28px;
}

canvas {
    width: 100% !important;
    height: 450px !important;
}

/* =========================
   RESPONSIVE
========================= */
.analytics-menu{
    background:rgba(255,255,255,0.05);
    border-radius:16px;
    padding:15px;
    margin-top:20px;
}

.analytics-menu h4{
    color:#fff;
    margin-bottom:15px;
    font-size:16px;
}

.analytics-item{
    display:flex;
    justify-content:space-between;
    align-items:center;
    padding:8px 0;
    border-bottom:1px solid rgba(255,255,255,0.08);
    font-size:14px;
}

.analytics-item:last-child{
    border-bottom:none;
}

.analytics-item span:last-child{
    font-weight:600;
    color:#22c55e;
}

@media (max-width: 1000px) {
    .dashboard {
        flex-direction: column;
    }

    .sidebar {
        width: 100%;
    }
}

@media (max-width: 700px) {
    .topbar {
        flex-direction: column;
        align-items: flex-start;
        gap: 20px;
    }

    .card-value {
        font-size: 52px;
    }

    .topbar h1 {
        font-size: 28px;
    }

    .stats-grid {
        grid-template-columns: repeat(2, 1fr);
    }
}

)rawliteral";
