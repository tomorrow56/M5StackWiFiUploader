/**
 * M5Stack WiFi Uploader - ファイルダウンロード機能サンプル
 * 
 * このサンプルは、SDカード内のファイル一覧表示とダウンロード機能のデモです。
 * Web UIから以下の操作が可能です：
 * - SDカード内のファイル一覧を表示
 * - ファイル名、サイズ、更新日時を確認
 * - ファイルをローカルにダウンロード
 * - ファイルを削除
 * 
 * 使い方:
 * 1. WiFi設定を自分の環境に合わせて変更
 * 2. M5Stackにアップロード
 * 3. シリアルモニタでIPアドレスを確認
 * 4. ブラウザでそのIPアドレスにアクセス
 * 5. ファイル一覧から任意のファイルをダウンロード
 * 
 * 対応モデル: M5Stack Core, Core2, CoreS3
 */

#include <M5Unified.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

// ========================================================================
// WiFi設定
// ========================================================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";         // WiFi SSID
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // WiFi パスワード

// ========================================================================
// グローバル変数
// ========================================================================
M5StackWiFiUploader uploader;

// 統計情報
uint32_t totalDownloaded = 0;
uint32_t downloadCount = 0;
String lastDownloadedFile = "";

// 画面更新タイマー
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 2000;

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
        displayMessage("ERROR: SD Card failed", TFT_RED);
        Serial.println("SD Card initialization failed!");
        delay(2000);
    } else {
        displayMessage("SD Card Ready!", TFT_GREEN);
        Serial.println("SD Card initialized successfully!");
    }
    
    displayMessage("Connecting to WiFi...", TFT_CYAN);
    
    // WiFi接続
    connectWiFi();
    
    // アップローダー設定
    configureUploader();
    
    // アップローダー開始
    if (uploader.begin(80, "/uploads")) {
        displayMessage("Server Ready!", TFT_GREEN);
        displayServerInfo();
    } else {
        displayMessage("ERROR: Server failed", TFT_RED);
        while (1) delay(1000);
    }
    
    displayButtons();
    displayInstructions();
}

// ========================================================================
// メインループ
// ========================================================================
void loop() {
    M5.update();
    
    // クライアントリクエスト処理
    uploader.handleClient();
    
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
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi Connection Failed!");
        displayMessage("WiFi Failed!", TFT_RED);
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
    uploader.setOverwriteProtection(false);
    
    // 許可拡張子
    const char* extensions[] = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json", "log", "pdf"
    };
    uploader.setAllowedExtensions(extensions, 12);
    
    // コールバック設定
    uploader.onUploadStart([](const char* filename, uint32_t size) {
        Serial.printf("[UPLOAD START] %s (%d bytes)\n", filename, size);
    });
    
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("[COMPLETE] %s (%d bytes)\n", filename, size);
        } else {
            Serial.printf("[FAILED] %s\n", filename);
        }
    });
    
    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("[ERROR] %s - Code %d: %s\n", filename, code, msg);
    });
}

// ========================================================================
// ボタン処理
// ========================================================================
void handleButtons() {
    if (M5.BtnA.wasPressed()) {
        // ボタンA: ファイル数表示
        displayFileCount();
    }
    
    if (M5.BtnB.wasPressed()) {
        // ボタンB: ダウンロード統計表示
        displayDownloadStats();
    }
    
    if (M5.BtnC.wasPressed()) {
        // ボタンC: 画面クリア
        M5.Display.fillScreen(BLACK);
        displayHeader();
        displayServerInfo();
        displayButtons();
        displayInstructions();
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
    M5.Display.print("File Download Demo");
}

void displayMessage(String msg, uint16_t color) {
    M5.Display.fillRect(0, 35, 320, 25, BLACK);
    M5.Display.setTextColor(color);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 38);
    M5.Display.print(msg);
}

void displayServerInfo() {
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 70);
    M5.Display.printf("SSID: %s", WIFI_SSID);
    M5.Display.setCursor(10, 85);
    M5.Display.printf("IP: %s", WiFi.localIP().toString().c_str());
    M5.Display.setCursor(10, 100);
    M5.Display.printf("URL: http://%s", WiFi.localIP().toString().c_str());
}

void displayInstructions() {
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 125);
    M5.Display.println("Features:");
    M5.Display.setCursor(10, 140);
    M5.Display.println("- View file list");
    M5.Display.setCursor(10, 155);
    M5.Display.println("- Download files");
    M5.Display.setCursor(10, 170);
    M5.Display.println("- Delete files");
    M5.Display.setCursor(10, 185);
    M5.Display.println("- Upload new files");
}

void displayButtons() {
    M5.Display.fillRect(0, 210, 320, 30, TFT_DARKGREY);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 218);
    M5.Display.print("Files");
    M5.Display.setCursor(130, 218);
    M5.Display.print("Stats");
    M5.Display.setCursor(250, 218);
    M5.Display.print("Clear");
}

void displayFileCount() {
    M5.Display.fillRect(0, 125, 320, 85, BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 130);
    M5.Display.println("SD Card Information:");
    
    auto files = uploader.listFiles();
    M5.Display.setCursor(15, 150);
    M5.Display.printf("Total files: %d", files.size());
    
    uint32_t totalSize = 0;
    for (const auto& file : files) {
        String fullPath = "/uploads/" + file;
        totalSize += SDCardManager::getFileSize(fullPath.c_str());
    }
    
    M5.Display.setCursor(15, 165);
    M5.Display.printf("Total size: %d KB", totalSize / 1024);
    
    M5.Display.setCursor(15, 180);
    M5.Display.printf("Free space: %d MB", uploader.getSDFreeSpace() / 1024 / 1024);
}

void displayDownloadStats() {
    M5.Display.fillRect(0, 125, 320, 85, BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 130);
    M5.Display.println("Download Statistics:");
    
    M5.Display.setCursor(15, 150);
    M5.Display.printf("Downloads: %d", downloadCount);
    
    M5.Display.setCursor(15, 165);
    M5.Display.printf("Total: %d KB", totalDownloaded / 1024);
    
    if (lastDownloadedFile.length() > 0) {
        M5.Display.setCursor(15, 180);
        M5.Display.printf("Last: %s", lastDownloadedFile.c_str());
    }
}

void updateDisplay() {
    // ステータスバー更新
    M5.Display.fillRect(0, 30, 320, 10, BLACK);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 30);
    M5.Display.printf("Active: %d | Files: %d", 
                 uploader.getActiveUploads(),
                 uploader.listFiles().size());
}
