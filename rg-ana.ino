#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <RF24.h>
#include "LittleFS.h" 
#include "webpages.h"

// Pin Definitions
#define PIN_SCK   14
#define PIN_MISO  12
#define PIN_MOSI  13
#define PIN_CSN   27
#define PIN_CE    26

// --- ADDED: Professional Feature Settings ---
#define RSSI_SAMPLES 10 // Number of samples per channel for pseudo-RSSI reading

// Network credentials
const char* ssid = "RF-Analyser-4.0";
const char* password = "0987654321";

// Global objects
SPIClass hspi(HSPI);
RF24 radio(PIN_CE, PIN_CSN);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Settings structure
struct ScannerSettings {
  int startFreq = 2400, endFreq = 2525, powerLevel = 1, dataRate = 1;
  int scanSpeed = 0; // Default to faster speed
  int sensitivity = 80, averaging = 1;
} settings;

// Data storage
uint16_t channelCounts[125];
float averagedData[125];
uint16_t peakData[125]; // --- ADDED: For Peak Hold mode ---

// Timers and state variables
unsigned long lastDataSend = 0, lastHealthCheck = 0, lastHeapPrint = 0;
bool scanningActive = true, radioOk = false;
bool peakHoldActive = false; // --- ADDED: Peak Hold state ---
int sweepCount = 0; // --- ADDED: For sweep-based averaging ---

// --- Function Prototypes ---
void applyRFSettings();
void checkRadioHealth();
bool initializeRadio();
void scanRfChannels();
void sendSpectrumData();
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void updateSettingsFromClient(JsonObject newSettings);
void sendCurrentSettings(AsyncWebSocketClient *client);
void handleCommand(String command);
void onNotFound(AsyncWebServerRequest *request);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- RF Spectrum Scanner Initializing ---");
  
  if(!LittleFS.begin(true)){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("[OK] LittleFS mounted successfully");

  // Check for required JavaScript files
  const char* plotlyPath = "/plotly-rf-scanner.min.js";
  Serial.printf("[INFO] Checking for %s...\n", plotlyPath);
  if(LittleFS.exists(plotlyPath)){
    Serial.println("[OK] File exists.");
  } else {
    Serial.println("[FAIL] File not found!");
  }

  const char* tonePath = "/tone.js";
  Serial.printf("[INFO] Checking for %s...\n", tonePath);
  if(LittleFS.exists(tonePath)){
    Serial.println("[OK] File exists.");
  } else {
    Serial.println("[FAIL] File not found! Audio features will not work.");
  }
  
  memset(channelCounts, 0, sizeof(channelCounts));
  memset(averagedData, 0, sizeof(averagedData));
  memset(peakData, 0, sizeof(peakData)); // --- ADDED: Initialize peak data ---
  Serial.println("[OK] Arrays initialized");
  
  hspi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CSN);
  Serial.println("[OK] HSPI initialized");
  
  radioOk = initializeRadio();
  if (!radioOk) {
    Serial.println("[WARNING] Radio module not found on startup. You can try to re-initialize it from the web interface.");
  }
  
  Serial.println("Starting WiFi Access Point on Channel 1...");
  // Set WiFi to channel 1
  WiFi.softAP(ssid, password, 1);
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("[OK] WiFi AP started. SSID: %s | IP: %s\n", ssid, IP.toString().c_str());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  
  // --- Server Endpoints ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send_P(200, "text/html", index_html); 
  });
  
  server.on("/plotly-rf-scanner.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/plotly-rf-scanner.min.js", "application/javascript");
  });

  // Endpoint to serve Tone.js from LittleFS
  server.on("/tone.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/tone.js", "application/javascript");
  });
  
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send(204); 
  });

  server.onNotFound(onNotFound);
  
  server.begin();
  Serial.println("[OK] Web server started");
  
  Serial.printf("Initial free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.println("--- Setup Complete ---");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (ws.count() > 0 && currentTime - lastHealthCheck >= 2000) {
    checkRadioHealth();
    lastHealthCheck = currentTime;
  }

  if (radioOk && scanningActive) {
    scanRfChannels();
    if (settings.scanSpeed > 0) delay(settings.scanSpeed);
  } else {
    delay(100);
  }
  
  if (sweepCount >= settings.averaging) {
    sendSpectrumData();
    sweepCount = 0;
  }

  if (currentTime - lastHeapPrint >= 30000) {
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    lastHeapPrint = currentTime;
  }
  
  ws.cleanupClients();
}

