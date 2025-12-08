#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// WiFi設定
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

// Uploaderオブジェクト
M5StackWiFiUploader uploader;

void setup() {
    M5.begin();
    Serial.begin(115200);
    delay(100);
    
    // WiFiに接続
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi Connected!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    
    // WebSocketを有効化
    uploader.enableWebSocket(true);
    
    // アップローダーを開始
    if (uploader.begin()) {
        Serial.printf("Server started at: %s\n", uploader.getServerURL().c_str());
        Serial.printf("WebSocket is available at: ws://%s:81\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("Failed to start server");
    }

    // コールバック設定
    uploader.onUploadStart([](const char* filename, uint32_t size) {
        Serial.printf("Upload started: %s (%.2f MB)\n", filename, size / 1024.0 / 1024.0);
    });

    uploader.onUploadProgress([](const char* filename, uint32_t uploaded, uint32_t total) {
        uint8_t progress = (uploaded * 100) / total;
        Serial.printf("Progress: %d%%\n", progress);
    });

    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s\n", filename);
        } else {
            Serial.printf("Upload failed: %s\n", filename);
        }
    });

    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("Error [%d]: %s - %s\n", code, filename, msg);
    });
}

void loop() {
    uploader.handleClient();
    delay(10);
}
