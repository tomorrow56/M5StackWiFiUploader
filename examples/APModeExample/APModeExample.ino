/**
 * M5Stack WiFi Uploader - APモード（アクセスポイント）サンプル
 * 
 * このサンプルは、M5StackをWiFiアクセスポイントとして動作させ、
 * 外部のWiFiルーターに接続せずにファイルをアップロードできます。
 * 
 * 使い方:
 * 1. M5Stackを起動
 * 2. PCやスマートフォンのWiFi設定で "M5Stack-AP" に接続
 * 3. パスワード: "12345678"
 * 4. ブラウザで http://192.168.4.1 にアクセス
 * 5. ファイルをアップロード
 * 
 * 対応モデル: M5Stack Core, Core2, CoreS3
 */

#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// ========================================================================
// APモード設定
// ========================================================================
const char* AP_SSID = "M5Stack-AP";           // アクセスポイント名
const char* AP_PASSWORD = "12345678";         // パスワード（8文字以上）
const IPAddress AP_IP(192, 168, 4, 1);        // M5StackのIPアドレス
const IPAddress AP_GATEWAY(192, 168, 4, 1);   // ゲートウェイ
const IPAddress AP_SUBNET(255, 255, 255, 0);  // サブネットマスク

// ========================================================================
// グローバル変数
// ========================================================================
M5StackWiFiUploader uploader;

// 統計情報
uint32_t totalUploaded = 0;
uint8_t connectedClients = 0;
String lastUploadedFile = "";

// 画面更新タイマー
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

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
    displayMessage("Starting AP Mode...", TFT_CYAN);
    
    // APモード開始
    startAPMode();
    
    // アップローダー設定
    configureUploader();
    
    // アップローダー開始
    if (uploader.begin(80, "/uploads")) {
        displayMessage("Server Ready!", TFT_GREEN);
        displayAPInfo();
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
// APモード開始
// ========================================================================
void startAPMode() {
    Serial.println("Starting Access Point Mode...");
    
    // APモード設定
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    
    // AP開始
    bool success = WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    if (success) {
        Serial.println("AP Mode Started!");
        Serial.printf("SSID: %s\n", AP_SSID);
        Serial.printf("Password: %s\n", AP_PASSWORD);
        Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("AP Mode Failed!");
        displayMessage("AP Mode Failed!", TFT_RED);
        while (1) delay(1000);
    }
    
    delay(1000);
}

// ========================================================================
// アップローダー設定
// ========================================================================
void configureUploader() {
    // 基本設定
    uploader.setMaxFileSize(50 * 1024 * 1024);  // 50MB
    uploader.setDebugLevel(3);
    uploader.enableWebSocket(false);  // APモードではWebSocketを無効化
    uploader.setOverwriteProtection(false);
    
    // 許可拡張子
    const char* extensions[] = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json", "log"
    };
    uploader.setAllowedExtensions(extensions, 11);
    
    // コールバック設定
    uploader.onUploadStart([](const char* filename, uint32_t size) {
        Serial.printf("[UPLOAD START] %s (%d bytes)\n", filename, size);
        displayUploadStatus(filename, 0, size);
    });
    
    uploader.onUploadProgress([](const char* filename, uint32_t uploaded, uint32_t total) {
        displayUploadStatus(filename, uploaded, total);
    });
    
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("[COMPLETE] %s (%d bytes)\n", filename, size);
            totalUploaded += size;
            lastUploadedFile = filename;
            displayMessage("Upload complete!", TFT_GREEN);
            delay(1000);
        } else {
            Serial.printf("[FAILED] %s\n", filename);
            displayMessage("Upload failed!", TFT_RED);
            delay(1000);
        }
    });
    
    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("[ERROR] %s - Code %d: %s\n", filename, code, msg);
        displayMessage(String("Error: ") + msg, TFT_RED);
        delay(2000);
    });
}

