# M5Stack WiFi File Uploader

[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](https://github.com/tomorrow56/M5StackWiFiUploader)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-lightgrey.svg)](https://www.espressif.com/en/products/socs/esp32)

**An advanced Arduino library for uploading files to M5Stack's SD card via WiFi.**

This library starts an HTTP/WebSocket server on your M5Stack device, allowing you to upload photos, binaries, text files, and more directly to the SD card through a web browser. It includes advanced features for robust file transfer, such as detailed progress tracking, automatic retries, and comprehensive error handling.

[日本語版](README.md)

## ✨ Key Features

- **Dual Protocol Support**: Supports both fast WebSocket and compatible HTTP protocols.
- **Robust Error Handling**: Features 14 error codes and an automatic retry mechanism with exponential backoff.
- **Detailed Progress Tracking**: Real-time tracking of transfer speed, remaining time, and overall progress.
- **Modern Web UI**: A responsive, drag-and-drop web interface.
- **Flexible Configuration**: Freely configure file size limits, allowed extensions, concurrent uploads, and more.
- **Rich Callbacks**: Execute custom logic at each stage of the upload process (start, progress, completion, error).
- **Operation Control**: Pause, resume, and cancel uploads.
- **Lightweight Design**: Built primarily on standard ESP32 libraries to minimize external dependencies.

## 🛠️ Supported Models

- M5Stack Core
- M5Stack Core2
- M5Stack CoreS3

## 📚 Dependencies

- `WiFi`, `WebServer`, `FS`, `SD` (Built into the ESP32 core)
- `WebSocketsServer` (Included with this library)
- `ArduinoJson` (Included with this library)

## 🚀 Installation

1.  Download the latest `M5StackWiFiUploader.zip` from the [Releases page](https://github.com/tomorrow56/M5StackWiFiUploader/releases).
2.  In the Arduino IDE, go to `Sketch` -> `Include Library` -> `Add .ZIP Library...`.
3.  Select the downloaded ZIP file to install it.

## 💡 Usage

Refer to `examples/FullFeaturedDemo/FullFeaturedDemo.ino` for a basic example.

```cpp
#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

M5StackWiFiUploader uploader;

void setup() {
    M5.begin();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

    // Enable WebSocket
    uploader.enableWebSocket(true);

    // Start the uploader
    uploader.begin();

    // Set up a callback
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s\n", filename);
        }
    });
}

void loop() {
    uploader.handleClient();
}
```

## 📖 Documentation

- **[API Reference](docs/API_REFERENCE_v1.1.md)**: Detailed specifications for all classes and methods.
- **[Troubleshooting Guide](docs/TROUBLESHOOTING.md)**: Common issues and their solutions.

## 🤝 Contributing

Bug reports and feature requests are welcome on [GitHub Issues](https://github.com/tomorrow56/M5StackWiFiUploader/issues). Pull requests are also encouraged.

## 📜 License

This library is released under the **MIT License**. See the `LICENSE` file for details.
