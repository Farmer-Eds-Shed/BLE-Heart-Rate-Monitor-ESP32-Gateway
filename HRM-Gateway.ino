#include <WiFi.h>
#include <WebSocketsClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

// ==== CONFIGURE YOUR SETTINGS HERE ====
#define WIFI_SSID "WiFi"
#define WIFI_PASS "password"
IPAddress WS_HOST(192, 168, 1, 22);
#define WS_PORT 1880
#define WS_PATH "/ws/hrm"
#define HRM_NAME_PREFIX "Fitcent_CL830"

#define UPPER_BPM_THRESHOLD 200
#define SAMPLES_BEFORE_ALERT 5
#define MUTE_DURATION_MS 600000
#define MAX_BUFFERED_EVENTS 50
// ======================================

WebSocketsClient webSocket;
bool wsConnected = false;

BLEScan* pBLEScan;
BLEClient* pClient;
BLERemoteCharacteristic* pHRMChar = nullptr;

bool mute = false;
unsigned long muteStart = 0;
int highCount = 0;
std::vector<uint8_t> bpmBuffer;

void connectWiFi() {
  Serial.printf("Connecting to %s...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");
}

void connectWebSocket() {
  webSocket.begin(WS_HOST, WS_PORT, WS_PATH);
  webSocket.setReconnectInterval(3000);

  webSocket.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        Serial.println("✅ WebSocket connected");
        wsConnected = true;
        break;
      case WStype_DISCONNECTED:
        Serial.println("❌ WebSocket disconnected");
        wsConnected = false;
        break;
      case WStype_TEXT:
        Serial.printf("📩 TEXT msg: %s\n", payload);
        break;
      default:
        break;
    }
  });
}

void sendBPM(uint8_t bpm) {
  if (wsConnected) {
    char msg[16];
    snprintf(msg, sizeof(msg), "%d", bpm);
    webSocket.sendTXT(msg);
  } else if (bpmBuffer.size() < MAX_BUFFERED_EVENTS) {
    bpmBuffer.push_back(bpm);
  }
}

void triggerAlert(uint8_t bpm) {
  Serial.printf("🚨 ALERT: BPM %d > %d for %d samples\n", bpm, UPPER_BPM_THRESHOLD, SAMPLES_BEFORE_ALERT);
  if (wsConnected) webSocket.sendTXT("ALERT");
  mute = true;
  muteStart = millis();
  highCount = 0;
}

void onHRMNotify(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
  if (length < 2) return;
  uint8_t bpm = data[1];  // Assuming UINT8 format
  Serial.printf("❤️ BPM: %d\n", bpm);

  if (mute && millis() - muteStart > MUTE_DURATION_MS) {
    mute = false;
    Serial.println("🔔 Alert unmuted");
  }

  if (!mute && bpm > UPPER_BPM_THRESHOLD) {
    highCount++;
    if (highCount >= SAMPLES_BEFORE_ALERT) {
      triggerAlert(bpm);
    }
  } else {
    highCount = 0;
  }

  sendBPM(bpm);
}

bool connectToDevice(BLEAdvertisedDevice device) {
  Serial.printf("🔗 Connecting to %s...\n", device.getAddress().toString().c_str());
  pClient = BLEDevice::createClient();
  if (!pClient->connect(&device)) {
    Serial.println("❌ Failed to connect");
    return false;
  }

  BLERemoteService* pService = pClient->getService("180D");
  if (!pService) {
    Serial.println("❌ HRM service not found");
    return false;
  }

  pHRMChar = pService->getCharacteristic("2A37");
  if (pHRMChar && pHRMChar->canNotify()) {
    pHRMChar->registerForNotify(onHRMNotify);
    Serial.println("✅ Subscribed to HRM characteristic");
    return true;
  }

  Serial.println("❌ HRM characteristic not found or can't notify");
  return false;
}

void scanAndConnectHRM() {
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  while (true) {
    Serial.println("🔁 Scanning for HRM...");
    BLEScanResults results = pBLEScan->start(5, false);
    Serial.printf("🔍 Found %d devices\n", results.getCount());

    for (int i = 0; i < results.getCount(); i++) {
      BLEAdvertisedDevice device = results.getDevice(i);
      String name = device.getName().c_str();

      if (name.startsWith(HRM_NAME_PREFIX)) {
        Serial.printf("✅ Found target HRM: %s\n", name.c_str());
        if (connectToDevice(device)) {
          return;  // Connected successfully
        }
      }
    }

    delay(3000);  // Wait before retry
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectWebSocket();
  BLEDevice::init("");
  scanAndConnectHRM();
}

void loop() {
  webSocket.loop();

  static unsigned long lastReconnect = 0;
  if (!webSocket.isConnected() && millis() - lastReconnect > 5000) {
    connectWebSocket();
    lastReconnect = millis();
  }

  delay(10);
}
