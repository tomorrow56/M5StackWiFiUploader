/**
 * @file MultiFileUploadExample.ino
 * @brief M5Stack WiFi Uploader - 複数ファイルアップロード例
 * 
 * このスケッチは複数のファイルを同時にアップロードする例です。
 * ブラウザのUIから複数ファイルを選択してアップロードできます。
 */

#include <M5Unified.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

// ============================================================================
// WiFi設定
// ============================================================================
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

// ============================================================================
// グローバル変数
// ============================================================================
M5StackWiFiUploader uploader(80);

// アップロード統計
struct UploadStats {
    uint32_t totalFiles;
    uint32_t completedFiles;
    uint32_t failedFiles;
    uint32_t totalBytes;
    unsigned long startTime;
} stats = {0, 0, 0, 0, 0};

// ============================================================================
// コールバック関数
// ============================================================================

void onUploadStart(const char* filename, uint32_t filesize) {
    stats.totalFiles++;
    Serial.printf("[START] File %d: %s (%.2f MB)\n", 
                 stats.totalFiles, filename, filesize / 1024.0 / 1024.0);
    
    displayStatus();
}

void onUploadProgress(const char* filename, uint32_t uploaded, uint32_t total) {
    // 0除算防止
    uint8_t progress = (total > 0) ? (uploaded * 100) / total : 0;
    
    static uint8_t lastProgress = 0;
    if (progress % 10 == 0 && progress != lastProgress) {
        Serial.printf("[PROGRESS] %s: %d%% (%d / %d bytes)\n", 
                     filename, progress, uploaded, total);
        lastProgress = progress;
    }
}

void onUploadComplete(const char* filename, uint32_t filesize, bool success) {
    if (success) {
        stats.completedFiles++;
        stats.totalBytes += filesize;
        Serial.printf("[COMPLETE] %s - %.2f MB (Total: %d/%d)\n", 
                     filename, filesize / 1024.0 / 1024.0,
                     stats.completedFiles, stats.totalFiles);
    } else {
        stats.failedFiles++;
        Serial.printf("[FAILED] %s\n", filename);
    }
    
    displayStatus();
}

void onUploadError(const char* filename, uint8_t errorCode, const char* message) {
    stats.failedFiles++;
    Serial.printf("[ERROR] %s - Code: %d, Message: %s\n", filename, errorCode, message);
    displayStatus();
}

// ============================================================================
// ディスプレイ表示
// ============================================================================

void displayStatus() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
    
    M5.Display.printf("Upload Status\n");
    M5.Display.printf("Total: %d\n", stats.totalFiles);
    M5.Display.printf("Done: %d\n", stats.completedFiles);
    M5.Display.printf("Failed: %d\n", stats.failedFiles);
    
    M5.Display.setTextSize(1);
    M5.Display.printf("Data: %.2f MB\n", stats.totalBytes / 1024.0 / 1024.0);
    
    if (stats.totalFiles > 0) {
        uint8_t progress = (stats.completedFiles * 100) / stats.totalFiles;
        M5.Display.printf("Progress: %d%%\n", progress);
    }
}

void displayServerInfo() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN);
    M5.Display.setTextSize(2);
    M5.Display.println("Server Ready!");
    
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.printf("URL: %s\n", uploader.getServerURL().c_str());
    M5.Display.printf("IP: %s\n", uploader.getServerIP().c_str());
    
    // SDカード情報
    uint32_t freeSpace = uploader.getSDFreeSpace();
    uint32_t totalSpace = uploader.getSDTotalSpace();
    M5.Display.printf("SD: %.1f / %.1f MB\n", 
                 freeSpace / 1024.0 / 1024.0,
                 totalSpace / 1024.0 / 1024.0);
    
    M5.Display.setTextColor(YELLOW);
    M5.Display.printf("Access above URL\n");
    M5.Display.printf("to upload files\n");
}

// ============================================================================
// WiFi接続
// ============================================================================

void setupWiFi() {
    Serial.println("\n[WiFi] Connecting...");
    
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
    M5.Display.println("Connecting WiFi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    uint8_t count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 20) {
        delay(500);
        Serial.print(".");
        count++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WiFi] Failed!");
        
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(RED);
        M5.Display.setTextSize(2);
        M5.Display.println("WiFi Failed!");
    }
}

