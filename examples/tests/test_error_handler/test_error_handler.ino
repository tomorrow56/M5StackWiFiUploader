/**
 * ErrorHandler テストスケッチ
 * 
 * このスケッチは ErrorHandler クラスの機能をテストします。
 */

#include <M5Unified.h>
#include "ErrorHandler.h"

ErrorHandler errorHandler;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== ErrorHandler Test Suite ===\n");
    
    // テスト1: エラーログ記録
    testLogError();
    
    // テスト2: エラー履歴取得
    testErrorHistory();
    
    // テスト3: 回復可能エラーの判定
    testRecoverableErrors();
    
    // テスト4: エラー統計
    testErrorStatistics();
    
    // テスト5: エラーコールバック
    testErrorCallback();
    
    Serial.println("\n=== All Tests Completed ===\n");
}

void loop() {
    delay(1000);
}

void testLogError() {
    Serial.println("Test 1: Log Error");
    
    errorHandler.logError(ERR_TIMEOUT, "Connection timeout", "test.jpg", 1024);
    errorHandler.logError(ERR_SD_WRITE_FAILED, "SD write failed", "test2.png", 2048);
    
    ErrorInfo lastError = errorHandler.getLastError();
    
    if (lastError.code == ERR_SD_WRITE_FAILED) {
        Serial.println("✓ Last error logged correctly");
    } else {
        Serial.println("✗ Last error mismatch");
    }
    
    Serial.println();
}

void testErrorHistory() {
    Serial.println("Test 2: Error History");
    
    const auto& history = errorHandler.getErrorHistory();
    
    if (history.size() >= 2) {
        Serial.printf("✓ Error history contains %d errors\n", history.size());
        
        for (size_t i = 0; i < history.size(); i++) {
            Serial.printf("  [%d] Code: %d, Message: %s, File: %s\n",
                         i, history[i].code, history[i].message.c_str(),
                         history[i].filename.c_str());
        }
    } else {
        Serial.println("✗ Error history incomplete");
    }
    
    Serial.println();
}

void testRecoverableErrors() {
    Serial.println("Test 3: Recoverable Error Detection");
    
    struct TestCase {
        uint8_t code;
        bool expectedRecoverable;
        const char* name;
    };
    
    TestCase cases[] = {
        {ERR_TIMEOUT, true, "ERR_TIMEOUT"},
        {ERR_CONNECTION_LOST, true, "ERR_CONNECTION_LOST"},
        {ERR_OUT_OF_MEMORY, true, "ERR_OUT_OF_MEMORY"},
        {ERR_FILE_TOO_LARGE, false, "ERR_FILE_TOO_LARGE"},
        {ERR_INVALID_EXTENSION, false, "ERR_INVALID_EXTENSION"},
        {ERR_SD_WRITE_FAILED, false, "ERR_SD_WRITE_FAILED"}
    };
    
    int passed = 0;
    int total = sizeof(cases) / sizeof(cases[0]);
    
    for (int i = 0; i < total; i++) {
        bool result = ErrorHandler::isRecoverable(cases[i].code);
        if (result == cases[i].expectedRecoverable) {
            Serial.printf("  ✓ %s: %s\n", cases[i].name,
                         result ? "Recoverable" : "Not recoverable");
            passed++;
        } else {
            Serial.printf("  ✗ %s: Expected %s, got %s\n", cases[i].name,
                         cases[i].expectedRecoverable ? "recoverable" : "not recoverable",
                         result ? "recoverable" : "not recoverable");
        }
    }
    
    Serial.printf("Passed: %d/%d\n", passed, total);
    Serial.println();
}

void testErrorStatistics() {
    Serial.println("Test 4: Error Statistics");
    
    uint32_t totalErrors = errorHandler.getTotalErrors();
    uint32_t recoverableCount = errorHandler.getRecoverableErrorCount();
    uint32_t fatalCount = errorHandler.getFatalErrorCount();
    
    Serial.printf("  Total errors: %d\n", totalErrors);
    Serial.printf("  Recoverable: %d\n", recoverableCount);
    Serial.printf("  Fatal: %d\n", fatalCount);
    
    if (totalErrors == recoverableCount + fatalCount) {
        Serial.println("✓ Statistics consistent");
    } else {
        Serial.println("✗ Statistics mismatch");
    }
    
    Serial.println();
}

void testErrorCallback() {
    Serial.println("Test 5: Error Callback");
    
    bool callbackCalled = false;
    
    errorHandler.onError([&callbackCalled](const ErrorInfo& error) {
        callbackCalled = true;
        Serial.printf("  Callback triggered: Code %d, Message: %s\n",
                     error.code, error.message.c_str());
    });
    
    errorHandler.logError(ERR_TIMEOUT, "Test callback", "callback.txt", 512);
    
    if (callbackCalled) {
        Serial.println("✓ Callback executed successfully");
    } else {
        Serial.println("✗ Callback not called");
    }
    
    Serial.println();
}
