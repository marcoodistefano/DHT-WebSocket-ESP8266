# 🌡️ ESP8266 Real-Time DHT21 Monitor

Questo progetto trasforma un modulo **ESP8266** (come Wemos D1 Mini) in un server di monitoraggio ambientale in tempo reale. Utilizza un sensore **DHT21 (AM2301)** per rilevare temperatura e umidità e trasmette i dati istantaneamente ai browser connessi tramite protocollo **WebSockets**.

## 🚀 Caratteristiche principali

* **Aggiornamento Real-Time**: Grazie ai WebSocket, i dati vengono inviati ("push") dal server al client istantaneamente, senza necessità di ricaricare la pagina.
* **Grafici Dinamici**: Visualizzazione dell'andamento storico (ultimi 20 campioni) tramite la libreria **Chart.js**.
* **Interfaccia Responsive**: Dashboard moderna e pulita, ottimizzata sia per desktop che per dispositivi mobile.
* **Dual Mode (Light/Dark)**: Pulsante dedicato per cambiare il tema (chiaro/scuro) con adattamento automatico dei colori del grafico.
* **mDNS Support**: Accessibile facilmente tramite l'indirizzo [http://wemosdht.local](http://wemosdht.local).

## 🛠️ Hardware Necessario

| Componente | Descrizione |
| :--- | :--- |
| **ESP8266** | Scheda di sviluppo (NodeMCU, Wemos D1 Mini, ecc.) |
| **DHT21 (AM2301)** | Sensore di temperatura e umidità ad alta precisione |
| **Cavi Jumper** | Per i collegamenti |

## 🔌 Schema di Collegamento

| DHT21 Pin | ESP8266 Pin (Wemos/NodeMCU) |
| :--- | :--- |
| **VCC** | 3.3V o 5V (in base al modello) |
| **DATA** | D2 (GPIO 4) |
| **GND** | GND |

<img width="588" height="477" alt="image" src="https://github.com/user-attachments/assets/fc08e33b-730e-4d28-802b-5431d5ef69fb" />


## 💻 Requisiti Software (Arduino IDE)

Assicurati di aver installato le seguenti librerie tramite il Gestore Librerie:

1.  **DHT sensor library** (Adafruit)
2.  **WebSockets** (di Markus Sattler)
3.  **ESP8266WebServer** (integrata nel pacchetto core ESP8266)

## ⚙️ Configurazione

Prima di caricare lo sketch, apri il file `.ino` e inserisci le tue credenziali Wi-Fi:

```cpp
const char* ssid = "your_ssid";
const char* pass = "your_secret_password";
