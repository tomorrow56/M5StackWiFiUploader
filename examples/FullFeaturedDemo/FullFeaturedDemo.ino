/**
 * M5Stack WiFi Uploader - フル機能デモ
 * 
 * このデモはライブラリのすべての機能を統合して動作を確認できます。
 * 
 * 機能:
 * - HTTP/WebSocketサーバー
 * - M5Stack画面への状態表示
 * - ボタン操作
 * - エラーハンドリング
 * - プログレス表示
 * - 再試行機能
 */

#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// ========================================================================
// WiFi設定
// ========================================================================
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

// ========================================================================
// グローバル変数
// ========================================================================
M5StackWiFiUploader uploader;

// 状態管理
enum AppState {
    STATE_INIT,
    STATE_WIFI_CONNECTING,
    STATE_RUNNING,
    STATE_ERROR
};

AppState currentState = STATE_INIT;
String statusMessage = "";
uint8_t activeUploads = 0;
uint32_t totalUploaded = 0;
String lastUploadedFile = "";

// 画面更新タイマー
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;

// ========================================================================
// セットアップ
// ========================================================================
void setup() {
    M5.begin();
    M5.Power.begin();
    Serial.begin(115200);
    
    // 画面初期化
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    
    displayHeader();
    displayMessage("Initializing...", TFT_CYAN);
    
    // WiFi接続
    connectWiFi();
    
    // アップローダー設定
    configureUploader();
    
    // アップローダー開始
    if (uploader.begin(80, "/uploads")) {
        currentState = STATE_RUNNING;
        statusMessage = "Server started";
        displayMessage("Ready!", TFT_GREEN);
        displayServerInfo();
    } else {
        currentState = STATE_ERROR;
        statusMessage = "Failed to start server";
        displayMessage("ERROR: Server failed", TFT_RED);
    }
    
    displayButtons();
}

// ========================================================================
// メインループ
// ========================================================================
void loop() {
    M5.update();
    
    // クライアントリクエスト処理
    if (currentState == STATE_RUNNING) {
        uploader.handleClient();
    }
    
    // ボタン処理
    handleButtons();
    
    // 画面更新
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    delay(10);
}

// ========================================================================
// WiFi接続
// ========================================================================
void connectWiFi() {
    currentState = STATE_WIFI_CONNECTING;
    displayMessage("Connecting to WiFi...", TFT_YELLOW);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        M5.Lcd.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        displayMessage("WiFi Connected!", TFT_GREEN);
        delay(1000);
    } else {
        currentState = STATE_ERROR;
        displayMessage("WiFi Connection Failed", TFT_RED);
        while (1) delay(1000);
    }
}

// ========================================================================
// アップローダー設定
// ========================================================================
void configureUploader() {
    // 基本設定
    uploader.setMaxFileSize(50 * 1024 * 1024);  // 50MB
    uploader.setDebugLevel(3);
    uploader.enableWebSocket(true);
    uploader.setOverwriteProtection(false);
    
    // 許可拡張子
    const char* extensions[] = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json"
    };
    uploader.setAllowedExtensions(extensions, 10);
    
    // コールバック設定
    uploader.onUploadStart([](const char* filename, uint32_t size) {
        Serial.printf("[UPLOAD START] %s (%d bytes)\n", filename, size);
        activeUploads++;
        displayUploadStatus(filename, 0, size);
    });
    
    uploader.onUploadProgress([](const char* filename, uint32_t uploaded, uint32_t total) {
        uint8_t progress = (uploaded * 100) / total;
        Serial.printf("[PROGRESS] %s: %d%%\n", filename, progress);
        displayUploadStatus(filename, uploaded, total);
    });
    
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("[COMPLETE] %s (%d bytes)\n", filename, size);
            activeUploads--;
            totalUploaded += size;
            lastUploadedFile = filename;
            displayMessage("Upload complete!", TFT_GREEN);
        } else {
            Serial.printf("[FAILED] %s\n", filename);
            activeUploads--;
            displayMessage("Upload failed!", TFT_RED);
        }
    });
    
    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("[ERROR] %s - Code %d: %s\n", filename, code, msg);
        displayMessage(String("Error: ") + msg, TFT_RED);
    });
}

