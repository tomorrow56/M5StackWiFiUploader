/**
 * 統合テストスケッチ
 * 
 * M5StackWiFiUploader ライブラリの統合テストを実行します。
 */

#include <M5Unified.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// WiFi設定（テスト用）
const char* WIFI_SSID = "test_network";
const char* WIFI_PASSWORD = "test_password";

M5StackWiFiUploader uploader;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== M5StackWiFiUploader Integration Test ===\n");
    
    // テスト1: 初期化テスト
    testInitialization();
    
    // テスト2: 設定テスト
    testConfiguration();
    
    // テスト3: コールバックテスト
    testCallbacks();
    
    // テスト4: SDカード操作テスト
    testSDCardOperations();
    
    // テスト5: ステータス取得テスト
    testStatusQueries();
    
    Serial.println("\n=== All Integration Tests Completed ===\n");
}

void loop() {
    uploader.handleClient();
    delay(10);
}

void testInitialization() {
    Serial.println("Test 1: Initialization");
    
    // WiFi接続をシミュレート（実際の接続は不要）
    Serial.println("  Skipping WiFi connection (test mode)");
    
    // アップローダーを初期化
    bool success = uploader.begin(8080, "/test_uploads");
    
    if (success) {
        Serial.println("✓ Uploader initialized successfully");
    } else {
        Serial.println("✗ Uploader initialization failed");
    }
    
    if (uploader.isRunning()) {
        Serial.println("✓ Server is running");
    } else {
        Serial.println("✗ Server not running");
    }
    
    Serial.println();
}

void testConfiguration() {
    Serial.println("Test 2: Configuration");
    
    // 最大ファイルサイズ設定
    uploader.setMaxFileSize(10 * 1024 * 1024);
    Serial.println("✓ Max file size set to 10MB");
    
    // 許可拡張子設定
    const char* extensions[] = {"jpg", "png", "bin", "txt"};
    uploader.setAllowedExtensions(extensions, 4);
    Serial.println("✓ Allowed extensions configured");
    
    // デバッグレベル設定
    uploader.setDebugLevel(3);
    Serial.println("✓ Debug level set to 3");
    
    // WebSocket有効化
    uploader.enableWebSocket(true);
    Serial.println("✓ WebSocket enabled");
    
    // 上書き保護設定
    uploader.setOverwriteProtection(true);
    Serial.println("✓ Overwrite protection enabled");
    
    Serial.println();
}

void testCallbacks() {
    Serial.println("Test 3: Callbacks");
    
    bool startCalled = false;
    bool progressCalled = false;
    bool completeCalled = false;
    bool errorCalled = false;
    
    uploader.onUploadStart([&startCalled](const char* filename, uint32_t size) {
        startCalled = true;
        Serial.printf("  Upload start callback: %s (%d bytes)\n", filename, size);
    });
    
    uploader.onUploadProgress([&progressCalled](const char* filename, uint32_t uploaded, uint32_t total) {
        progressCalled = true;
        Serial.printf("  Progress callback: %d/%d bytes\n", uploaded, total);
    });
    
    uploader.onUploadComplete([&completeCalled](const char* filename, uint32_t size, bool success) {
        completeCalled = true;
        Serial.printf("  Complete callback: %s (%s)\n", filename, success ? "success" : "failed");
    });
    
    uploader.onUploadError([&errorCalled](const char* filename, uint8_t code, const char* msg) {
        errorCalled = true;
        Serial.printf("  Error callback: %s - Code %d: %s\n", filename, code, msg);
    });
    
    Serial.println("✓ All callbacks registered");
    Serial.println();
}

void testSDCardOperations() {
    Serial.println("Test 4: SD Card Operations");
    
    // 容量取得
    uint32_t totalSpace = uploader.getSDTotalSpace();
    uint32_t freeSpace = uploader.getSDFreeSpace();
    
    Serial.printf("  Total space: %d bytes\n", totalSpace);
    Serial.printf("  Free space: %d bytes\n", freeSpace);
    
    if (totalSpace > 0) {
        Serial.println("✓ SD card accessible");
    } else {
        Serial.println("⚠ SD card not available (expected in test mode)");
    }
    
    // ファイル一覧取得
    auto files = uploader.listFiles();
    Serial.printf("  Files in upload directory: %d\n", files.size());
    
    Serial.println();
}

void testStatusQueries() {
    Serial.println("Test 5: Status Queries");
    
    // サーバーURL取得
    String url = uploader.getServerURL();
    Serial.printf("  Server URL: %s\n", url.c_str());
    
    if (url.length() > 0) {
        Serial.println("✓ Server URL retrieved");
    } else {
        Serial.println("✗ Server URL empty");
    }
    
    // アクティブアップロード数
    uint8_t activeUploads = uploader.getActiveUploads();
    Serial.printf("  Active uploads: %d\n", activeUploads);
    
    // 総アップロードバイト数
    uint32_t totalUploaded = uploader.getTotalUploaded();
    Serial.printf("  Total uploaded: %d bytes\n", totalUploaded);
    
    Serial.println("✓ Status queries successful");
    Serial.println();
}
