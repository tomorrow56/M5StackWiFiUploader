#include "ProgressTracker.h"

// ============================================================================
// コンストラクタ・デストラクタ
// ============================================================================

ProgressTracker::ProgressTracker()
    : _nextSessionId(1),
      _totalUploads(0),
      _successfulUploads(0),
      _failedUploads(0),
      _totalBytesTransferred(0),
      _speedUpdateInterval(1000),
      _speedSamples(5),
      _progressCallback(nullptr),
      _overallCallback(nullptr),
      _speedCallback(nullptr) {
}

ProgressTracker::~ProgressTracker() {
    _sessions.clear();
}

// ============================================================================
// 進捗管理
// ============================================================================

uint8_t ProgressTracker::startUpload(const char* filename, uint32_t totalBytes) {
    uint8_t sessionId = _nextSessionId++;
    
    ProgressInfo progress;
    progress.filename = filename;
    progress.totalBytes = totalBytes;
    progress.uploadedBytes = 0;
    progress.percentage = 0;
    progress.transferSpeed = 0.0f;
    progress.remainingTime = 0;
    progress.startTime = millis();
    progress.lastUpdateTime = millis();
    progress.isActive = true;
    progress.isPaused = false;
    
    _sessions[sessionId] = progress;
    _totalUploads++;
    
    Serial.printf("[PROGRESS] Upload started: %s (Session: %d, Size: %s)\n",
                 filename, sessionId, formatBytes(totalBytes).c_str());
    
    return sessionId;
}

void ProgressTracker::updateProgress(uint8_t sessionId, uint32_t uploadedBytes) {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end() || !it->second.isActive) {
        return;
    }
    
    ProgressInfo& progress = it->second;
    
    // 一時停止中はスキップ
    if (progress.isPaused) {
        return;
    }
    
    progress.uploadedBytes = uploadedBytes;
    progress.percentage = _calculatePercentage(uploadedBytes, progress.totalBytes);
    progress.lastUpdateTime = millis();
    
    // 転送速度を計算
    progress.transferSpeed = _calculateSpeed(sessionId);
    
    // 残り時間を計算
    progress.remainingTime = _calculateRemainingTime(sessionId);
    
    // コールバックを呼び出し
    if (_progressCallback) {
        _progressCallback(progress);
    }
    
    // 速度コールバックを呼び出し
    if (_speedCallback && progress.transferSpeed > 0) {
        _speedCallback(progress.filename.c_str(), progress.transferSpeed);
    }
    
    // 全体進捗コールバックを呼び出し
    if (_overallCallback) {
        _overallCallback(getOverallProgress());
    }
}

void ProgressTracker::completeUpload(uint8_t sessionId, bool success) {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return;
    }
    
    ProgressInfo& progress = it->second;
    progress.isActive = false;
    
    if (success) {
        _successfulUploads++;
        _totalBytesTransferred += progress.uploadedBytes;
        Serial.printf("[PROGRESS] Upload completed: %s (Session: %d, Speed: %s)\n",
                     progress.filename.c_str(), sessionId,
                     formatSpeed(progress.transferSpeed).c_str());
    } else {
        _failedUploads++;
        Serial.printf("[PROGRESS] Upload failed: %s (Session: %d)\n",
                     progress.filename.c_str(), sessionId);
    }
    
    // セッションを削除
    _sessions.erase(it);
    
    // 全体進捗コールバックを呼び出し
    if (_overallCallback) {
        _overallCallback(getOverallProgress());
    }
}

void ProgressTracker::pauseUpload(uint8_t sessionId) {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return;
    }
    
    it->second.isPaused = true;
    Serial.printf("[PROGRESS] Upload paused: %s (Session: %d)\n",
                 it->second.filename.c_str(), sessionId);
}

void ProgressTracker::resumeUpload(uint8_t sessionId) {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return;
    }
    
    it->second.isPaused = false;
    it->second.lastUpdateTime = millis();
    Serial.printf("[PROGRESS] Upload resumed: %s (Session: %d)\n",
                 it->second.filename.c_str(), sessionId);
}

void ProgressTracker::cancelUpload(uint8_t sessionId) {
    completeUpload(sessionId, false);
}

// ============================================================================
// 進捗取得
// ============================================================================

ProgressInfo ProgressTracker::getProgress(uint8_t sessionId) const {
    auto it = _sessions.find(sessionId);
    if (it != _sessions.end()) {
        return it->second;
    }
    return ProgressInfo();
}

