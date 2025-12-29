#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>

#define DHTPIN D2
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "your_ssid";
const char* pass = "your_secret_password";

ESP8266WebServer server(80); // Porta standard 80 per semplicità
WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket sulla porta 81

unsigned long lastMsg = 0;

// --- PAGINA HTML ---
void getHome() {
  String html = R"=====(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
    <meta charset='utf-8'>
    <title>Monitor Real-Time DHT21</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      /* Variabili di colore per i temi */
      :root {
        --bg-body: #f0f2f5;
        --bg-card: #ffffff;
        --text-main: #333333;
        --text-label: #7f8c8d;
        --grid-color: #f0f0f0;
      }

      [data-theme='dark'] {
        --bg-body: #1a1a1a;
        --bg-card: #2d2d2d;
        --text-main: #e0e0e0;
        --text-label: #aaaaaa;
        --grid-color: #404040;
      }

      body { font-family: 'Segoe UI', sans-serif; background: var(--bg-body); color: var(--text-main); text-align: center; padding: 20px; margin: 0; transition: all 0.3s ease; }
      h1 { font-weight: 300; margin-top: 40px; }
      
      /* Pulsante Tema */
      .theme-toggle {
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 10px 15px;
        border-radius: 30px;
        border: none;
        cursor: pointer;
        background: var(--bg-card);
        color: var(--text-main);
        box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        font-weight: bold;
        z-index: 1000;
      }

      .card { background: var(--bg-card); border-radius: 15px; padding: 20px; margin: 10px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); flex: 1; min-width: 350px; transition: background 0.3s ease; }
      .dashboard { display: flex; flex-wrap: wrap; justify-content: center; max-width: 1200px; margin: auto; }
      #stat { font-size: 0.5em; vertical-align: middle; padding: 4px 10px; border-radius: 20px; background: #e74c3c; color: white; }
      .online { background: #27ae60 !important; }
      .temp-val { color: #e74c3c; font-size: 2.5em; font-weight: bold; }
      .hum-val { color: #3498db; font-size: 2.5em; font-weight: bold; }
      .label { display: block; color: var(--text-label); text-transform: uppercase; font-size: 0.8em; letter-spacing: 1px; margin-bottom: 10px; }
    </style>
  </head>
  <body>
    <button class="theme-toggle" onclick="toggleTheme()" id="themeBtn">🌙 Dark Mode</button>
    
    <h1>DHT21 Monitor <span id="stat">OFFLINE</span></h1>
    <div class="dashboard">
      <div class="card">
        <span class="label">Temperatura Attuale</span>
        <span class="temp-val"><span id='t'>--</span>°C</span>
        <canvas id="chartT"></canvas>
      </div>
      <div class="card">
        <span class="label">Umidità Attuale</span>
        <span class="hum-val"><span id='h'>--</span>%</span>
        <canvas id="chartH"></canvas>
      </div>
    </div>

    <script>
      let socket;
      
      // Funzione per cambiare tema
      function toggleTheme() {
        const body = document.documentElement;
        const btn = document.getElementById('themeBtn');
        const isDark = body.getAttribute('data-theme') === 'dark';
        
        const newTheme = isDark ? 'light' : 'dark';
        body.setAttribute('data-theme', newTheme);
        btn.innerText = isDark ? '🌙 Dark Mode' : '☀️ Light Mode';
        
        updateChartColors(newTheme);
      }

      // Funzione per aggiornare i colori dei grafici dinamicamente
      function updateChartColors(theme) {
        const textColor = theme === 'dark' ? '#e0e0e0' : '#333333';
        const gridColor = theme === 'dark' ? '#404040' : '#f0f0f0';

        [cT, cH].forEach(chart => {
          chart.options.scales.y.ticks.color = textColor;
          chart.options.scales.y.title.color = textColor;
          chart.options.scales.y.grid.color = gridColor;
          chart.options.scales.x.ticks.color = textColor;
          chart.options.scales.x.title.color = textColor;
          chart.update();
        });
      }

      const chartConf = (col, labelY, unit, yMin, yMax) => ({
        type: 'line',
        data: { labels: [], datasets: [{ borderColor: col, data: [], tension: 0.4, fill: true, backgroundColor: col+'22', pointRadius: 3 }] },
        options: { 
          responsive: true, 
          scales: {
            y: {
              min: yMin, max: yMax,
              title: { display: true, text: labelY + ' (' + unit + ')', font: { weight: 'bold' } },
              grid: { color: '#f0f0f0' }
            },
            x: { 
              title: { display: true, text: 'Orario (hh:mm:ss)', font: { weight: 'bold' } },
              grid: { display: false } 
            }
          },
          plugins: { legend: { display: false } } 
        }
      });

      const cT = new Chart(document.getElementById('chartT'), chartConf('#e74c3c', 'Temperatura', '°C', -40, 80));
      const cH = new Chart(document.getElementById('chartH'), chartConf('#3498db', 'Umidità', '%', 0, 100));

      function initWebSocket() {
        socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        socket.onopen = () => { 
          const s = document.getElementById('stat');
          s.innerText = "LIVE"; s.classList.add('online');
        };
        socket.onmessage = (event) => {
          const data = JSON.parse(event.data);
          const now = new Date().toLocaleTimeString('it-IT', { hour12: false });
          document.getElementById('t').innerText = data.T.toFixed(1);
          document.getElementById('h').innerText = data.H.toFixed(1);
          cT.data.labels.push(now);
          cT.data.datasets[0].data.push(data.T);
          cH.data.labels.push(now);
          cH.data.datasets[0].data.push(data.H);
          if(cT.data.labels.length > 20) {
            [cT, cH].forEach(chart => {
              chart.data.labels.shift();
              chart.data.datasets[0].data.shift();
            });
          }
          cT.update('none');
          cH.update('none');
        };
        socket.onclose = () => { setTimeout(initWebSocket, 2000); };
      }
      window.onload = initWebSocket;
    </script>
  </body>
  </html>
  )=====";
  server.send(200, "text/html", html);
}

// --- SERVER ---
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_CONNECTED) {
    Serial.printf("[%u] Client connesso!\n", num);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  if (MDNS.begin("wemosdht")) Serial.println("mDNS avviato");

  server.on("/", getHome);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  MDNS.update();
  server.handleClient();
  webSocket.loop();

  // Invia dati ogni 2 secondi (limite fisico DHT21)
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      // Crea stringa JSON: {"T":22.5, "H":50.2} da stampare sul monitor seriale
      String json = "{\"T\":" + String(t) + ",\"H\":" + String(h) + "}";
      
      // Push del dato a tutti i browser connessi simultaneamente
      webSocket.broadcastTXT(json);
      Serial.println("Broadcast: " + json);
    }
  }
}
