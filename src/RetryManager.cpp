#include "RetryManager.h"

// ============================================================================
// コンストラクタ・デストラクタ
// ============================================================================

RetryManager::RetryManager()
    : _totalRetries(0),
      _successfulRetries(0),
      _failedRetries(0),
      _retryAttemptCallback(nullptr),
      _retryCompleteCallback(nullptr) {
    _config = getDefaultConfig();
}

RetryManager::~RetryManager() {
}

// ============================================================================
// 再試行実行
// ============================================================================

bool RetryManager::executeWithRetry(RetryOperation operation, UploadErrorCode errorCode) {
    if (!operation) {
        return false;
    }

    uint8_t attempt = 0;
    bool success = false;

    while (attempt <= _config.maxRetries) {
        // 再試行開始コールバック
        if (attempt > 0 && _retryAttemptCallback) {
            _retryAttemptCallback(attempt, _config.maxRetries);
        }

        // 操作を実行
        success = operation();

        if (success) {
            // 成功
            if (attempt > 0) {
                _successfulRetries++;
                Serial.printf("[RETRY] Success after %d attempts\n", attempt);
            }
            break;
        }

        // 失敗 - 再試行判定
        if (attempt >= _config.maxRetries) {
            _failedRetries++;
            Serial.printf("[RETRY] Failed after %d attempts\n", attempt + 1);
            break;
        }

        // 再試行可能かチェック
        if (!shouldRetry(errorCode)) {
            Serial.printf("[RETRY] Error not retryable: %d\n", errorCode);
            break;
        }

        // 遅延を計算して待機
        uint32_t delay = calculateDelay(attempt);
        Serial.printf("[RETRY] Attempt %d/%d failed, retrying in %lu ms\n",
                     attempt + 1, _config.maxRetries, delay);
        _delay(delay);

        attempt++;
        _totalRetries++;
    }

    // 再試行完了コールバック
    if (_retryCompleteCallback) {
        _retryCompleteCallback(success, attempt);
    }

    return success;
}

bool RetryManager::shouldRetry(UploadErrorCode errorCode) const {
    switch (errorCode) {
        case ERR_TIMEOUT:
            return _config.retryOnTimeout;
        
        case ERR_CONNECTION_LOST:
            return _config.retryOnConnectionLost;
        
        case ERR_OUT_OF_MEMORY:
        case ERR_SD_NOT_READY:
            return _config.retryOnMemoryError;
        
        case ERR_FILE_TOO_LARGE:
        case ERR_INVALID_EXTENSION:
        case ERR_SD_FULL:
        case ERR_INVALID_DATA:
        case ERR_CHECKSUM_MISMATCH:
        case ERR_CANCELLED:
            return false;
        
        default:
            return ErrorHandler::isRecoverable(errorCode);
    }
}

uint32_t RetryManager::calculateDelay(uint8_t attempt) const {
    uint32_t delay = _config.initialDelay;

    switch (_config.strategy) {
        case RETRY_FIXED:
            // 固定間隔
            delay = _config.initialDelay;
            break;

        case RETRY_LINEAR:
            // 線形増加
            delay = _config.initialDelay * (attempt + 1);
            break;

        case RETRY_EXPONENTIAL:
            // 指数バックオフ
            delay = _config.initialDelay * pow(_config.backoffMultiplier, attempt);
            break;
    }

    // 最大遅延時間を超えないように制限
    if (delay > _config.maxDelay) {
        delay = _config.maxDelay;
    }

    return delay;
}

// ============================================================================
// 統計情報
// ============================================================================

float RetryManager::getRetrySuccessRate() const {
    if (_totalRetries == 0) return 0.0f;
    return (_successfulRetries * 100.0f) / _totalRetries;
}

void RetryManager::resetStatistics() {
    _totalRetries = 0;
    _successfulRetries = 0;
    _failedRetries = 0;
}

// ============================================================================
// デフォルト設定
// ============================================================================

RetryConfig RetryManager::getDefaultConfig() {
    RetryConfig config;
    config.maxRetries = 3;
    config.initialDelay = 1000;      // 1秒
    config.maxDelay = 10000;         // 10秒
    config.strategy = RETRY_EXPONENTIAL;
    config.backoffMultiplier = 2.0f;
    config.retryOnTimeout = true;
    config.retryOnConnectionLost = true;
    config.retryOnMemoryError = true;
    return config;
}

RetryConfig RetryManager::getAggressiveConfig() {
    RetryConfig config;
    config.maxRetries = 5;
    config.initialDelay = 500;       // 0.5秒
    config.maxDelay = 5000;          // 5秒
    config.strategy = RETRY_EXPONENTIAL;
    config.backoffMultiplier = 1.5f;
    config.retryOnTimeout = true;
    config.retryOnConnectionLost = true;
    config.retryOnMemoryError = true;
    return config;
}

RetryConfig RetryManager::getConservativeConfig() {
    RetryConfig config;
    config.maxRetries = 2;
    config.initialDelay = 2000;      // 2秒
    config.maxDelay = 15000;         // 15秒
    config.strategy = RETRY_LINEAR;
    config.backoffMultiplier = 2.0f;
    config.retryOnTimeout = true;
    config.retryOnConnectionLost = false;
    config.retryOnMemoryError = false;
    return config;
}

// ============================================================================
// プライベートメソッド
// ============================================================================

void RetryManager::_delay(uint32_t delayMs) {
    delay(delayMs);
}
