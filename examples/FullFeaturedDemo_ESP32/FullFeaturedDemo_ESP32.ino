/**
 * WiFi File Uploader - フル機能デモ
 * 
 * このデモはライブラリの機能を統合して動作を確認できます。
 * 
 * 機能:
 * - HTTP/WebSocketサーバー
 * - Serial Portへの状態表示
 * - エラーハンドリング
 * - 進行状況表示
 * 
 * フラッシュ削減設定:
 * 以下のdefineを#include前に記述することで、個別に機能を制御できます
 */

// ========================================================================
// フラッシュ削減設定（必要に応じてコメント解除）
// ========================================================================

// 方法1: LITE_MODEで一括設定（すべての機能を無効化）
// #define LITE_MODE 1

// 方法2: 個別に機能を選択（細かく制御したい場合）
// 以下をコメント解除して、必要な機能だけを有効/無効にできます

// WebSocket機能を無効化（削減効果: 30-50KB）
// #define ENABLE_WEBSOCKET 0

// デバッグログを無効化（削減効果: 2-5KB）
// #define ENABLE_DEBUG_LOG 0

// 高度なHTTPエンドポイントを無効化（削減効果: 3-5KB）
// - /api/files/list (詳細ファイルリスト)
// - /api/download (ファイルダウンロード)
// - /api/debug (デバッグログ)
// #define ENABLE_ADVANCED_ENDPOINTS 0

// 詳細な進捗追跡を無効化（削減効果: 10-15KB）※将来実装
// #define ENABLE_PROGRESS_TRACKER_FULL 0

// 高度な再試行機能を無効化（削減効果: 5-10KB）※将来実装
// #define ENABLE_RETRY_MANAGER_FULL 0

// ========================================================================
// 設定例
// ========================================================================

// 例1: WebSocketだけ無効化してデバッグログは残す
// #define ENABLE_WEBSOCKET 0
// #define ENABLE_DEBUG_LOG 1

// 例2: 最小構成（HTTPアップロードのみ）
// #define ENABLE_WEBSOCKET 0
// #define ENABLE_DEBUG_LOG 0
// #define ENABLE_ADVANCED_ENDPOINTS 0

#include <WiFi.h>
#include <FastLED.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

// ========================================================================
// WiFi設定
// ========================================================================
const char* WIFI_SSID = "airport_03";
const char* WIFI_PASSWORD = "09052204803011616125356565";

// ========================================================================
// SDカードSPI設定
// ========================================================================
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// ========================================================================
// LED設定 (WS2812B)
// ========================================================================
#define LED_PIN 27
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

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
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(20);
    leds[0] = CRGB::Blue;
    FastLED.show();

    Serial.begin(115200);
    
    displayHeader();
    displayMessage("Initializing SD Card...");
    
    // SDカード初期化
    if (!SDCardManager::initialize(SD_CS)) {
        currentState = STATE_ERROR;
        statusMessage = "SD Card initialization failed";
        displayMessage("ERROR: SD Card failed");
        Serial.println("SD Card initialization failed!");
        delay(2000);
    } else {
        displayMessage("SD Card Ready!");
        Serial.println("SD Card initialized successfully!");
        delay(1000);
    }
    
    displayMessage("Connecting WiFi...");
    
    // WiFi接続
    connectWiFi();
    
    // アップローダー設定
    configureUploader();
    
#if LITE_MODE
    displayMessage("LITE_MODE enabled");
    Serial.println("Running in LITE_MODE (reduced flash usage)");
#endif
    
    // アップローダー開始
    if (uploader.begin(80, "/uploads")) {
        currentState = STATE_RUNNING;
        statusMessage = "Server started";
        displayMessage("Ready!");
        displayServerInfo();
#if !ENABLE_WEBSOCKET
    Serial.println("Note: WebSocket disabled (LITE_MODE)");
#endif
    } else {
        currentState = STATE_ERROR;
        statusMessage = "Failed to start server";
        displayMessage("ERROR: Server failed");
    }
}

