#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <Arduino.h>
#include <functional>

// ============================================================================
// エラーコード拡張
// ============================================================================
enum UploadErrorCode {
    ERR_SUCCESS = 0,
    ERR_FILE_TOO_LARGE = 1,
    ERR_INVALID_EXTENSION = 2,
    ERR_SD_WRITE_FAILED = 3,
    ERR_INVALID_REQUEST = 4,
    ERR_TIMEOUT = 5,
    ERR_OUT_OF_MEMORY = 6,
    ERR_CONNECTION_LOST = 7,
    ERR_SD_FULL = 8,
    ERR_SD_NOT_READY = 9,
    ERR_INVALID_DATA = 10,
    ERR_CHECKSUM_MISMATCH = 11,
    ERR_MAX_RETRIES_EXCEEDED = 12,
    ERR_CANCELLED = 13,
    ERR_UNKNOWN = 255
};

// ============================================================================
// エラー情報構造体
// ============================================================================
struct ErrorInfo {
    UploadErrorCode code;
    String message;
    String filename;
    unsigned long timestamp;
    uint32_t bytesTransferred;
    uint8_t retryCount;
    bool isRecoverable;
};

// ============================================================================
// エラーハンドラーコールバック
// ============================================================================
typedef std::function<void(const ErrorInfo& error)> ErrorHandlerCallback;
typedef std::function<bool(const ErrorInfo& error)> ErrorRecoveryCallback;

// ============================================================================
// ErrorHandler クラス
// ============================================================================
class ErrorHandler {
public:
    ErrorHandler();
    ~ErrorHandler();

    // ========================================================================
    // エラー記録
    // ========================================================================

    /**
     * @brief エラーを記録
     * @param code エラーコード
     * @param message エラーメッセージ
     * @param filename 対象ファイル名
     * @param bytesTransferred 転送済みバイト数
     */
    void logError(UploadErrorCode code, const char* message, 
                  const char* filename = nullptr, uint32_t bytesTransferred = 0);

    /**
     * @brief 最後のエラー情報を取得
     * @return エラー情報
     */
    ErrorInfo getLastError() const { return _lastError; }

    /**
     * @brief エラー履歴を取得
     * @param maxCount 最大取得数
     * @return エラー情報の配列
     */
    std::vector<ErrorInfo> getErrorHistory(uint8_t maxCount = 10) const;

    /**
     * @brief エラー履歴をクリア
     */
    void clearErrorHistory();

    // ========================================================================
    // エラーハンドリング設定
    // ========================================================================

    /**
     * @brief エラーハンドラーコールバックを設定
     * @param callback コールバック関数
     */
    void setErrorCallback(ErrorHandlerCallback callback) { _errorCallback = callback; }

    /**
     * @brief エラー回復コールバックを設定
     * @param callback コールバック関数（trueを返すと回復試行）
     */
    void setRecoveryCallback(ErrorRecoveryCallback callback) { _recoveryCallback = callback; }

    // ========================================================================
    // エラー判定
    // ========================================================================

    /**
     * @brief エラーが回復可能かチェック
     * @param code エラーコード
     * @return 回復可能ならtrue
     */
    static bool isRecoverable(UploadErrorCode code);

    /**
     * @brief エラーが致命的かチェック
     * @param code エラーコード
     * @return 致命的ならtrue
     */
    static bool isFatal(UploadErrorCode code);

    /**
     * @brief エラーコードから文字列を取得
     * @param code エラーコード
     * @return エラーコード名
     */
    static const char* getErrorCodeString(UploadErrorCode code);

    /**
     * @brief エラーコードから説明を取得
     * @param code エラーコード
     * @return エラー説明
     */
    static const char* getErrorDescription(UploadErrorCode code);

    // ========================================================================
    // 統計情報
    // ========================================================================

    /**
     * @brief 総エラー数を取得
     * @return エラー数
     */
    uint32_t getTotalErrors() const { return _totalErrors; }

    /**
     * @brief 回復可能エラー数を取得
     * @return エラー数
     */
    uint32_t getRecoverableErrors() const { return _recoverableErrors; }

    /**
     * @brief 致命的エラー数を取得
     * @return エラー数
     */
    uint32_t getFatalErrors() const { return _fatalErrors; }

    /**
     * @brief エラー率を取得
     * @param totalAttempts 総試行回数
     * @return エラー率（0-100）
     */
    float getErrorRate(uint32_t totalAttempts) const;

    /**
     * @brief 統計をリセット
     */
    void resetStatistics();

private:
    ErrorInfo _lastError;
    std::vector<ErrorInfo> _errorHistory;
    uint32_t _totalErrors;
    uint32_t _recoverableErrors;
    uint32_t _fatalErrors;
    
    ErrorHandlerCallback _errorCallback;
    ErrorRecoveryCallback _recoveryCallback;

    static const uint8_t MAX_ERROR_HISTORY = 50;

    /**
     * @brief エラー情報を作成
     */
    ErrorInfo _createErrorInfo(UploadErrorCode code, const char* message,
                               const char* filename, uint32_t bytesTransferred);

    /**
     * @brief エラーを処理
     */
    void _handleError(const ErrorInfo& error);
};

#endif // ERROR_HANDLER_H
