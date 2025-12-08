#ifndef PROGRESS_TRACKER_H
#define PROGRESS_TRACKER_H

#include <Arduino.h>
#include <functional>
#include <map>

// ============================================================================
// 進捗情報構造体
// ============================================================================
struct ProgressInfo {
    String filename;              // ファイル名
    uint32_t totalBytes;          // 総バイト数
    uint32_t uploadedBytes;       // アップロード済みバイト数
    uint8_t percentage;           // 進捗率（0-100）
    float transferSpeed;          // 転送速度（バイト/秒）
    uint32_t remainingTime;       // 残り時間（秒）
    unsigned long startTime;      // 開始時刻
    unsigned long lastUpdateTime; // 最終更新時刻
    bool isActive;                // アクティブフラグ
    bool isPaused;                // 一時停止フラグ
};

// ============================================================================
// 全体進捗情報
// ============================================================================
struct OverallProgress {
    uint8_t activeUploads;        // アクティブなアップロード数
    uint8_t completedUploads;     // 完了したアップロード数
    uint8_t failedUploads;        // 失敗したアップロード数
    uint32_t totalBytes;          // 総バイト数
    uint32_t uploadedBytes;       // アップロード済みバイト数
    uint8_t percentage;           // 全体進捗率（0-100）
    float averageSpeed;           // 平均転送速度（バイト/秒）
    uint32_t estimatedTimeRemaining; // 推定残り時間（秒）
};

// ============================================================================
// 進捗コールバック
// ============================================================================
typedef std::function<void(const ProgressInfo& progress)> ProgressUpdateCallback;
typedef std::function<void(const OverallProgress& progress)> OverallProgressCallback;
typedef std::function<void(const char* filename, float speed)> SpeedUpdateCallback;

// ============================================================================
// ProgressTracker クラス
// ============================================================================
class ProgressTracker {
public:
    ProgressTracker();
    ~ProgressTracker();

    // ========================================================================
    // 進捗管理
    // ========================================================================

    /**
     * @brief 新しいアップロードを開始
     * @param filename ファイル名
     * @param totalBytes 総バイト数
     * @return セッションID
     */
    uint8_t startUpload(const char* filename, uint32_t totalBytes);

    /**
     * @brief 進捗を更新
     * @param sessionId セッションID
     * @param uploadedBytes アップロード済みバイト数
     */
    void updateProgress(uint8_t sessionId, uint32_t uploadedBytes);

    /**
     * @brief アップロードを完了
     * @param sessionId セッションID
     * @param success 成功フラグ
     */
    void completeUpload(uint8_t sessionId, bool success = true);

    /**
     * @brief アップロードを一時停止
     * @param sessionId セッションID
     */
    void pauseUpload(uint8_t sessionId);

    /**
     * @brief アップロードを再開
     * @param sessionId セッションID
     */
    void resumeUpload(uint8_t sessionId);

    /**
     * @brief アップロードをキャンセル
     * @param sessionId セッションID
     */
    void cancelUpload(uint8_t sessionId);

    // ========================================================================
    // 進捗取得
    // ========================================================================

    /**
     * @brief 進捗情報を取得
     * @param sessionId セッションID
     * @return 進捗情報
     */
    ProgressInfo getProgress(uint8_t sessionId) const;

    /**
     * @brief 全体進捗を取得
     * @return 全体進捗情報
     */
    OverallProgress getOverallProgress() const;

    /**
     * @brief 転送速度を取得
     * @param sessionId セッションID
     * @return 転送速度（バイト/秒）
     */
    float getTransferSpeed(uint8_t sessionId) const;

    /**
     * @brief 残り時間を取得
     * @param sessionId セッションID
     * @return 残り時間（秒）
     */
    uint32_t getRemainingTime(uint8_t sessionId) const;

    /**
     * @brief アクティブなセッション数を取得
     * @return セッション数
     */
    uint8_t getActiveSessionCount() const;

