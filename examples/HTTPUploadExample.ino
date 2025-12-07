/**
 * @file HTTPUploadExample.ino
 * @brief M5Stack WiFi Uploader - HTTPアップロード例
 * 
 * このスケッチはHTTPプロトコルを使用してファイルをM5StackのSDカードに
 * アップロードする基本的な例です。
 * 
 * 使用方法:
 * 1. WiFi認証情報を設定
 * 2. スケッチをM5Stackにアップロード
 * 3. シリアルモニタでサーバーURLを確認
 * 4. ブラウザでそのURLにアクセス
 * 5. ファイルをアップロード
 */

#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// ============================================================================
// WiFi設定
// ============================================================================
const char* WIFI_SSID = "your_ssid";           // WiFi SSID
const char* WIFI_PASSWORD = "your_password";   // WiFiパスワード

// ============================================================================
// グローバル変数
// ============================================================================
M5StackWiFiUploader uploader(80);  // ポート80でHTTPサーバーを起動

// ============================================================================
// コールバック関数
// ============================================================================

/**
 * @brief アップロード開始時のコールバック
 */
void onUploadStart(const char* filename, uint32_t filesize) {
    Serial.printf("[UPLOAD START] %s (%.2f MB)\n", filename, filesize / 1024.0 / 1024.0);
    
    // M5Stackディスプレイに表示
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Uploading: %s\n", filename);
    M5.Lcd.printf("Size: %.2f MB\n", filesize / 1024.0 / 1024.0);
}

/**
 * @brief アップロード進捗時のコールバック
 */
void onUploadProgress(const char* filename, uint32_t uploaded, uint32_t total) {
    // 進捗率を計算
    uint8_t progress = (uploaded * 100) / total;
    
    // シリアルに出力（1%ごと）
    static uint8_t lastProgress = 0;
    if (progress != lastProgress) {
        Serial.printf("[PROGRESS] %s: %d%% (%d / %d bytes)\n", 
                     filename, progress, uploaded, total);
        lastProgress = progress;
        
        // ディスプレイに表示
        M5.Lcd.fillRect(0, 100, 320, 40, BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("Progress: %d%%\n", progress);
        M5.Lcd.printf("%d / %d bytes\n", uploaded, total);
    }
}

/**
 * @brief アップロード完了時のコールバック
 */
void onUploadComplete(const char* filename, uint32_t filesize, bool success) {
    if (success) {
        Serial.printf("[UPLOAD COMPLETE] %s successfully saved (%.2f MB)\n", 
                     filename, filesize / 1024.0 / 1024.0);
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("Upload Complete!\n");
        M5.Lcd.printf("File: %s\n", filename);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("Size: %.2f MB\n", filesize / 1024.0 / 1024.0);
    } else {
        Serial.printf("[UPLOAD FAILED] %s\n", filename);
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("Upload Failed!\n");
        M5.Lcd.printf("File: %s\n", filename);
    }
}

/**
 * @brief エラー発生時のコールバック
 */
void onUploadError(const char* filename, uint8_t errorCode, const char* message) {
    Serial.printf("[ERROR] %s - Code: %d, Message: %s\n", filename, errorCode, message);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Error!\n");
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("File: %s\n", filename);
    M5.Lcd.printf("Code: %d\n", errorCode);
    M5.Lcd.printf("Msg: %s\n", message);
}

// ============================================================================
// WiFi接続
// ============================================================================

void setupWiFi() {
    Serial.println("\n[WiFi] Connecting to WiFi...");
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Connecting WiFi...");
    
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
        Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("WiFi Connected!");
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WiFi] Failed to connect!");
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("WiFi Failed!");
    }
}

// ============================================================================
// セットアップ
// ============================================================================

void setup() {
    M5.begin();
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n=== M5Stack WiFi Uploader - HTTP Example ===\n");
    
    // WiFiに接続
    setupWiFi();
    
    // アップロードライブラリを初期化
    Serial.println("[Setup] Initializing WiFi Uploader...");
    if (uploader.begin(80, "/uploads")) {
        Serial.println("[Setup] WiFi Uploader started successfully");
        
        // コールバックを設定
        uploader.onUploadStart(onUploadStart);
        uploader.onUploadProgress(onUploadProgress);
        uploader.onUploadComplete(onUploadComplete);
        uploader.onUploadError(onUploadError);
        
        // デバッグレベルを設定（0=なし, 1=エラー, 2=警告, 3=情報, 4=詳細）
        uploader.setDebugLevel(3);
        
        // 許可する拡張子を設定
        const char* extensions[] = {"jpg", "jpeg", "png", "gif", "bin", "txt", "dat"};
        uploader.setAllowedExtensions(extensions, 7);
        
        // 最大ファイルサイズを設定（100MB）
        uploader.setMaxFileSize(100 * 1024 * 1024);
        
        // サーバーURLを表示
        Serial.println("\n=== Server Information ===");
        Serial.printf("Server URL: %s\n", uploader.getServerURL().c_str());
        Serial.printf("IP Address: %s\n", uploader.getServerIP().c_str());
        Serial.printf("Port: 80\n");
        Serial.printf("Upload Path: /uploads\n");
        Serial.println("========================\n");
        
        // ディスプレイに表示
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("Server Started!");
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("URL: %s\n", uploader.getServerURL().c_str());
        M5.Lcd.printf("IP: %s\n", uploader.getServerIP().c_str());
        
    } else {
        Serial.println("[Setup] Failed to start WiFi Uploader");
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("Server Failed!");
    }
    
    delay(2000);
}

// ============================================================================
// メインループ
// ============================================================================

void loop() {
    // M5Stackのボタンを更新
    M5.update();
    
    // クライアントリクエストを処理
    uploader.handleClient();
    
    // 定期的にステータスを表示
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime > 10000) {  // 10秒ごと
        lastStatusTime = millis();
        
        // SDカード情報を取得
        uint32_t freeSpace = uploader.getSDFreeSpace();
        uint32_t totalSpace = uploader.getSDTotalSpace();
        uint8_t activeUploads = uploader.getActiveUploads();
        
        Serial.println("\n=== Status ===");
        Serial.printf("Active Uploads: %d\n", activeUploads);
        Serial.printf("SD Free Space: %.2f MB / %.2f MB\n", 
                     freeSpace / 1024.0 / 1024.0, 
                     totalSpace / 1024.0 / 1024.0);
        Serial.printf("Total Uploaded: %.2f MB\n", 
                     uploader.getTotalUploaded() / 1024.0 / 1024.0);
        Serial.println("==============\n");
        
        // ディスプレイに表示
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.printf("Active: %d\n", activeUploads);
        M5.Lcd.printf("Free: %.1f MB\n", freeSpace / 1024.0 / 1024.0);
        M5.Lcd.printf("Total: %.1f MB\n", uploader.getTotalUploaded() / 1024.0 / 1024.0);
        M5.Lcd.printf("IP: %s\n", uploader.getServerIP().c_str());
    }
    
    delay(10);
}