OverallProgress ProgressTracker::getOverallProgress() const {
    OverallProgress overall;
    overall.activeUploads = 0;
    overall.completedUploads = _successfulUploads;
    overall.failedUploads = _failedUploads;
    overall.totalBytes = 0;
    overall.uploadedBytes = 0;
    overall.percentage = 0;
    overall.averageSpeed = 0.0f;
    overall.estimatedTimeRemaining = 0;
    
    float totalSpeed = 0.0f;
    uint8_t speedCount = 0;
    
    for (const auto& pair : _sessions) {
        const ProgressInfo& progress = pair.second;
        if (progress.isActive) {
            overall.activeUploads++;
            overall.totalBytes += progress.totalBytes;
            overall.uploadedBytes += progress.uploadedBytes;
            
            if (progress.transferSpeed > 0) {
                totalSpeed += progress.transferSpeed;
                speedCount++;
            }
        }
    }
    
    if (overall.totalBytes > 0) {
        overall.percentage = _calculatePercentage(overall.uploadedBytes, overall.totalBytes);
    }
    
    if (speedCount > 0) {
        overall.averageSpeed = totalSpeed / speedCount;
        
        uint32_t remainingBytes = overall.totalBytes - overall.uploadedBytes;
        if (overall.averageSpeed > 0) {
            overall.estimatedTimeRemaining = remainingBytes / overall.averageSpeed;
        }
    }
    
    return overall;
}

float ProgressTracker::getTransferSpeed(uint8_t sessionId) const {
    return _calculateSpeed(sessionId);
}

uint32_t ProgressTracker::getRemainingTime(uint8_t sessionId) const {
    return _calculateRemainingTime(sessionId);
}

uint8_t ProgressTracker::getActiveSessionCount() const {
    uint8_t count = 0;
    for (const auto& pair : _sessions) {
        if (pair.second.isActive) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// 統計情報
// ============================================================================

float ProgressTracker::getAverageSpeed() const {
    if (_successfulUploads == 0) return 0.0f;
    
    // 簡易計算: 総バイト数 / 総時間
    // より正確な計算は各セッションの速度を記録する必要がある
    return 0.0f;
}

void ProgressTracker::resetStatistics() {
    _totalUploads = 0;
    _successfulUploads = 0;
    _failedUploads = 0;
    _totalBytesTransferred = 0;
}

// ============================================================================
// ユーティリティ
// ============================================================================

String ProgressTracker::formatBytes(uint32_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return String(bytes / 1024.0, 2) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return String(bytes / (1024.0 * 1024.0), 2) + " MB";
    } else {
        return String(bytes / (1024.0 * 1024.0 * 1024.0), 2) + " GB";
    }
}

String ProgressTracker::formatSpeed(float bytesPerSecond) {
    return formatBytes((uint32_t)bytesPerSecond) + "/s";
}

String ProgressTracker::formatTime(uint32_t seconds) {
    if (seconds < 60) {
        return String(seconds) + "s";
    } else if (seconds < 3600) {
        uint32_t minutes = seconds / 60;
        uint32_t secs = seconds % 60;
        return String(minutes) + "m " + String(secs) + "s";
    } else {
        uint32_t hours = seconds / 3600;
        uint32_t minutes = (seconds % 3600) / 60;
        return String(hours) + "h " + String(minutes) + "m";
    }
}

// ============================================================================
// プライベートメソッド
// ============================================================================

float ProgressTracker::_calculateSpeed(uint8_t sessionId) const {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return 0.0f;
    }
    
    const ProgressInfo& progress = it->second;
    
    unsigned long elapsedTime = millis() - progress.startTime;
    if (elapsedTime == 0) {
        return 0.0f;
    }
    
    // バイト/秒を計算
    float speed = (progress.uploadedBytes * 1000.0f) / elapsedTime;
    return speed;
}

uint32_t ProgressTracker::_calculateRemainingTime(uint8_t sessionId) const {
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end()) {
        return 0;
    }
    
    const ProgressInfo& progress = it->second;
    
    if (progress.transferSpeed <= 0) {
        return 0;
    }
    
    uint32_t remainingBytes = progress.totalBytes - progress.uploadedBytes;
    uint32_t remainingTime = remainingBytes / progress.transferSpeed;
    
    return remainingTime;
}

uint8_t ProgressTracker::_calculatePercentage(uint32_t uploaded, uint32_t total) {
    if (total == 0) return 0;
    return (uploaded * 100) / total;
}