// ============================================================================
// セットアップ
// ============================================================================

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n=== M5Stack WiFi Uploader - Multi-File Example ===\n");
    
    // SDカード初期化
    Serial.println("[Setup] Initializing SD Card...");
    if (!SDCardManager::initialize()) {
        Serial.println("[Setup] ERROR: SD Card initialization failed!");
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(RED);
        M5.Display.setTextSize(2);
        M5.Display.println("SD Card Failed!");
        delay(3000);
    } else {
        Serial.println("[Setup] SD Card initialized successfully!");
    }
    
    // WiFiに接続
    setupWiFi();
    
    // ライブラリを初期化
    Serial.println("[Setup] Initializing uploader...");
    if (uploader.begin(80, "/uploads")) {
        Serial.println("[Setup] Uploader started");
        
        // コールバックを設定
        uploader.onUploadStart(onUploadStart);
        uploader.onUploadProgress(onUploadProgress);
        uploader.onUploadComplete(onUploadComplete);
        uploader.onUploadError(onUploadError);
        
        // 設定
        uploader.setDebugLevel(2);
        
        // 複数の画像形式を許可
        const char* extensions[] = {
            "jpg", "jpeg", "png", "gif", "bmp",
            "bin", "dat", "txt", "csv", "json"
        };
        uploader.setAllowedExtensions(extensions, 10);
        
        // 最大ファイルサイズを設定（100MB）
        uploader.setMaxFileSize(100 * 1024 * 1024);
        
        // サーバー情報を表示
        displayServerInfo();
        
        Serial.println("\n=== Server Information ===");
        Serial.printf("URL: %s\n", uploader.getServerURL().c_str());
        Serial.printf("IP: %s\n", uploader.getServerIP().c_str());
        Serial.println("===========================\n");
        
    } else {
        Serial.println("[Setup] Failed to start uploader");
        
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(RED);
        M5.Display.setTextSize(2);
        M5.Display.println("Server Failed!");
    }
    
    delay(2000);
}

// ============================================================================
// メインループ
// ============================================================================

void loop() {
    M5.update();
    
    // クライアントリクエストを処理
    uploader.handleClient();
    
    // 定期的にステータスを更新
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime > 5000) {
        lastStatusTime = millis();
        
        // アップロードが完了したかチェック
        if (stats.totalFiles > 0 && 
            stats.completedFiles + stats.failedFiles == stats.totalFiles) {
            
            // すべてのアップロードが完了
            unsigned long elapsedTime = millis() - stats.startTime;
            
            Serial.println("\n=== Upload Summary ===");
            Serial.printf("Total Files: %d\n", stats.totalFiles);
            Serial.printf("Completed: %d\n", stats.completedFiles);
            Serial.printf("Failed: %d\n", stats.failedFiles);
            Serial.printf("Total Data: %.2f MB\n", stats.totalBytes / 1024.0 / 1024.0);
            Serial.printf("Time: %lu ms\n", elapsedTime);
            if (elapsedTime > 0) {
                Serial.printf("Speed: %.2f MB/s\n", 
                             (stats.totalBytes / 1024.0 / 1024.0) / (elapsedTime / 1000.0));
            }
            Serial.println("======================\n");
            
            // 統計をリセット
            stats = {0, 0, 0, 0, 0};
            
            // サーバー情報を再表示
            displayServerInfo();
        }
        
        // 定期的にSDカード情報を出力
        static unsigned long lastInfoTime = 0;
        if (millis() - lastInfoTime > 30000) {
            lastInfoTime = millis();
            
            uint32_t freeSpace = uploader.getSDFreeSpace();
            uint32_t totalSpace = uploader.getSDTotalSpace();
            
            Serial.println("\n=== SD Card Info ===");
            Serial.printf("Free: %.2f MB\n", freeSpace / 1024.0 / 1024.0);
            Serial.printf("Total: %.2f MB\n", totalSpace / 1024.0 / 1024.0);
            Serial.printf("Usage: %d%%\n", 
                         (uint8_t)((totalSpace - freeSpace) * 100 / totalSpace));
            Serial.println("====================\n");
        }
    }
    
    delay(10);
}
