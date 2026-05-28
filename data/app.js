// =========================
// ELEMENTS
// =========================
/* This JavaScript file manages the real-time data visualization for a healthcare IoT dashboard. 
It establishes a WebSocket connection to receive heart rate and SpO2 data, updates the UI elements with the latest readings, and dynamically updates a line chart to display trends over time. 

The code includes functions for handling WebSocket events such as connection, disconnection, and incoming messages, ensuring a responsive and interactive user experience. */


const heartRateElement =
document.getElementById("heartRate");

const spo2Element =
document.getElementById("spo2");

const statusElement =
document.getElementById("status");

const dotElement =
document.getElementById("dot");

// =========================
// CHART DATA
// =========================

const labels = [];

const heartRateData = [];

const spo2Data = [];

// =========================
// CHART CONFIG
// =========================

const ctx =
document
.getElementById("healthChart")
.getContext("2d");

const healthChart =
new Chart(ctx, {

    type: "line",

    data: {

        labels: labels,

        datasets: [

            {
                label: "Heart Rate",

                data: heartRateData,

                borderColor: "#ef4444",

                backgroundColor:
                "rgba(239,68,68,0.2)",

                borderWidth: 3,

                fill: true,

                tension: 0.4
            },

            {
                label: "SpO2",

                data: spo2Data,

                borderColor: "#22c55e",

                backgroundColor:
                "rgba(34,197,94,0.2)",

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
                    font: {
                        size: 14
                    }
                }
            }

        },

        scales: {

            x: {

                ticks: {
                    color: "#cbd5e1"
                },

                grid: {
                    color:
                    "rgba(255,255,255,0.05)"
                }

            },

            y: {

                beginAtZero: true,

                ticks: {
                    color: "#cbd5e1"
                },

                grid: {
                    color:
                    "rgba(255,255,255,0.05)"
                }

            }

        }

    }

});

// =========================
// WEBSOCKET
// =========================

const gateway =
`ws://${window.location.hostname}/ws`;

let websocket;

// =========================
// INIT SOCKET
// =========================

function initWebSocket(){

    console.log("Connecting WebSocket...");

    websocket =
    new WebSocket(gateway);

    websocket.onopen = onOpen;

    websocket.onclose = onClose;

    websocket.onmessage = onMessage;
}

// =========================
// CONNECTED
// =========================

function onOpen(){

    console.log("Connected");

    statusElement.innerHTML =
    "Connected";

    dotElement.style.background =
    "#22c55e";
}

// =========================
// DISCONNECTED
// =========================

function onClose(){

    console.log("Disconnected");

    statusElement.innerHTML =
    "Disconnected";

    dotElement.style.background =
    "#ef4444";

    // reconnect
    setTimeout(initWebSocket,2000);
}

// =========================
// RECEIVE DATA
// =========================

function onMessage(event){

    const data =
    JSON.parse(event.data);

    const heartRate =
    data.heartRate;

    const spo2 =
    data.spo2;

    // UPDATE UI

    heartRateElement.innerHTML =
    heartRate;

    spo2Element.innerHTML =
    spo2;

    // TIME LABEL

    const time =
    new Date()
    .toLocaleTimeString();

    // PUSH DATA

    labels.push(time);

    heartRateData.push(heartRate);

    spo2Data.push(spo2);

    // LIMIT DATA

    if(labels.length > 20){

        labels.shift();

        heartRateData.shift();

        spo2Data.shift();
    }

    // UPDATE CHART

    healthChart.update();
}

// =========================
// START
// =========================

window.onload = () => {

    initWebSocket();

};