// ========================================================================
// ボタン処理
// ========================================================================
void handleButtons() {
    if (M5.BtnA.wasPressed()) {
        // ボタンA: 接続クライアント数表示
        displayClientInfo();
    }
    
    if (M5.BtnB.wasPressed()) {
        // ボタンB: ファイル一覧表示
        displayFileList();
    }
    
    if (M5.BtnC.wasPressed()) {
        // ボタンC: 画面クリア
        M5.Lcd.fillScreen(BLACK);
        displayHeader();
        displayAPInfo();
        displayButtons();
        displayInstructions();
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
    M5.Lcd.print("WiFi AP Mode");
}

void displayMessage(String msg, uint16_t color) {
    M5.Lcd.fillRect(0, 35, 320, 25, BLACK);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 38);
    M5.Lcd.print(msg);
}

void displayAPInfo() {
    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.printf("SSID: %s", AP_SSID);
    M5.Lcd.setCursor(10, 85);
    M5.Lcd.printf("Password: %s", AP_PASSWORD);
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.printf("IP: %s", WiFi.softAPIP().toString().c_str());
    M5.Lcd.setCursor(10, 115);
    M5.Lcd.printf("URL: http://%s", WiFi.softAPIP().toString().c_str());
}

void displayInstructions() {
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 140);
    M5.Lcd.println("How to use:");
    M5.Lcd.setCursor(10, 155);
    M5.Lcd.println("1. Connect to WiFi AP");
    M5.Lcd.setCursor(10, 170);
    M5.Lcd.println("2. Open browser");
    M5.Lcd.setCursor(10, 185);
    M5.Lcd.println("3. Go to http://192.168.4.1");
}

void displayButtons() {
    M5.Lcd.fillRect(0, 210, 320, 30, TFT_DARKGREY);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 218);
    M5.Lcd.print("Clients");
    M5.Lcd.setCursor(130, 218);
    M5.Lcd.print("Files");
    M5.Lcd.setCursor(250, 218);
    M5.Lcd.print("Clear");
}

void displayUploadStatus(const char* filename, uint32_t uploaded, uint32_t total) {
    M5.Lcd.fillRect(0, 140, 320, 70, BLACK);
    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 145);
    M5.Lcd.printf("Uploading: %s", filename);
    
    // プログレスバー
    uint8_t progress = (uploaded * 100) / total;
    M5.Lcd.setCursor(10, 160);
    M5.Lcd.printf("Progress: %d%%", progress);
    
    int barWidth = (progress * 280) / 100;
    M5.Lcd.fillRect(20, 175, 280, 15, TFT_DARKGREY);
    M5.Lcd.fillRect(20, 175, barWidth, 15, TFT_GREEN);
    
    M5.Lcd.setCursor(10, 195);
    M5.Lcd.printf("%d / %d bytes", uploaded, total);
}

void displayClientInfo() {
    M5.Lcd.fillRect(0, 140, 320, 70, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 145);
    M5.Lcd.println("Client Information:");
    
    connectedClients = WiFi.softAPgetStationNum();
    M5.Lcd.setCursor(15, 165);
    M5.Lcd.printf("Connected: %d", connectedClients);
    
    M5.Lcd.setCursor(15, 180);
    M5.Lcd.printf("Total uploaded: %d KB", totalUploaded / 1024);
}

void displayFileList() {
    M5.Lcd.fillRect(0, 140, 320, 70, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 145);
    M5.Lcd.println("Files on SD Card:");
    
    auto files = uploader.listFiles();
    int y = 165;
    for (int i = 0; i < min((int)files.size(), 3); i++) {
        M5.Lcd.setCursor(15, y);
        M5.Lcd.printf("%d. %s", i + 1, files[i].c_str());
        y += 15;
    }
    
    if (files.size() == 0) {
        M5.Lcd.setCursor(15, 165);
        M5.Lcd.print("No files");
    } else if (files.size() > 3) {
        M5.Lcd.setCursor(15, y);
        M5.Lcd.printf("...and %d more", files.size() - 3);
    }
}

void updateDisplay() {
    // ステータスバー更新
    connectedClients = WiFi.softAPgetStationNum();
    
    M5.Lcd.fillRect(0, 30, 320, 10, BLACK);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.printf("Clients: %d | Uploaded: %d KB", 
                 connectedClients,
                 totalUploaded / 1024);
}
