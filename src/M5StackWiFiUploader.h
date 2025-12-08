#ifndef M5STACK_WIFI_UPLOADER_H
#define M5STACK_WIFI_UPLOADER_H

#include <Arduino.h>
#include <WebServer.h>
#include "ErrorHandler.h"
#include "RetryManager.h"
#include "ProgressTracker.h"
#include "WebSocketHandler.h"
#include <FS.h>
#include <SD.h>
#include <functional>
#include <vector>
#include <map>

// ============================================================================
// エラーコード定義
// ============================================================================
enum UploadError {
    ERR_SUCCESS = 0,
    ERR_FILE_TOO_LARGE = 1,
    ERR_INVALID_EXTENSION = 2,
    ERR_SD_WRITE_FAILED = 3,
    ERR_INVALID_REQUEST = 4,
    ERR_TIMEOUT = 5,
    ERR_OUT_OF_MEMORY = 6,
    ERR_UNKNOWN = 255
};

// ============================================================================
// コールバック型定義
// ============================================================================
typedef std::function<void(const char* filename, uint32_t filesize)> UploadCallback;
typedef std::function<void(const char* filename, uint32_t uploaded, uint32_t total)> ProgressCallback;
typedef std::function<void(const char* filename, uint32_t filesize, bool success)> CompleteCallback;
typedef std::function<void(const char* filename, uint8_t errorCode, const char* message)> ErrorCallback;

// ============================================================================
// アップロードセッション情報
// ============================================================================
struct UploadSession {
    String filename;
    uint32_t filesize;
    uint32_t uploaded;
    unsigned long startTime;
    File file;
    bool isActive;
    uint8_t sessionId;
};

// ============================================================================
// M5StackWiFiUploader メインクラス
// ============================================================================
class M5StackWiFiUploader {
public:
    M5StackWiFiUploader(uint16_t port = 80);
    ~M5StackWiFiUploader();

    // ========================================================================
    // 初期化・制御
    // ========================================================================
    
    /**
     * @brief ライブラリを初期化してHTTPサーバーを開始
     * @param port HTTPサーバーのポート番号（デフォルト80）
     * @param uploadPath アップロードファイルの保存先パス
     * @return 初期化成功時true
     */
    bool begin(uint16_t port = 80, const char* uploadPath = "/uploads");

    /**
     * @brief クライアントリクエストを処理（ループ内で定期的に呼び出す）
     */
    void handleClient();

    /**
     * @brief サーバーを停止
     */
    void end();

    /**
     * @brief サーバーが稼働中かチェック
     * @return 稼働中ならtrue
     */
    bool isRunning() const { return _isRunning; }

    // ========================================================================
    // 設定
    // ========================================================================

    /**
     * @brief 最大ファイルサイズを設定（バイト単位）
     * @param maxSize 最大サイズ（デフォルト50MB）
     */
    void setMaxFileSize(uint32_t maxSize);

    /**
     * @brief アップロード可能な拡張子を設定
     * @param extensions 拡張子配列（例: "jpg", "png", "bin"）
     * @param count 拡張子の個数
     */
    void setAllowedExtensions(const char** extensions, uint8_t count);

    /**
     * @brief アップロードパスを設定
     * @param path SDカード上のパス（例: "/uploads"）
     */
    void setUploadPath(const char* path);

    /**
     * @brief デバッグレベルを設定
     * @param level 0=なし, 1=エラー, 2=警告, 3=情報, 4=詳細
     */
    void setDebugLevel(uint8_t level);

    /**
     * @brief WebSocketサポートを有効化
     * @param enable true=有効, false=無効
     */
    void enableWebSocket(bool enable = true);

    /**
     * @brief ファイル上書き保護を有効化
     * @param enable true=上書き禁止, false=上書き許可
     */
    void setOverwriteProtection(bool enable = true);

    // ========================================================================
    // コールバック設定
    // ========================================================================

    /**
     * @brief アップロード開始時のコールバックを設定
     */
    void onUploadStart(UploadCallback callback) { _onUploadStart = callback; }

    /**
     * @brief アップロード進捗時のコールバックを設定
     */
    void onUploadProgress(ProgressCallback callback) { _onUploadProgress = callback; }

    /**
     * @brief アップロード完了時のコールバックを設定
     */
    void onUploadComplete(CompleteCallback callback) { _onUploadComplete = callback; }

    /**
     * @brief エラー発生時のコールバックを設定
     */
    void onUploadError(ErrorCallback callback) { _onUploadError = callback; }

    // ========================================================================
    // ステータス取得
    // ========================================================================

    /**
     * @brief 合計アップロードバイト数を取得
     */
    uint32_t getTotalUploaded() const { return _totalUploaded; }

    /**
     * @brief アクティブなアップロード数を取得
     */
    uint8_t getActiveUploads() const;

    /**
     * @brief サーバーのIPアドレスを取得
     */
    String getServerIP() const;

    /**
     * @brief サーバーのURLを取得
     */
    String getServerURL() const;

    // ========================================================================
    // ユーティリティ
    // ========================================================================

    /**
     * @brief SDカードの空き容量を取得（バイト単位）
     */
    uint32_t getSDFreeSpace() const;

    /**
     * @brief SDカードの総容量を取得（バイト単位）
     */
    uint32_t getSDTotalSpace() const;

    /**
     * @brief ファイルが存在するかチェック
     */
    bool fileExists(const char* filename) const;

    /**
     * @brief ファイルを削除
     */
    bool deleteFile(const char* filename);

    /**
     * @brief ディレクトリ内のファイル一覧を取得
     */
    std::vector<String> listFiles(const char* path = nullptr);

private:
    // ========================================================================
    // プライベートメンバ
    // ========================================================================
    
    WebServer* _webServer;
    ErrorHandler _errorHandler;
    RetryManager _retryManager;
    ProgressTracker _progressTracker;
    WebSocketHandler* _wsHandler;
    uint16_t _port;
    bool _isRunning;
    String _uploadPath;
    uint32_t _maxFileSize;
    std::vector<String> _allowedExtensions;
    uint8_t _debugLevel;
    bool _webSocketEnabled;
    bool _overwriteProtection;
    uint32_t _totalUploaded;
    
    std::map<uint8_t, UploadSession> _activeSessions;
    uint8_t _nextSessionId;

    // コールバック
    UploadCallback _onUploadStart;
    ProgressCallback _onUploadProgress;
    CompleteCallback _onUploadComplete;
    ErrorCallback _onUploadError;

    // ========================================================================
    // プライベートメソッド
    // ========================================================================

    // HTTPハンドラー
    void _handleUploadHTTP();
    void _handleUploadWebSocket();
    void _handleListFiles();
    void _handleDeleteFile();
    void _handleStatus();
    void _handleRoot();

    // ファイル操作
    bool _saveFile(const char* filename, uint8_t* data, uint32_t size);
    bool _isValidExtension(const char* filename);
    bool _isValidFilename(const char* filename);
    String _sanitizeFilename(const char* filename);
    bool _ensureUploadDirectory();

    // ユーティリティ
    void _log(uint8_t level, const char* format, ...);
    void _sendJSONResponse(bool success, const char* message, const char* filename = nullptr);
    String _getContentType(const char* filename);

    // セッション管理
    uint8_t _createSession(const char* filename, uint32_t filesize);
    UploadSession* _getSession(uint8_t sessionId);
    void _closeSession(uint8_t sessionId);
    void _closeAllSessions();
};

#endif // M5STACK_WIFI_UPLOADER_H
