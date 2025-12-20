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

#include <M5Unified.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

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
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    
    // 画面初期化
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(WHITE);
    
    displayHeader();
    displayMessage("Initializing SD Card...", TFT_CYAN);
    
    // SDカード初期化
    if (!SDCardManager::initialize()) {
        currentState = STATE_ERROR;
        statusMessage = "SD Card initialization failed";
        displayMessage("ERROR: SD Card failed", TFT_RED);
        Serial.println("SD Card initialization failed!");
        delay(2000);
    } else {
        displayMessage("SD Card Ready!", TFT_GREEN);
        Serial.println("SD Card initialized successfully!");
        delay(1000);
    }
    
    displayMessage("Connecting WiFi...", TFT_CYAN);
    
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
        M5.Display.print(".");
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
        uint8_t progress = (total > 0) ? (uploaded * 100) / total : 0;
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
        M5.Display.fillScreen(BLACK);
        displayHeader();
        displayServerInfo();
        displayButtons();
    }
}

// ========================================================================
// 画面表示関数
// ========================================================================
void displayHeader() {
    M5.Display.fillRect(0, 0, 320, 30, TFT_NAVY);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 8);
    M5.Display.print("WiFi File Uploader");
}

void displayMessage(String msg, uint16_t color) {
    M5.Display.fillRect(0, 40, 320, 30, BLACK);
    M5.Display.setTextColor(color);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 45);
    M5.Display.print(msg);
}

void displayServerInfo() {
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 80);
    M5.Display.printf("IP: %s", WiFi.localIP().toString().c_str());
    M5.Display.setCursor(10, 95);
    M5.Display.printf("URL: http://%s", WiFi.localIP().toString().c_str());
    M5.Display.setCursor(10, 110);
    M5.Display.printf("WebSocket: ws://%s:81", WiFi.localIP().toString().c_str());
}

void displayButtons() {
    M5.Display.fillRect(0, 210, 320, 30, TFT_DARKGREY);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 218);
    M5.Display.print("Files");
    M5.Display.setCursor(130, 218);
    M5.Display.print("Status");
    M5.Display.setCursor(250, 218);
    M5.Display.print("Clear");
}

void displayUploadStatus(const char* filename, uint32_t uploaded, uint32_t total) {
    M5.Display.fillRect(0, 130, 320, 70, BLACK);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 135);
    M5.Display.printf("Uploading: %s", filename);
    
    // プログレスバー
    uint8_t progress = (total > 0) ? (uploaded * 100) / total : 0;
    M5.Display.setCursor(10, 150);
    M5.Display.printf("Progress: %d%%", progress);
    
    int barWidth = (progress * 280) / 100;
    M5.Display.fillRect(20, 165, 280, 15, TFT_DARKGREY);
    M5.Display.fillRect(20, 165, barWidth, 15, TFT_GREEN);
    
    M5.Display.setCursor(10, 185);
    M5.Display.printf("%d / %d bytes", uploaded, total);
}

void displayFileList() {
    M5.Display.fillRect(0, 40, 320, 170, BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 45);
    M5.Display.println("Files on SD Card:");
    
    auto files = uploader.listFiles();
    int y = 65;
    for (int i = 0; i < min((int)files.size(), 8); i++) {
        M5.Display.setCursor(15, y);
        M5.Display.printf("%d. %s", i + 1, files[i].c_str());
        y += 15;
    }
    
    if (files.size() == 0) {
        M5.Display.setCursor(15, 65);
        M5.Display.print("No files");
    }
}

void displayStatus() {
    M5.Display.fillRect(0, 40, 320, 170, BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 45);
    M5.Display.println("System Status:");
    
    M5.Display.setCursor(15, 65);
    M5.Display.printf("Active uploads: %d", uploader.getActiveUploads());
    
    M5.Display.setCursor(15, 80);
    M5.Display.printf("Total uploaded: %d bytes", uploader.getTotalUploaded());
    
    M5.Display.setCursor(15, 95);
    M5.Display.printf("SD Free: %d MB", uploader.getSDFreeSpace() / 1024 / 1024);
    
    M5.Display.setCursor(15, 110);
    M5.Display.printf("SD Total: %d MB", uploader.getSDTotalSpace() / 1024 / 1024);
    
    M5.Display.setCursor(15, 125);
    M5.Display.printf("Last file: %s", lastUploadedFile.c_str());
}

void updateDisplay() {
    // ステータスバー更新
    M5.Display.fillRect(0, 30, 320, 10, BLACK);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 30);
    M5.Display.printf("Active: %d | Total: %d KB", 
                 uploader.getActiveUploads(),
                 uploader.getTotalUploaded() / 1024);
}