    // ========================================================================
    // コールバック設定
    // ========================================================================

    /**
     * @brief 進捗更新コールバックを設定
     * @param callback コールバック関数
     */
    void onProgressUpdate(ProgressUpdateCallback callback) { _progressCallback = callback; }

    /**
     * @brief 全体進捗コールバックを設定
     * @param callback コールバック関数
     */
    void onOverallProgress(OverallProgressCallback callback) { _overallCallback = callback; }

    /**
     * @brief 速度更新コールバックを設定
     * @param callback コールバック関数
     */
    void onSpeedUpdate(SpeedUpdateCallback callback) { _speedCallback = callback; }

    // ========================================================================
    // 設定
    // ========================================================================

    /**
     * @brief 速度計算の更新間隔を設定
     * @param intervalMs 更新間隔（ミリ秒）
     */
    void setSpeedUpdateInterval(uint32_t intervalMs) { _speedUpdateInterval = intervalMs; }

    /**
     * @brief 速度計算のサンプル数を設定
     * @param samples サンプル数
     */
    void setSpeedSamples(uint8_t samples) { _speedSamples = samples; }

    // ========================================================================
    // 統計情報
    // ========================================================================

    /**
     * @brief 総アップロード数を取得
     * @return アップロード数
     */
    uint32_t getTotalUploads() const { return _totalUploads; }

    /**
     * @brief 成功したアップロード数を取得
     * @return アップロード数
     */
    uint32_t getSuccessfulUploads() const { return _successfulUploads; }

    /**
     * @brief 失敗したアップロード数を取得
     * @return アップロード数
     */
    uint32_t getFailedUploads() const { return _failedUploads; }

    /**
     * @brief 総転送バイト数を取得
     * @return バイト数
     */
    uint64_t getTotalBytesTransferred() const { return _totalBytesTransferred; }

    /**
     * @brief 平均転送速度を取得
     * @return 転送速度（バイト/秒）
     */
    float getAverageSpeed() const;

    /**
     * @brief 統計をリセット
     */
    void resetStatistics();

    // ========================================================================
    // ユーティリティ
    // ========================================================================

    /**
     * @brief バイト数を人間が読める形式に変換
     * @param bytes バイト数
     * @return フォーマット済み文字列
     */
    static String formatBytes(uint32_t bytes);

    /**
     * @brief 速度を人間が読める形式に変換
     * @param bytesPerSecond バイト/秒
     * @return フォーマット済み文字列
     */
    static String formatSpeed(float bytesPerSecond);

    /**
     * @brief 時間を人間が読める形式に変換
     * @param seconds 秒
     * @return フォーマット済み文字列
     */
    static String formatTime(uint32_t seconds);

private:
    std::map<uint8_t, ProgressInfo> _sessions;
    uint8_t _nextSessionId;
    uint32_t _totalUploads;
    uint32_t _successfulUploads;
    uint32_t _failedUploads;
    uint64_t _totalBytesTransferred;
    uint32_t _speedUpdateInterval;
    uint8_t _speedSamples;
    
    ProgressUpdateCallback _progressCallback;
    OverallProgressCallback _overallCallback;
    SpeedUpdateCallback _speedCallback;

    /**
     * @brief 転送速度を計算
     * @param sessionId セッションID
     * @return 転送速度（バイト/秒）
     */
    float _calculateSpeed(uint8_t sessionId) const;

    /**
     * @brief 残り時間を計算
     * @param sessionId セッションID
     * @return 残り時間（秒）
     */
    uint32_t _calculateRemainingTime(uint8_t sessionId) const;

    /**
     * @brief 進捗率を計算
     * @param uploaded アップロード済みバイト数
     * @param total 総バイト数
     * @return 進捗率（0-100）
     */
    static uint8_t _calculatePercentage(uint32_t uploaded, uint32_t total);
};

#endif // PROGRESS_TRACKER_H
