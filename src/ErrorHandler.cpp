#include "ErrorHandler.h"

// ============================================================================
// コンストラクタ・デストラクタ
// ============================================================================

ErrorHandler::ErrorHandler()
    : _totalErrors(0),
      _recoverableErrors(0),
      _fatalErrors(0),
      _errorCallback(nullptr),
      _recoveryCallback(nullptr) {
    _errorHistory.reserve(MAX_ERROR_HISTORY);
}

ErrorHandler::~ErrorHandler() {
    clearErrorHistory();
}

// ============================================================================
// エラー記録
// ============================================================================

void ErrorHandler::logError(UploadErrorCode code, const char* message,
                            const char* filename, uint32_t bytesTransferred) {
    ErrorInfo error = _createErrorInfo(code, message, filename, bytesTransferred);
    
    _lastError = error;
    _totalErrors++;
    
    if (error.isRecoverable) {
        _recoverableErrors++;
    } else {
        _fatalErrors++;
    }
    
    // エラー履歴に追加
    if (_errorHistory.size() >= MAX_ERROR_HISTORY) {
        _errorHistory.erase(_errorHistory.begin());
    }
    _errorHistory.push_back(error);
    
    // エラーハンドラーを呼び出し
    _handleError(error);
    
    // ログ出力
    Serial.printf("[ERROR] Code: %d, Message: %s", code, message);
    if (filename) {
        Serial.printf(", File: %s", filename);
    }
    Serial.printf(", Bytes: %u\n", bytesTransferred);
}

std::vector<ErrorInfo> ErrorHandler::getErrorHistory(uint8_t maxCount) const {
    if (maxCount >= _errorHistory.size()) {
        return _errorHistory;
    }
    
    std::vector<ErrorInfo> result;
    size_t startIndex = _errorHistory.size() - maxCount;
    for (size_t i = startIndex; i < _errorHistory.size(); i++) {
        result.push_back(_errorHistory[i]);
    }
    
    return result;
}

void ErrorHandler::clearErrorHistory() {
    _errorHistory.clear();
}

// ============================================================================
// エラー判定
// ============================================================================

bool ErrorHandler::isRecoverable(UploadErrorCode code) {
    switch (code) {
        case ERR_TIMEOUT:
        case ERR_CONNECTION_LOST:
        case ERR_SD_NOT_READY:
        case ERR_OUT_OF_MEMORY:
            return true;
        
        case ERR_FILE_TOO_LARGE:
        case ERR_INVALID_EXTENSION:
        case ERR_SD_WRITE_FAILED:
        case ERR_SD_FULL:
        case ERR_INVALID_DATA:
        case ERR_CHECKSUM_MISMATCH:
        case ERR_MAX_RETRIES_EXCEEDED:
        case ERR_CANCELLED:
            return false;
        
        default:
            return false;
    }
}

bool ErrorHandler::isFatal(UploadErrorCode code) {
    return !isRecoverable(code);
}

const char* ErrorHandler::getErrorCodeString(UploadErrorCode code) {
    switch (code) {
        case ERR_SUCCESS: return "ERR_SUCCESS";
        case ERR_FILE_TOO_LARGE: return "ERR_FILE_TOO_LARGE";
        case ERR_INVALID_EXTENSION: return "ERR_INVALID_EXTENSION";
        case ERR_SD_WRITE_FAILED: return "ERR_SD_WRITE_FAILED";
        case ERR_INVALID_REQUEST: return "ERR_INVALID_REQUEST";
        case ERR_TIMEOUT: return "ERR_TIMEOUT";
        case ERR_OUT_OF_MEMORY: return "ERR_OUT_OF_MEMORY";
        case ERR_CONNECTION_LOST: return "ERR_CONNECTION_LOST";
        case ERR_SD_FULL: return "ERR_SD_FULL";
        case ERR_SD_NOT_READY: return "ERR_SD_NOT_READY";
        case ERR_INVALID_DATA: return "ERR_INVALID_DATA";
        case ERR_CHECKSUM_MISMATCH: return "ERR_CHECKSUM_MISMATCH";
        case ERR_MAX_RETRIES_EXCEEDED: return "ERR_MAX_RETRIES_EXCEEDED";
        case ERR_CANCELLED: return "ERR_CANCELLED";
        default: return "ERR_UNKNOWN";
    }
}

const char* ErrorHandler::getErrorDescription(UploadErrorCode code) {
    switch (code) {
        case ERR_SUCCESS:
            return "Operation completed successfully";
        case ERR_FILE_TOO_LARGE:
            return "File size exceeds maximum allowed size";
        case ERR_INVALID_EXTENSION:
            return "File extension is not allowed";
        case ERR_SD_WRITE_FAILED:
            return "Failed to write to SD card";
        case ERR_INVALID_REQUEST:
            return "Invalid HTTP request format";
        case ERR_TIMEOUT:
            return "Operation timed out";
        case ERR_OUT_OF_MEMORY:
            return "Insufficient memory available";
        case ERR_CONNECTION_LOST:
            return "Network connection was lost";
        case ERR_SD_FULL:
            return "SD card is full";
        case ERR_SD_NOT_READY:
            return "SD card is not ready or not inserted";
        case ERR_INVALID_DATA:
            return "Received data is invalid or corrupted";
        case ERR_CHECKSUM_MISMATCH:
            return "Data checksum verification failed";
        case ERR_MAX_RETRIES_EXCEEDED:
            return "Maximum retry attempts exceeded";
        case ERR_CANCELLED:
            return "Operation was cancelled by user";
        default:
            return "Unknown error occurred";
    }
}

// ============================================================================
// 統計情報
// ============================================================================

float ErrorHandler::getErrorRate(uint32_t totalAttempts) const {
    if (totalAttempts == 0) return 0.0f;
    return (_totalErrors * 100.0f) / totalAttempts;
}

void ErrorHandler::resetStatistics() {
    _totalErrors = 0;
    _recoverableErrors = 0;
    _fatalErrors = 0;
}

// ============================================================================
// プライベートメソッド
// ============================================================================

ErrorInfo ErrorHandler::_createErrorInfo(UploadErrorCode code, const char* message,
                                         const char* filename, uint32_t bytesTransferred) {
    ErrorInfo error;
    error.code = code;
    error.message = message ? message : "";
    error.filename = filename ? filename : "";
    error.timestamp = millis();
    error.bytesTransferred = bytesTransferred;
    error.retryCount = 0;
    error.isRecoverable = isRecoverable(code);
    
    return error;
}

void ErrorHandler::_handleError(const ErrorInfo& error) {
    // エラーコールバックを呼び出し
    if (_errorCallback) {
        _errorCallback(error);
    }
    
    // 回復可能なエラーの場合、回復コールバックを呼び出し
    if (error.isRecoverable && _recoveryCallback) {
        bool shouldRecover = _recoveryCallback(error);
        if (shouldRecover) {
            Serial.println("[ERROR] Recovery attempt initiated");
        }
    }
}