void onNotFound(AsyncWebServerRequest *request) {
  Serial.printf("NOT_FOUND: http://%s%s\n", request->host().c_str(), request->url().c_str());
  request->send(404, "text/plain", "Not found");
}

bool initializeRadio() {
  Serial.println("Initializing NRF24L01+PA+LNA...");
  if (!radio.begin(&hspi)) {
    Serial.println("[FAIL] NRF24L01 initialization failed! Check wiring.");
    return false;
  }
  Serial.println("[OK] NRF24L01 initialized successfully");
  applyRFSettings();
  radio.printDetails();
  return true;
}

void checkRadioHealth() {
  if (!radio.isChipConnected()) {
    Serial.println("[ERROR] NRF24L01 connection lost!");
    radioOk = false;
  } else if (!radioOk) {
      radioOk = true;
      Serial.println("[OK] NRF24L01 reconnected.");
  }
}

void applyRFSettings() {
  Serial.println("Applying RF settings...");
  rf24_pa_dbm_e p[] = {RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX};
  rf24_datarate_e d[] = {RF24_250KBPS, RF24_1MBPS, RF24_2MBPS};
  radio.setPALevel(p[settings.powerLevel]);
  radio.setDataRate(d[settings.dataRate]);
  radio.setAutoAck(false);
  radio.disableCRC();
  radio.setPayloadSize(1);
}

void scanRfChannels() {
  uint16_t currentSweep[125] = {0};

  for (int freq = settings.startFreq; freq <= settings.endFreq; freq++) {
    int nrfChannel = freq - 2400;
    if (nrfChannel < 0 || nrfChannel > 124) continue;
    
    radio.setChannel(nrfChannel);
    radio.startListening();
    delayMicroseconds(map(settings.sensitivity, 10, 100, 130, 500));
    
    int hits = 0;
    for (int i = 0; i < RSSI_SAMPLES; i++) {
        if (radio.testRPD()) {
            hits++;
        }
    }
    currentSweep[nrfChannel] = hits;
    radio.stopListening();
    yield();
  }

  for(int i=0; i<125; i++) {
    channelCounts[i] += currentSweep[i];
    if (peakHoldActive && currentSweep[i] > peakData[i]) {
        peakData[i] = currentSweep[i];
    }
  }
  sweepCount++;
}

void sendSpectrumData() {
  if (ws.count() == 0) {
    memset(channelCounts, 0, sizeof(channelCounts));
    sweepCount = 0;
    return;
  }
  
  if (!radioOk) return;

  DynamicJsonDocument doc(4096);
  doc["type"] = "spectrum";
  JsonArray data = doc.createNestedArray("data");
  int totalActivity = 0, peakValue = 0, peakChannel = 0;

  for (int i = 0; i < 125; i++) {
    int value;
    if (peakHoldActive) {
      value = peakData[i];
    } else {
      value = (settings.averaging > 0) ? (channelCounts[i] / settings.averaging) : channelCounts[i];
    }
    
    data.add(value);
    totalActivity += value;
    if (value > peakValue) {
      peakValue = value;
      peakChannel = i;
    }
  }

  JsonObject stats = doc.createNestedObject("statistics");
  stats["total"] = totalActivity;
  stats["peak_freq"] = 2400 + peakChannel;
  stats["peak_value"] = peakValue;
  String message;
  serializeJson(doc, message);
  ws.textAll(message);
  
  memset(channelCounts, 0, sizeof(channelCounts));
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    sendCurrentSettings(client);
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Client #%u disconnected\n", client->id());
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;  
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;
      DynamicJsonDocument doc(1024);
      if (deserializeJson(doc, (char*)data) == DeserializationError::Ok) {
        String msgType = doc["type"];
        if (msgType == "settings") updateSettingsFromClient(doc["settings"]);
        else if (msgType == "command") handleCommand(doc["command"]);
      }
    }
  }
}

