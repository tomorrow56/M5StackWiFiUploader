/**
 * ProgressTracker テストスケッチ
 * 
 * このスケッチは ProgressTracker クラスの機能をテストします。
 */

#include <M5Unified.h>
#include "ProgressTracker.h"

ProgressTracker tracker;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== ProgressTracker Test Suite ===\n");
    
    // テスト1: アップロード開始と進捗更新
    testProgressTracking();
    
    // テスト2: 転送速度計算
    testSpeedCalculation();
    
    // テスト3: 残り時間計算
    testRemainingTime();
    
    // テスト4: 複数ファイルの進捗管理
    testMultipleUploads();
    
    // テスト5: フォーマット関数
    testFormatFunctions();
    
    // テスト6: 一時停止/再開
    testPauseResume();
    
    Serial.println("\n=== All Tests Completed ===\n");
}

void loop() {
    delay(1000);
}

void testProgressTracking() {
    Serial.println("Test 1: Progress Tracking");
    
    uint8_t sessionId = tracker.startUpload("test.jpg", 1024000);
    
    // 進捗を段階的に更新
    tracker.updateProgress(sessionId, 256000);  // 25%
    ProgressInfo progress = tracker.getProgress(sessionId);
    
    if (progress.percentage == 25) {
        Serial.println("✓ Progress calculation correct (25%)");
    } else {
        Serial.printf("✗ Progress mismatch: expected 25%%, got %d%%\n", progress.percentage);
    }
    
    tracker.updateProgress(sessionId, 512000);  // 50%
    progress = tracker.getProgress(sessionId);
    
    if (progress.percentage == 50) {
        Serial.println("✓ Progress calculation correct (50%)");
    } else {
        Serial.printf("✗ Progress mismatch: expected 50%%, got %d%%\n", progress.percentage);
    }
    
    tracker.completeUpload(sessionId, true);
    Serial.println();
}

void testSpeedCalculation() {
    Serial.println("Test 2: Speed Calculation");
    
    uint8_t sessionId = tracker.startUpload("speed_test.bin", 10240000);
    
    delay(100);
    tracker.updateProgress(sessionId, 1024000);
    
    delay(100);
    tracker.updateProgress(sessionId, 2048000);
    
    ProgressInfo progress = tracker.getProgress(sessionId);
    
    if (progress.transferSpeed > 0) {
        Serial.printf("✓ Transfer speed calculated: %s\n",
                     ProgressTracker::formatSpeed(progress.transferSpeed).c_str());
    } else {
        Serial.println("✗ Transfer speed not calculated");
    }
    
    tracker.completeUpload(sessionId, true);
    Serial.println();
}

void testRemainingTime() {
    Serial.println("Test 3: Remaining Time Calculation");
    
    uint8_t sessionId = tracker.startUpload("time_test.dat", 5120000);
    
    delay(100);
    tracker.updateProgress(sessionId, 1024000);
    
    ProgressInfo progress = tracker.getProgress(sessionId);
    
    if (progress.remainingTime > 0) {
        Serial.printf("✓ Remaining time calculated: %s\n",
                     ProgressTracker::formatTime(progress.remainingTime).c_str());
    } else {
        Serial.println("✗ Remaining time not calculated");
    }
    
    tracker.completeUpload(sessionId, true);
    Serial.println();
}

void testMultipleUploads() {
    Serial.println("Test 4: Multiple Uploads");
    
    uint8_t session1 = tracker.startUpload("file1.jpg", 1024000);
    uint8_t session2 = tracker.startUpload("file2.png", 2048000);
    uint8_t session3 = tracker.startUpload("file3.bin", 512000);
    
    tracker.updateProgress(session1, 512000);
    tracker.updateProgress(session2, 1024000);
    tracker.updateProgress(session3, 256000);
    
    OverallProgress overall = tracker.getOverallProgress();
    
    Serial.printf("  Active uploads: %d\n", overall.activeUploads);
    Serial.printf("  Total bytes: %s\n", ProgressTracker::formatBytes(overall.totalBytes).c_str());
    Serial.printf("  Uploaded bytes: %s\n", ProgressTracker::formatBytes(overall.uploadedBytes).c_str());
    Serial.printf("  Overall progress: %d%%\n", overall.percentage);
    
    if (overall.activeUploads == 3) {
        Serial.println("✓ Multiple uploads tracked correctly");
    } else {
        Serial.println("✗ Active upload count mismatch");
    }
    
    tracker.completeUpload(session1, true);
    tracker.completeUpload(session2, true);
    tracker.completeUpload(session3, true);
    Serial.println();
}

void testFormatFunctions() {
    Serial.println("Test 5: Format Functions");
    
    // バイト数フォーマット
    String formatted = ProgressTracker::formatBytes(1536);
    if (formatted.indexOf("KB") >= 0) {
        Serial.printf("✓ formatBytes: %s\n", formatted.c_str());
    } else {
        Serial.printf("✗ formatBytes failed: %s\n", formatted.c_str());
    }
    
    // 速度フォーマット
    formatted = ProgressTracker::formatSpeed(1048576);
    if (formatted.indexOf("MB/s") >= 0) {
        Serial.printf("✓ formatSpeed: %s\n", formatted.c_str());
    } else {
        Serial.printf("✗ formatSpeed failed: %s\n", formatted.c_str());
    }
    
    // 時間フォーマット
    formatted = ProgressTracker::formatTime(125);
    if (formatted.indexOf("m") >= 0) {
        Serial.printf("✓ formatTime: %s\n", formatted.c_str());
    } else {
        Serial.printf("✗ formatTime failed: %s\n", formatted.c_str());
    }
    
    Serial.println();
}

void testPauseResume() {
    Serial.println("Test 6: Pause/Resume");
    
    uint8_t sessionId = tracker.startUpload("pause_test.bin", 1024000);
    
    tracker.updateProgress(sessionId, 256000);
    tracker.pauseUpload(sessionId);
    
    ProgressInfo progress = tracker.getProgress(sessionId);
    if (progress.isPaused) {
        Serial.println("✓ Upload paused successfully");
    } else {
        Serial.println("✗ Upload pause failed");
    }
    
    tracker.resumeUpload(sessionId);
    progress = tracker.getProgress(sessionId);
    
    if (!progress.isPaused) {
        Serial.println("✓ Upload resumed successfully");
    } else {
        Serial.println("✗ Upload resume failed");
    }
    
    tracker.completeUpload(sessionId, true);
    Serial.println();
}
