#ifndef RETRY_MANAGER_H
#define RETRY_MANAGER_H

#include <Arduino.h>
#include <functional>
#include "ErrorHandler.h"

// ============================================================================
// 再試行戦略
// ============================================================================
enum RetryStrategy {
    RETRY_FIXED,        // 固定間隔
    RETRY_LINEAR,       // 線形増加
    RETRY_EXPONENTIAL   // 指数バックオフ
};

// ============================================================================
// 再試行設定
// ============================================================================
struct RetryConfig {
    uint8_t maxRetries;           // 最大再試行回数
    uint32_t initialDelay;        // 初期遅延時間（ミリ秒）
    uint32_t maxDelay;            // 最大遅延時間（ミリ秒）
    RetryStrategy strategy;       // 再試行戦略
    float backoffMultiplier;      // バックオフ乗数（指数戦略用）
    bool retryOnTimeout;          // タイムアウト時に再試行
    bool retryOnConnectionLost;   // 接続断時に再試行
    bool retryOnMemoryError;      // メモリエラー時に再試行
};

// ============================================================================
// 再試行コールバック
// ============================================================================
typedef std::function<bool()> RetryOperation;
typedef std::function<void(uint8_t attempt, uint8_t maxRetries)> RetryAttemptCallback;
typedef std::function<void(bool success, uint8_t attempts)> RetryCompleteCallback;

// ============================================================================
// RetryManager クラス
// ============================================================================
class RetryManager {
public:
    RetryManager();
    ~RetryManager();

    // ========================================================================
    // 設定
    // ========================================================================

    /**
     * @brief 再試行設定を設定
     * @param config 再試行設定
     */
    void setConfig(const RetryConfig& config) { _config = config; }

    /**
     * @brief 再試行設定を取得
     * @return 再試行設定
     */
    RetryConfig getConfig() const { return _config; }

    /**
     * @brief 最大再試行回数を設定
     * @param maxRetries 最大再試行回数
     */
    void setMaxRetries(uint8_t maxRetries) { _config.maxRetries = maxRetries; }

    /**
     * @brief 初期遅延時間を設定
     * @param delay 遅延時間（ミリ秒）
     */
    void setInitialDelay(uint32_t delay) { _config.initialDelay = delay; }

    /**
     * @brief 再試行戦略を設定
     * @param strategy 再試行戦略
     */
    void setStrategy(RetryStrategy strategy) { _config.strategy = strategy; }

    // ========================================================================
    // コールバック設定
    // ========================================================================

    /**
     * @brief 再試行開始時のコールバックを設定
     * @param callback コールバック関数
     */
    void onRetryAttempt(RetryAttemptCallback callback) { _retryAttemptCallback = callback; }

    /**
     * @brief 再試行完了時のコールバックを設定
     * @param callback コールバック関数
     */
    void onRetryComplete(RetryCompleteCallback callback) { _retryCompleteCallback = callback; }

    // ========================================================================
    // 再試行実行
    // ========================================================================

    /**
     * @brief 操作を再試行付きで実行
     * @param operation 実行する操作（成功時trueを返す）
     * @param errorCode エラーコード（再試行判定用）
     * @return 最終的な成功/失敗
     */
    bool executeWithRetry(RetryOperation operation, UploadErrorCode errorCode = ERR_UNKNOWN);

    /**
     * @brief エラーが再試行可能かチェック
     * @param errorCode エラーコード
     * @return 再試行可能ならtrue
     */
    bool shouldRetry(UploadErrorCode errorCode) const;

    /**
     * @brief 次の再試行までの遅延時間を計算
     * @param attempt 現在の試行回数
     * @return 遅延時間（ミリ秒）
     */
    uint32_t calculateDelay(uint8_t attempt) const;

    // ========================================================================
    // 統計情報
    // ========================================================================

    /**
     * @brief 総再試行回数を取得
     * @return 再試行回数
     */
    uint32_t getTotalRetries() const { return _totalRetries; }

    /**
     * @brief 成功した再試行回数を取得
     * @return 再試行回数
     */
    uint32_t getSuccessfulRetries() const { return _successfulRetries; }

    /**
     * @brief 失敗した再試行回数を取得
     * @return 再試行回数
     */
    uint32_t getFailedRetries() const { return _failedRetries; }

    /**
     * @brief 再試行成功率を取得
     * @return 成功率（0-100）
     */
    float getRetrySuccessRate() const;

    /**
     * @brief 統計をリセット
     */
    void resetStatistics();

    // ========================================================================
    // デフォルト設定
    // ========================================================================

    /**
     * @brief デフォルト設定を取得
     * @return デフォルト再試行設定
     */
    static RetryConfig getDefaultConfig();

    /**
     * @brief 積極的な再試行設定を取得
     * @return 積極的な再試行設定
     */
    static RetryConfig getAggressiveConfig();

    /**
     * @brief 保守的な再試行設定を取得
     * @return 保守的な再試行設定
     */
    static RetryConfig getConservativeConfig();

private:
    RetryConfig _config;
    uint32_t _totalRetries;
    uint32_t _successfulRetries;
    uint32_t _failedRetries;
    
    RetryAttemptCallback _retryAttemptCallback;
    RetryCompleteCallback _retryCompleteCallback;

    /**
     * @brief 遅延を実行
     * @param delayMs 遅延時間（ミリ秒）
     */
    void _delay(uint32_t delayMs);
};

#endif // RETRY_MANAGER_H