// ========================================================================
// ボタン処理
// ========================================================================
void handleButtons() {
    if (M5.BtnA.wasPressed()) {
        // ボタンA: ファイル一覧表示
        displayFileList();
    }
    
    if (M5.BtnB.wasPressed()) {
        // ボタンB: ステータス表示
        displayStatus();
    }
    
    if (M5.BtnC.wasPressed()) {
        // ボタンC: 画面クリア
        M5.Lcd.fillScreen(BLACK);
        displayHeader();
        displayServerInfo();
        displayButtons();
    }
}

// ========================================================================
// 画面表示関数
// ========================================================================
void displayHeader() {
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_NAVY);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 8);
    M5.Lcd.print("WiFi File Uploader");
}

void displayMessage(String msg, uint16_t color) {
    M5.Lcd.fillRect(0, 40, 320, 30, BLACK);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 45);
    M5.Lcd.print(msg);
}

void displayServerInfo() {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("IP: %s", WiFi.localIP().toString().c_str());
    M5.Lcd.setCursor(10, 95);
    M5.Lcd.printf("URL: http://%s", WiFi.localIP().toString().c_str());
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.printf("WebSocket: ws://%s:81", WiFi.localIP().toString().c_str());
}

void displayButtons() {
    M5.Lcd.fillRect(0, 210, 320, 30, TFT_DARKGREY);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 218);
    M5.Lcd.print("Files");
    M5.Lcd.setCursor(130, 218);
    M5.Lcd.print("Status");
    M5.Lcd.setCursor(250, 218);
    M5.Lcd.print("Clear");
}

void displayUploadStatus(const char* filename, uint32_t uploaded, uint32_t total) {
    M5.Lcd.fillRect(0, 130, 320, 70, BLACK);
    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 135);
    M5.Lcd.printf("Uploading: %s", filename);
    
    // プログレスバー
    uint8_t progress = (uploaded * 100) / total;
    M5.Lcd.setCursor(10, 150);
    M5.Lcd.printf("Progress: %d%%", progress);
    
    int barWidth = (progress * 280) / 100;
    M5.Lcd.fillRect(20, 165, 280, 15, TFT_DARKGREY);
    M5.Lcd.fillRect(20, 165, barWidth, 15, TFT_GREEN);
    
    M5.Lcd.setCursor(10, 185);
    M5.Lcd.printf("%d / %d bytes", uploaded, total);
}

void displayFileList() {
    M5.Lcd.fillRect(0, 40, 320, 170, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 45);
    M5.Lcd.println("Files on SD Card:");
    
    auto files = uploader.listFiles();
    int y = 65;
    for (int i = 0; i < min((int)files.size(), 8); i++) {
        M5.Lcd.setCursor(15, y);
        M5.Lcd.printf("%d. %s", i + 1, files[i].c_str());
        y += 15;
    }
    
    if (files.size() == 0) {
        M5.Lcd.setCursor(15, 65);
        M5.Lcd.print("No files");
    }
}

void displayStatus() {
    M5.Lcd.fillRect(0, 40, 320, 170, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 45);
    M5.Lcd.println("System Status:");
    
    M5.Lcd.setCursor(15, 65);
    M5.Lcd.printf("Active uploads: %d", uploader.getActiveUploads());
    
    M5.Lcd.setCursor(15, 80);
    M5.Lcd.printf("Total uploaded: %d bytes", uploader.getTotalUploaded());
    
    M5.Lcd.setCursor(15, 95);
    M5.Lcd.printf("SD Free: %d MB", uploader.getSDFreeSpace() / 1024 / 1024);
    
    M5.Lcd.setCursor(15, 110);
    M5.Lcd.printf("SD Total: %d MB", uploader.getSDTotalSpace() / 1024 / 1024);
    
    M5.Lcd.setCursor(15, 125);
    M5.Lcd.printf("Last file: %s", lastUploadedFile.c_str());
}

void updateDisplay() {
    // ステータスバー更新
    M5.Lcd.fillRect(0, 30, 320, 10, BLACK);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.printf("Active: %d | Total: %d KB", 
                 uploader.getActiveUploads(),
                 uploader.getTotalUploaded() / 1024);
}