void updateSettingsFromClient(JsonObject newSettings) {
  Serial.println("Updating settings from client...");
  bool rfChanged = false;
  if (newSettings.containsKey("startFreq")) { settings.startFreq = constrain(newSettings["startFreq"].as<int>(), 2400, 2524); rfChanged = true; }
  if (newSettings.containsKey("endFreq")) { settings.endFreq = constrain(newSettings["endFreq"].as<int>(), settings.startFreq + 1, 2525); rfChanged = true; }
  if (newSettings.containsKey("powerLevel")) { settings.powerLevel = constrain(newSettings["powerLevel"].as<int>(), 0, 3); rfChanged = true; }
  if (newSettings.containsKey("dataRate")) { settings.dataRate = constrain(newSettings["dataRate"].as<int>(), 0, 2); rfChanged = true; }
  if (newSettings.containsKey("scanSpeed")) settings.scanSpeed = constrain(newSettings["scanSpeed"].as<int>(), 0, 50);
  if (newSettings.containsKey("sensitivity")) settings.sensitivity = constrain(newSettings["sensitivity"].as<int>(), 10, 100);
  if (newSettings.containsKey("averaging")) {
    int avg = newSettings["averaging"].as<int>();
    settings.averaging = constrain(avg, 1, 10);
  }
  
  if (rfChanged) applyRFSettings();
  memset(channelCounts, 0, sizeof(channelCounts));
  sweepCount = 0;
}

void sendCurrentSettings(AsyncWebSocketClient *client) {
  DynamicJsonDocument doc(512);
  doc["type"] = "settings";
  JsonObject s = doc.createNestedObject("settings");
  s["startFreq"] = settings.startFreq; s["endFreq"] = settings.endFreq;
  s["powerLevel"] = settings.powerLevel; s["dataRate"] = settings.dataRate;
  s["scanSpeed"] = settings.scanSpeed; s["sensitivity"] = settings.sensitivity;
  s["averaging"] = settings.averaging;
  String message;
  serializeJson(doc, message);
  client->text(message);
}

void handleCommand(String command) {
  if (command == "pause") { scanningActive = false; Serial.println("Scanning paused"); } 
  else if (command == "resume") { scanningActive = true; Serial.println("Scanning resumed"); } 
  else if (command == "clear") {
    memset(channelCounts, 0, sizeof(channelCounts));
    memset(peakData, 0, sizeof(peakData));
    sweepCount = 0;
    Serial.println("Data cleared");
  } else if (command == "reset") {
    settings = {2400, 2525, 1, 1, 0, 80, 1};
    applyRFSettings();
    DynamicJsonDocument doc(512);
    doc["type"] = "settings";
    JsonObject s = doc.createNestedObject("settings");
    s["startFreq"] = settings.startFreq; s["endFreq"] = settings.endFreq;
    s["powerLevel"] = settings.powerLevel; s["dataRate"] = settings.dataRate;
    s["scanSpeed"] = settings.scanSpeed; s["sensitivity"] = settings.sensitivity;
    s["averaging"] = settings.averaging;
    String message;
    serializeJson(doc, message);
    ws.textAll(message);
    Serial.println("Settings reset to default");
  } else if (command == "reinit") {
    Serial.println("Re-initializing radio from client command...");
    radioOk = initializeRadio();
    
    DynamicJsonDocument doc(64);
    doc["type"] = "reinitStatus";
    doc["status"] = radioOk ? "success" : "fail";
    String response;
    serializeJson(doc, response);
    ws.textAll(response);
  } 
  else if (command == "peak_hold_on") {
    peakHoldActive = true;
    Serial.println("Peak Hold ON");
  } else if (command == "peak_hold_off") {
    peakHoldActive = false;
    Serial.println("Peak Hold OFF");
  } else if (command == "peak_hold_clear") {
    memset(peakData, 0, sizeof(peakData));
    Serial.println("Peak Hold data cleared");
  }
}