// ========================================================================
// メインループ
// ========================================================================
void loop() {

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
    displayMessage("Connecting to WiFi...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        displayMessage("WiFi Connected!");
        delay(1000);
    } else {
        currentState = STATE_ERROR;
        displayMessage("WiFi Connection Failed");
        while (1) delay(1000);
    }
}

// ========================================================================
// アップローダー設定
// ========================================================================
void configureUploader() {
    // 基本設定
    uploader.setMaxFileSize(50 * 1024 * 1024);  // 50MB
#if LITE_MODE
    uploader.setDebugLevel(0);  // LITE_MODEではログ無効
    uploader.enableWebSocket(false);  // LITE_MODEではWebSocket無効
#else
    uploader.setDebugLevel(3);
    uploader.enableWebSocket(true);
#endif
    uploader.setOverwriteProtection(false);
    
    // 許可拡張子
    const char* extensions[] = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json",
        "zip", "rar", "7z", "tar", "gz",
        "pdf", "mp4"
    };
    uploader.setAllowedExtensions(extensions, 17);
    
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
            displayMessage("Upload complete!");
        } else {
            Serial.printf("[FAILED] %s\n", filename);
            activeUploads--;
            displayMessage("Upload failed!");
        }
    });
    
    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("[ERROR] %s - Code %d: %s\n", filename, code, msg);
        displayMessage(String("Error: ") + msg);
    });
}

// ========================================================================
// ボタン処理
// ========================================================================
void handleButtons() {
    // 必要に応じて追加してください
}

// ========================================================================
// 画面表示関数
// ========================================================================
void displayHeader() {
    Serial.println("WiFi File Uploader");
}

void displayMessage(String msg) {
    Serial.println(msg);
}

void displayServerInfo() {
    Serial.printf("IP: %s", WiFi.localIP().toString().c_str());
    Serial.println();
    Serial.printf("URL: http://%s", WiFi.localIP().toString().c_str());
    Serial.println();
#if ENABLE_WEBSOCKET
    Serial.printf("WebSocket: ws://%s:81", WiFi.localIP().toString().c_str());
    Serial.println();
#else
    Serial.println("WebSocket: Disabled (LITE_MODE)");
#endif
}

void displayUploadStatus(const char* filename, uint32_t uploaded, uint32_t total) {
    Serial.printf("Uploading: %s", filename);
    Serial.println();
    // プログレス
    uint8_t progress = (total > 0) ? (uploaded * 100) / total : 0;
    Serial.printf("Progress: %d%%", progress);
    Serial.println();
    
    int barWidth = (progress * 280) / 100;
    Serial.printf("%d / %d bytes", uploaded, total);
    Serial.println();
}

void displayFileList() {
    Serial.println("Files on SD Card:");
    
    auto files = uploader.listFiles();
    int y = 65;
    for (int i = 0; i < min((int)files.size(), 8); i++) {
        Serial.printf("%d. %s", i + 1, files[i].c_str());
        Serial.println();
    }
    
    if (files.size() == 0) {
        Serial.println("No files");
    }
}

void displayStatus() {
    Serial.println("System Status:");
    
    Serial.printf("Active uploads: %d", uploader.getActiveUploads());
    Serial.println();
    
    Serial.printf("Total uploaded: %d bytes", uploader.getTotalUploaded());
    Serial.println();
    
    Serial.printf("SD Free: %d MB", uploader.getSDFreeSpace() / 1024 / 1024);
    Serial.println();
    
    Serial.printf("SD Total: %d MB", uploader.getSDTotalSpace() / 1024 / 1024);
    Serial.println();
    
    Serial.printf("Last file: %s", lastUploadedFile.c_str());
    Serial.println();
}

void updateDisplay() {
    // ステータスバー更新
    Serial.printf("Active: %d | Total: %d KB", 
                 uploader.getActiveUploads(),
                 uploader.getTotalUploaded() / 1024);
    Serial.println();
}
