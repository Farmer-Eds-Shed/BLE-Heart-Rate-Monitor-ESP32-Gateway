# BLE Heart Rate Monitor ESP32 Gateway

This project implements a gateway using an ESP32 that scans for Bluetooth Low Energy (BLE) heart rate monitors, connects to a compatible device, and sends live heart rate data to a WebSocket server over WiFi. It is ideal for integrating BLE fitness devices into home automation or health monitoring dashboards.

## Features

- **WiFi Connectivity:** Connects to your local WiFi network.
- **WebSocket Client:** Streams BPM data to a configurable WebSocket server (e.g., Node-RED).
- **BLE Scanning:** Automatically scans and connects to heart rate monitors with a specific name prefix.
- **Threshold Alerts:** Triggers an alert if high BPM is detected repeatedly.
- **Buffering:** Buffers BPM values if the WebSocket is temporarily unavailable and sends them when reconnected.
- **Mute Duration:** Avoids repeated alerts for a configurable mute period.

## Hardware Required

- ESP32 Development Board
- BLE Heart Rate Monitor (compatible with "Fitcent_CL830" or modify the prefix)
- USB cable and serial monitor

## Getting Started

### 1. Clone the Repository

```sh
git clone https://github.com/Farmer-Eds-Shed/BLE-Heart-Rate-Monitor-ESP32-Gateway.git
```

### 2. Open the Sketch

Open `HRM-Gateway.ino` in the Arduino IDE or PlatformIO.

### 3. Configure WiFi and Server

Edit these lines in `HRM-Gateway.ino` to match your setup:

```cpp
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASS "YourWiFiPassword"
IPAddress WS_HOST(192, 168, 1, 22); // Set your server IP
#define WS_PORT 1880                 // Set your server port
#define WS_PATH "/ws/hrm"            // Set your WebSocket path
#define HRM_NAME_PREFIX "Fitcent_CL830" // BLE device name prefix
```

### 4. Upload to ESP32

Compile and upload the sketch. Open the Serial Monitor at 115200 baud to view logs.

## WebSocket Server Example

You can use [Node-RED](https://nodered.org/) or any WebSocket server to receive the BPM data. The ESP32 sends plain text messages with the BPM value, and sends "ALERT" in case of repeated high BPM.

## Customization

- **Thresholds:** Adjust `UPPER_BPM_THRESHOLD`, `SAMPLES_BEFORE_ALERT`, and `MUTE_DURATION_MS` as needed.
- **Buffer Size:** Change `MAX_BUFFERED_EVENTS` to control the buffer for unsent data.

## Dependencies

- [WiFi.h](https://www.arduino.cc/en/Reference/WiFi)
- [WebSocketsClient](https://github.com/Links2004/arduinoWebSockets)
- [ESP32 BLE Arduino](https://github.com/nkolban/ESP32_BLE_Arduino)

Install these libraries via the Arduino Library Manager or PlatformIO.

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0). See the LICENSE file for details.

## Credits

Developed by [Farmer-Eds-Shed](https://github.com/Farmer-Eds-Shed)